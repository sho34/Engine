#include "pch.h"
#include "Material.h"
#include "Variables.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/DeviceUtils/Resources/Resources.h"
#include "../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include "../../Scene/Lights/Lights.h"
#include "../../Scene/Camera/Camera.h"
#include "../../Common/d3dx12.h"
#include "../../Shaders/Compiler/ShaderCompiler.h"
#include <nlohmann/json.hpp>
#include <set>
#include "../../Common/DirectXHelper.h"
#include "../../pch/NoStd.h"

namespace Templates {

	static std::map<std::string, nlohmann::json> materialTemplates;

	//Material+Mesh = MaterialInstance
	typedef std::pair<std::tuple<std::string, std::map<TextureType, MaterialTexture>>, std::shared_ptr<MeshInstance>> MaterialMeshInstancePair;
	static nostd::SharedRefTracker<MaterialMeshInstancePair, MaterialInstance> refTracker;

	//NOTIFICATIONS
	/*
	TemplatesNotification<std::shared_ptr<Material>*> materialChangeNotifications;

	static void OnShaderCompilationStart(void* materialPtr) {
		auto material = static_cast<MaterialPtr*>(materialPtr);
		if (materialChangeNotifications.contains(material)) {
			NotifyOnLoadStart<MaterialPtr*>(materialChangeNotifications[material]);
		}
		(*material)->loading = true;
	}

	static void OnShaderCompilationComplete(void* materialPtr, void* shaderPtr) {
		auto& material = *static_cast<MaterialPtr*>(materialPtr);
		BuildMaterialProperties(material);
		material->loading = false;
		if (materialChangeNotifications.contains(&material)) {
			NotifyOnLoadComplete(&material, materialChangeNotifications[&material]);
		}
	}

	static void OnShaderDestroy(void* materialPtr, void* shaderPtr) {}
	*/

	//CREATE
	void CreateMaterial(std::string name, nlohmann::json json)
	{
		if (materialTemplates.contains(name)) return;
		materialTemplates.insert_or_assign(name, json);
	}

	static std::mt19937 g;

	std::shared_ptr<MaterialInstance> GetMaterialInstance(std::string name, const std::map<TextureType, MaterialTexture>& textures, const std::shared_ptr<MeshInstance>& mesh, bool uniqueMaterialInstance)
	{
		if (!materialTemplates.contains(name)) return nullptr;

		std::string instanceName = name;
		if (uniqueMaterialInstance) { instanceName += nostd::gen_string(8, g); }

		auto key = MaterialMeshInstancePair(std::make_tuple(instanceName, textures), mesh);

		return refTracker.AddRef(key, [name, mesh, textures]()
			{
				std::shared_ptr<MaterialInstance> instance = std::make_shared<MaterialInstance>();
				LoadMaterialInstance(name, mesh, instance, textures);
				return instance;
			}
		);
	}

