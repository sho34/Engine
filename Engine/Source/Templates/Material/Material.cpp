#include "pch.h"
#include "../Templates.h"
#include "Material.h"
#include "Variables.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/DeviceUtils/Resources/Resources.h"
#include "../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include "../../Scene/Lights/Lights.h"
#include "../../Scene/Camera/Camera.h"
#include "../../Shaders/Compiler/ShaderCompiler.h"
#include "../../Common/d3dx12.h"
#include "../../Common/DirectXHelper.h"
#include <nlohmann/json.hpp>
#include <set>
#include <random>
#include <NoStd.h>
#include <DXTypes.h>

#if defined(_EDITOR)
#include "../Editor/Editor.h"
#include <Editor.h>
namespace Editor {
	extern _Templates tempTab;
	extern std::string selTemp;
}
#endif

namespace Templates {

	std::map<std::string, MaterialTemplate> materials;

	//Material+Mesh = MaterialInstance
	typedef std::pair<std::tuple<std::string, std::map<TextureType, MaterialTexture>, bool>, std::shared_ptr<MeshInstance>> MaterialMeshInstancePair;

	namespace Material
	{
#if defined(_EDITOR)
		nlohmann::json creationJson;
		unsigned int popupModalId = 0U;
#endif
		static nostd::RefTracker<MaterialMeshInstancePair, std::shared_ptr<MaterialInstance>> refTracker;
	};

	//CREATE
	void CreateMaterial(nlohmann::json json)
	{
		std::string uuid = json.at("uuid");

		if (materials.contains(uuid))
		{
			assert(!!!"materials creation collision");
		}

		MaterialTemplate t;
		std::string& name = std::get<0>(t);
		name = json.at("name");

		nlohmann::json& data = std::get<1>(t);
		data = json;
		data.erase("name");
		data.erase("uuid");

		materials.insert_or_assign(uuid, t);
#if defined(_EDITOR)
		AttachMaterialToShader(json.at("shader_vs"), uuid);
		AttachMaterialToShader(json.at("shader_ps"), uuid);
#endif
	}

	static std::mt19937 g;

	std::shared_ptr<MaterialInstance> GetMaterialInstance(std::string uuid, const std::map<TextureType, MaterialTexture>& textures, const std::shared_ptr<MeshInstance>& mesh, nlohmann::json shaderAttributes)
	{
		if (!materials.contains(uuid))
		{
			assert(!!!"material doesn't exist");
			return nullptr;
		}

		std::string instanceName = uuid;
		if (shaderAttributes.at("uniqueMaterialInstance"))
		{
			instanceName += nostd::gen_string(8, g);
		}

		auto key = MaterialMeshInstancePair(std::make_tuple(instanceName, textures, shaderAttributes.at("castShadows")), mesh);

		using namespace Material;
		return refTracker.AddRef(key, [uuid, mesh, textures, shaderAttributes]()
			{
				std::shared_ptr<MaterialInstance> instance = std::make_shared<MaterialInstance>();
				LoadMaterialInstance(uuid, mesh, instance, textures, shaderAttributes.at("castShadows"));
				return instance;
			}
		);
	}

	void LoadMaterialInstance(std::string uuid, const std::shared_ptr<MeshInstance>& mesh, const std::shared_ptr<MaterialInstance>& material, const std::map<TextureType, MaterialTexture>& textures, bool castShadows)
	{
		material->vertexClass = mesh->vertexClass;
		material->material = uuid;
		material->tupleTextures = textures;

		nlohmann::json mat = GetMaterialTemplate(uuid);

		material->vertexShaderUUID = mat.at("shader_vs");
		material->pixelShaderUUID = mat.at("shader_ps");
		std::vector<std::string> vertexClassDefines = VertexClassDefines.at(mesh->vertexClass);
		std::move(vertexClassDefines.begin(), vertexClassDefines.end(), std::back_inserter(material->defines));
		if (mat.contains("textures"))
		{
			nlohmann::json jtextures = mat.at("textures");
			for (nlohmann::json::iterator it = jtextures.begin(); it != jtextures.end(); it++)
			{
				TextureType texType = strToTextureType.at(it.key());
				DXGI_FORMAT texFormat = stringToDxgiFormat.at(it.value().at("format"));
				material->defines.push_back(textureTypeToShaderDefine.at(texType));
				if (texType == TextureType_Base && FormatsInGammaSpace.contains(texFormat))
				{
					material->defines.push_back(BaseTextureFromGammaSpaceDefine);
				}
			}
		}

		if (castShadows)
		{
			material->defines.push_back(textureTypeToShaderDefine.at(TextureType_ShadowMaps));
		}

		TransformJsonToMaterialSamplers(material->samplers, mat, "samplers");
		std::map<TextureType, MaterialTexture> matTextures;
		if (textures.size() > 0)
		{
			matTextures = textures;
		}
		else
		{
			TransformJsonToMaterialTextures(matTextures, mat, "textures");
		}
		material->textures = GetTextures(matTextures);
		material->GetShaderInstances();
	}

	void MaterialInstance::GetShaderInstances()
	{
		using namespace ShaderCompiler;
		Source compVS = { .shaderType = VERTEX_SHADER, .shaderUUID = vertexShaderUUID, .defines = defines };
		Source compPS = { .shaderType = PIXEL_SHADER, .shaderUUID = pixelShaderUUID, .defines = defines };
		vertexShader = GetShaderInstance(compVS);
		pixelShader = GetShaderInstance(compPS);
		vertexShader->BindChange([this] { NotifyRebuild(); });
		pixelShader->BindChange([this] { NotifyRebuild(); });
		nlohmann::json mat = GetMaterialTemplate(material);
		LoadVariablesMapping(mat);
	}

	void MaterialInstance::BindRebuildChange(std::function<void()> changeListener)
	{
		rebuildCallbacks.push_back(changeListener);
	}

	void MaterialInstance::NotifyRebuild()
	{
		changesCounter++;

		if (changesCounter < 2U) return;

		for (auto& cb : rebuildCallbacks)
		{
			cb();
		}

		changesCounter = 0U;
	}

	void MaterialInstance::BindMappedValueChange(std::function<void()> changeListener)
	{
		propagateMappedValueChanges.push_back(changeListener);
	}

	void MaterialInstance::NotifyMappedValueChange()
	{
		for (auto& cb : propagateMappedValueChanges)
		{
			cb();
		}
	}

	void MaterialInstance::Destroy()
	{
		using namespace ShaderCompiler;

		DestroyShaderBinary(vertexShader);
		DestroyShaderBinary(pixelShader);
		for (auto& [type, tex] : textures)
		{
			DestroyMaterialTextureInstance(tex);
		}
		textures.clear();
	}

	//READ&GET
	bool MaterialInstance::ShaderInstanceHasRegister(std::function<int(std::shared_ptr<ShaderInstance>&)> getRegister)
	{
		return (getRegister(vertexShader) != -1) || (getRegister(pixelShader) != -1);
	}

	bool MaterialInstance::ConstantsBufferContains(std::string varName)
	{
		ShaderConstantsBufferVariablesMap& vsVars = vertexShader->constantsBuffersVariables;
		ShaderConstantsBufferVariablesMap& psVars = pixelShader->constantsBuffersVariables;

		return vsVars.contains(varName) || psVars.contains(varName);
	}

	ShaderConstantsBufferVariable& MaterialInstance::GetConstantsBufferVariable(std::string varName)
	{
		ShaderConstantsBufferVariablesMap& vsVars = vertexShader->constantsBuffersVariables;
		ShaderConstantsBufferVariablesMap& psVars = pixelShader->constantsBuffersVariables;
		return (vsVars.contains(varName) ? vsVars.at(varName) : psVars.at(varName));
	}