	void LoadMaterialInstance(std::string name, const std::shared_ptr<MeshInstance>& mesh, const std::shared_ptr<MaterialInstance>& material, const std::map<TextureType, MaterialTexture>& textures)
	{
		material->vertexClass = mesh->vertexClass;

		nlohmann::json mat = GetMaterialTemplate(name);
		nlohmann::json shader = GetShaderTemplate(mat.at("shader"));

		using namespace ShaderCompiler;
		std::string fileName = shader.contains("fileName") ? std::string(shader.at("fileName")) : std::string(mat.at("shader"));
		std::vector<std::string> defines;
		std::vector<std::string> vertexClassDefines = VertexClassDefines.at(mesh->vertexClass);
		std::move(vertexClassDefines.begin(), vertexClassDefines.end(), std::back_inserter(defines));
		if (mat.contains("textures"))
		{
			nlohmann::json jtextures = mat.at("textures");
			for (nlohmann::json::iterator it = jtextures.begin(); it != jtextures.end(); it++)
			{
				defines.push_back(textureTypeToShaderDefine.at(strToTextureType.at(it.key())));
			}
		}

		Source compVS = { .shaderType = VERTEX_SHADER, .shaderName = fileName, .defines = defines };
		Source compPS = { .shaderType = PIXEL_SHADER, .shaderName = fileName, .defines = defines };
		material->material = name;
		material->tupleTextures = textures;
		material->vertexShader = GetShaderBinary(compVS);
		material->pixelShader = GetShaderBinary(compPS);
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
		material->LoadVariablesMapping(mat);
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
	bool MaterialInstance::ShaderBinaryHasRegister(std::function<int(std::shared_ptr<ShaderBinary>&)> getRegister)
	{
		return (getRegister(vertexShader) != -1) || (getRegister(pixelShader) != -1);
	}

	void MaterialInstance::LoadVariablesMapping(nlohmann::json material)
	{
		unsigned int numConstantsBuffers = static_cast<unsigned int>(max(
			vertexShader->cbufferSize.size(),
			pixelShader->cbufferSize.size()
		)); //do i really need this?

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
		nlohmann::json shaderMappedValues = nlohmann::json::array();
		std::map<std::string, nlohmann::json> mappedValues;

		if (material.contains("mappedValues")) {
			materialMappedValues = material.at("mappedValues");
		}

		nlohmann::json shader = GetShaderTemplate(material.at("shader"));
		if (shader.contains("mappedValues")) {
			shaderMappedValues = shader.at("mappedValues");
		}

		for (nlohmann::json mappedValue : materialMappedValues)
		{
			mappedValues.insert_or_assign(mappedValue.at("variable"), mappedValue);
		}

		for (nlohmann::json mappedValue : shaderMappedValues)
		{
			mappedValues.insert_or_assign(mappedValue.at("variable"), mappedValue);
		}

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
			if (!vertexShader->constantsBuffersVariables.contains(varName)) continue;

			auto& def = vertexShader->constantsBuffersVariables.at(varName);

			unmapped.insert(varName);

			size_t size = def.size;
			byte* dst = variablesBuffer[def.bufferIndex].data();
			dst += def.offset;
			auto initialValue = JsonToMaterialInitialValue(mappedValue);
			WriteMappedInitialValuesToDestination(initialValue, dst, size);
		}

		//write the values to mapped memory
		for (auto [varName, mapping] : variablesMapping) {

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
			//if (!matVarIndex.contains(varName)) continue;
			if (!mappedValues.contains(varName)) continue;
			if (!vertexShader->constantsBuffersVariables.contains(varName)) continue;

			//auto& matdef = material.at("mappedValues").at(matVarIndex.at(varName));
			auto& matdef = mappedValues.at(varName);
			auto& variableType = StrToMaterialVariablesTypes.at(std::string(matdef.at("variableType")));
			auto& mapping = vertexShader->constantsBuffersVariables.at(varName);

			variablesMapping.insert_or_assign(varName,
				MaterialVariableMapping(
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

	nlohmann::json GetMaterialTemplate(std::string name)
	{
		return (materialTemplates.contains(name)) ? materialTemplates.at(name) : nlohmann::json();
	}

	void DestroyMaterialInstance(std::shared_ptr<MaterialInstance>& material, const std::shared_ptr<MeshInstance>& mesh)
	{
		auto key = MaterialMeshInstancePair(std::make_tuple(material->material, material->tupleTextures), mesh);
		refTracker.RemoveRef(key, material);
	}

	//DESTROY
	void ReleaseMaterialTemplates()
	{
		refTracker.Clear();
		materialTemplates.clear();
	}

	std::vector<std::string> GetMaterialsNames() {
		return nostd::GetKeysFromMap(materialTemplates);
	}

	//EDITOR
#if defined(_EDITOR)
	void DrawMaterialPanel(std::string& material, ImVec2 pos, ImVec2 size, bool pop)
	{

	}

	std::string GetMaterialInstanceTemplateName(std::shared_ptr<MaterialInstance> material)
	{
		for (auto it : refTracker.instances) {
			if (it.second == material) return std::get<0>(it.first.first);
		}
		return "";
	}

	/*
	nlohmann::json json()
	{
		nlohmann::json j = nlohmann::json({});

		for (auto& [name, material] : materialTemplates) {
			if (material->materialDefinition.systemCreated) continue;

			j[name] = nlohmann::json({});
			j[name]["shaderTemplate"] = material->materialDefinition.shaderTemplate;
			j[name]["mappedValues"] = TransformMaterialValueMappingToJson(material->materialDefinition.mappedValues);

			if (material->materialDefinition.textures.size() > 0) {
				j[name]["textures"] = TransformTexturesDefinitionToJson(material->materialDefinition.textures);
			}

			if (material->materialDefinition.samplers.size() > 0) {
				j[name]["samplers"] = nlohmann::json::array();

				for (auto& sampler : material->materialDefinition.samplers) {
					j[name]["samplers"].push_back(sampler.json());
				}

			}

			j[name]["twoSided"] = material->materialDefinition.twoSided;
		}
		return j;
	}
	*/
#endif
}