	void MaterialInstance::LoadVariablesMapping(nlohmann::json material)
	{
		unsigned int numConstantsBuffers = static_cast<unsigned int>(max(
			vertexShader->cbufferSize.size(),
			pixelShader->cbufferSize.size()
		)); //do i really need this?

		variablesBufferSize.clear();
		variablesBuffer.clear();
		for (unsigned int index = 0; index < numConstantsBuffers; index++)
		{
			//get the CBuffer size
			size_t vsConstantBufferSize = vertexShader->cbufferSize.size() > index ? vertexShader->cbufferSize[index] : 0;
			size_t psConstantBufferSize = pixelShader->cbufferSize.size() > index ? pixelShader->cbufferSize[index] : 0;

			unsigned int constantsBufferSize = static_cast<unsigned int>(max(vsConstantBufferSize, psConstantBufferSize));
			variablesBufferSize.push_back(constantsBufferSize);

			//allocate memory for the cbuffer on the CPU
			variablesBuffer.push_back(std::vector<byte>(constantsBufferSize));
		}

		ShaderConstantsBufferVariablesMap& vsVars = vertexShader->constantsBuffersVariables;
		ShaderConstantsBufferVariablesMap& psVars = pixelShader->constantsBuffersVariables;

		//initialize the variables mapping
		nlohmann::json materialMappedValues = nlohmann::json::array();
		nlohmann::json shaderVSMappedValues = nlohmann::json::array();
		nlohmann::json shaderPSMappedValues = nlohmann::json::array();
		std::map<std::string, nlohmann::json> mappedValues;

		if (material.contains("mappedValues"))
		{
			materialMappedValues = material.at("mappedValues");
		}

		nlohmann::json shader_vs = GetShaderTemplate(material.at("shader_vs"));
		if (shader_vs.contains("mappedValues"))
		{
			shaderVSMappedValues = shader_vs.at("mappedValues");
		}

		nlohmann::json shader_ps = GetShaderTemplate(material.at("shader_ps"));
		if (shader_ps.contains("mappedValues"))
		{
			shaderPSMappedValues = shader_ps.at("mappedValues");
		}

		for (nlohmann::json mappedValue : materialMappedValues)
		{
			mappedValues.insert_or_assign(mappedValue.at("variable"), mappedValue);
		}

		for (nlohmann::json mappedValue : shaderVSMappedValues)
		{
			mappedValues.insert_or_assign(mappedValue.at("variable"), mappedValue);
		}

		for (nlohmann::json mappedValue : shaderPSMappedValues)
		{
			mappedValues.insert_or_assign(mappedValue.at("variable"), mappedValue);
		}

		variablesMapping.clear();
		for (auto& [varName, mappedValue] : mappedValues)
		{
			if (vsVars.contains(varName))
			{
				MaterialVariablesTypes variableType = StrToMaterialVariablesTypes.at(mappedValue.at("variableType"));
				ShaderConstantsBufferVariable& mapping = vsVars.at(varName);
				variablesMapping.insert_or_assign(varName, MaterialVariableMapping({ .variableType = variableType, .mapping = mapping }));
				continue;
			}

			if (psVars.contains(varName))
			{
				MaterialVariablesTypes variableType = StrToMaterialVariablesTypes.at(mappedValue.at("variableType"));
				ShaderConstantsBufferVariable& mapping = psVars.at(varName);
				variablesMapping.insert_or_assign(varName, MaterialVariableMapping({ .variableType = variableType, .mapping = mapping }));
				continue;
			}
		}

		std::set<std::string> unmapped;
		for (auto& [varName, mappedValue] : mappedValues)
		{
			if (!ConstantsBufferContains(varName)) continue;

			auto& def = GetConstantsBufferVariable(varName);

			unmapped.insert(varName);

			size_t size = def.size;
			byte* dst = variablesBuffer[def.bufferIndex].data();
			dst += def.offset;
			auto initialValue = JsonToMaterialInitialValue(mappedValue);
			WriteMappedInitialValuesToDestination(initialValue, dst, size);
		}

		//write the values to mapped memory
		for (auto [varName, mapping] : variablesMapping)
		{
			if (!mappedValues.contains(varName)) continue;

			nlohmann::json def = mappedValues.at(varName);

			size_t size = mapping.mapping.size;
			byte* dst = variablesBuffer[mapping.mapping.bufferIndex].data();
			dst += mapping.mapping.offset;
			auto initialValue = JsonToMaterialInitialValue(def);
			WriteMappedInitialValuesToDestination(initialValue, dst, size);
			unmapped.erase(varName);
		}

		//map variables defined in the shader that the material has not yet define, so it can be updated in the rendereable cbuffer
		for (auto varName : unmapped)
		{
			if (!mappedValues.contains(varName)) continue;
			if (!ConstantsBufferContains(varName)) continue;

			//auto& matdef = material.at("mappedValues").at(matVarIndex.at(varName));
			auto& matdef = mappedValues.at(varName);
			auto& variableType = StrToMaterialVariablesTypes.at(std::string(matdef.at("variableType")));
			auto& mapping = GetConstantsBufferVariable(varName);

			variablesMapping.insert_or_assign(varName,
				MaterialVariableMapping
				(
					{
					.variableType = variableType,
					.mapping = mapping
					}
				)
			);
		}
	}

	void MaterialInstance::SetRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot)
	{
		for (auto& [textureType, texParam] : pixelShader->texturesParameters)
		{
			if (texParam.numTextures == 0xFFFFFFFF) continue;
			commandList->SetGraphicsRootDescriptorTable(cbvSlot, textures.at(textureType)->gpuHandle);
			cbvSlot++;
		}
	}

	void MaterialInstance::UpdateMappedValues(nlohmann::json mappedValues)
	{
		//write the values to mapped memory
		for (unsigned int i = 0; i < mappedValues.size(); i++)
		{
			nlohmann::json def = mappedValues.at(i);
			std::string varName(def.at("variable"));

			if (!variablesMapping.contains(varName)) continue;

			MaterialVariableMapping mapping = variablesMapping.at(varName);
			size_t size = mapping.mapping.size;
			byte* dst = variablesBuffer[mapping.mapping.bufferIndex].data();
			dst += mapping.mapping.offset;
			auto initialValue = JsonToMaterialInitialValue(def);
			WriteMappedInitialValuesToDestination(initialValue, dst, size);
		}

		NotifyMappedValueChange();
	}

	void MaterialInstance::ReleaseGPUUploadIntermediateResources()
	{
		std::for_each(textures.begin(), textures.end(), [](auto& pair)
			{
				if (pair.second->upload)
				{
					pair.second->upload = nullptr;
				}
			}
		);
	}

	nlohmann::json GetMaterialTemplate(std::string uuid)
	{
		return (materials.contains(uuid)) ? std::get<1>(materials.at(uuid)) : nlohmann::json();
	}

	std::string GetMaterialName(std::string uuid)
	{
		return std::get<0>(materials.at(uuid));
	}

	void DestroyMaterialInstance(std::shared_ptr<MaterialInstance>& material, const std::shared_ptr<MeshInstance>& mesh, nlohmann::json shaderAttributes)
	{
		auto key = MaterialMeshInstancePair(std::make_tuple(material->material, material->tupleTextures, shaderAttributes.at("castShadows")), mesh);
		using namespace Material;
		refTracker.RemoveRef(key, material);
	}

	void FreeGPUTexturesUploadIntermediateResources()
	{
		using namespace Material;
		std::for_each(refTracker.instances.begin(), refTracker.instances.end(), [](auto& pair)
			{
				pair.second->ReleaseGPUUploadIntermediateResources();
			}
		);
	}

	void DestroyMaterial(std::string uuid)
	{
		materials.erase(uuid);
	}

	//DESTROY
	void ReleaseMaterialTemplates()
	{
		using namespace Material;
		refTracker.Clear();
		materials.clear();
	}

#if defined(_DEVELOPMENT)
	std::vector<std::string> GetMaterialsNames() {
		return GetNames(materials);
	}

	std::vector<UUIDName> GetMaterialsUUIDsNames()
	{
		return GetUUIDsNames(materials);
	}
#endif

	std::string FindMaterialUUIDByName(std::string name)
	{
		for (auto& [materialUUID, materialTemplate] : materials)
		{
			if (std::get<0>(materialTemplate) == name) return materialUUID;
		}

		assert(!!!"material not found");
		return "";
	}

	//EDITOR
#if defined(_EDITOR)

	void DrawMaterialPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop)
	{
		MaterialTemplate& material = materials.at(uuid);

		nlohmann::json& mat = std::get<1>(material);

		Editor::ImDrawMaterialShaderSelection(mat, "shader_vs", VERTEX_SHADER);
		Editor::ImDrawMaterialShaderSelection(mat, "shader_ps", PIXEL_SHADER);

		std::string shader_vs = mat.at("shader_vs");
		std::string shader_ps = mat.at("shader_ps");

		bool updateMappedValues = false;
		bool rebuildMaterial = false;
		ImGui::PushID("material-mapped-values");
		{
			auto mappeableVars = GetShaderMappeableVariables(shader_vs);
			auto mappeableVarsPS = GetShaderMappeableVariables(shader_ps);
			mappeableVars.insert(mappeableVarsPS.begin(), mappeableVarsPS.end());
			updateMappedValues |= ImDrawMappedValues(mat, mappeableVars);
		}
		ImGui::PopID();

		ImGui::PushID("material-textures");
		{
			auto texParams = GetShaderTextureParameters(shader_vs);
			auto texParamsPS = GetShaderTextureParameters(shader_ps);
			texParams.insert(texParamsPS.begin(), texParamsPS.end());
			rebuildMaterial |= ImDrawTextureParameters(mat, texParams);
		}
		ImGui::PopID();

		ImGui::PushID("material-samplers");
		{
			rebuildMaterial |= ImDrawSamplerParameters(mat, GetShaderSamplerParameters(shader_ps), [] { return MaterialSamplerDesc().json(); });
		}
		ImGui::PopID();

		if (updateMappedValues || rebuildMaterial)
		{
			using namespace Material;
			for (auto& [_, instance] : refTracker.instances)
			{
				if (instance->material != uuid) continue;

				if (updateMappedValues)
				{
					instance->UpdateMappedValues(mat.at("mappedValues"));
				}

				if (rebuildMaterial)
				{
					instance->CallRebuildCallbacks();
				}
			}
		}
	}

	void CreateNewMaterial()
	{
		Material::popupModalId = MaterialPopupModal_CreateNew;
		Material::creationJson = nlohmann::json(
			{
				{ "name", "" },
				{ "shader_vs", "" },
				{ "shader_ps", "" },
				{ "uuid", getUUID() }
			}
		);
	}

	void DeleteMaterial(std::string uuid)
	{
		nlohmann::json material = GetMaterialTemplate(uuid);
		if (material.contains("systemCreated") && material.at("systemCreated") == true)
		{
			Material::popupModalId = MaterialPopupModal_CannotDelete;
			return;
		}
	}

	void DrawMaterialsPopups()
	{
		Editor::DrawOkPopup(Material::popupModalId, MaterialPopupModal_CannotDelete, "Cannot delete material", []
			{
				ImGui::Text("Cannot delete a system created material");
			}
		);

		Editor::DrawCreateWindow(Material::popupModalId, MaterialPopupModal_CreateNew, "Create new material", [](auto OnCancel)
			{
				nlohmann::json& json = Material::creationJson;

				ImGui::PushID("material-name");
				{
					ImGui::Text("Name");
					ImDrawJsonInputText(json, "name");
				}
				ImGui::PopID();

				Editor::ImDrawMaterialShaderSelection(json, "shader_vs", VERTEX_SHADER);
				Editor::ImDrawMaterialShaderSelection(json, "shader_ps", PIXEL_SHADER);

				if (ImGui::Button("Cancel")) { OnCancel(); }
				ImGui::SameLine();

				bool disabledCreate = json.at("name") == "" || json.at("shader_vs") == "" || json.at("shader_ps") == "";

				if (disabledCreate)
				{
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}

				if (ImGui::Button("Create"))
				{
					Material::popupModalId = 0;
					CreateMaterial(json);
				}

				if (disabledCreate)
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}
			}
		);
	}

	void DetachShader(std::string uuid)
	{
		nlohmann::json& mat = std::get<1>(materials.at(uuid));
		if (StrToShaderType.at(mat.at("type")) == VERTEX_SHADER)
		{
			mat.at("shader_vs") = Material::fallbackShader_vs;
		}
		else
		{
			mat.at("shader_ps") = Material::fallbackShader_ps;
		}

		using namespace Material;
		for (auto& [_, instance] : refTracker.instances)
		{
			if (instance->material != uuid) continue;

			instance->CallRebuildCallbacks();
		}
	}

	void WriteMaterialsJson(nlohmann::json& json)
	{
		WriteTemplateJson(json, materials);
	}

#endif
}
