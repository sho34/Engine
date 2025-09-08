#include "pch.h"
#include <Templates.h>
#include <TemplateDef.h>
#include "Material.h"
#include "Variables.h"
#include <ShaderCompiler.h>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <random>
#include <NoStd.h>
#include <DXTypes.h>

#if defined(_EDITOR)
#include <VertexFormats.h>
#endif

namespace Templates {

#include <Editor/JDrawersDef.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <MaterialAtt.h>
#include <JEnd.h>

	MaterialJson::MaterialJson(nlohmann::json json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <MaterialAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <MaterialAtt.h>
#include <JEnd.h>
	}

	TEMPDEF_FULL(Material);
	TEMPDEF_REFTRACKER(Material);

	void MaterialJsonStep()
	{
		std::set<std::shared_ptr<MaterialJson>> mats;
		std::transform(Materialtemplates.begin(), Materialtemplates.end(), std::inserter(mats, mats.begin()), [](auto& temps)
			{
				auto& matJ = std::get<1>(temps.second);
				return matJ;
			}
		);

		std::set<std::shared_ptr<MaterialJson>> rebuildMaterials;
		std::copy_if(mats.begin(), mats.end(), std::inserter(rebuildMaterials, rebuildMaterials.begin()), [](auto& mat)
			{
				return mat->dirty(MaterialJson::Update_shader_vs) ||
					mat->dirty(MaterialJson::Update_shader_ps) ||
					mat->dirty(MaterialJson::Update_samplers) ||
					mat->dirty(MaterialJson::Update_mappedValues) ||
					mat->dirty(MaterialJson::Update_textures) ||
					mat->dirty(MaterialJson::Update_rasterizerState) ||
					mat->dirty(MaterialJson::Update_blendState);
			}
		);

		if (rebuildMaterials.size() > 0ULL)
		{
			JObject::RunChangesCallback(rebuildMaterials, [](auto mat)
				{
					mat->clean(MaterialJson::Update_shader_vs);
					mat->clean(MaterialJson::Update_shader_ps);
					mat->clean(MaterialJson::Update_samplers);
					mat->clean(MaterialJson::Update_mappedValues);
					mat->clean(MaterialJson::Update_textures);
					mat->clean(MaterialJson::Update_rasterizerState);
					mat->clean(MaterialJson::Update_blendState);
				}
			);
		}
	}

	MaterialInstance::MaterialInstance(
		const std::string instance_uuid,
		const std::string uuid,
		VertexClass vClass,
		bool isShadowed,
		TextureShaderUsageMap overrideTextures,
		std::string bindingUUID,
		JObjectChangeCallback materialChangeCallback,
		JObjectChangePostCallback materialChangePostCallback
	)
	{
		instanceUUID = instance_uuid;
		materialUUID = uuid;

		std::shared_ptr<MaterialJson> material = GetMaterialTemplate(uuid);
		if (bindingUUID != "" && (materialChangeCallback != nullptr || materialChangePostCallback != nullptr)) {
			material->BindChangeCallback(bindingUUID, materialChangeCallback, materialChangePostCallback);
		}

		auto matTextures = material->textures();
		std::transform(matTextures.begin(), matTextures.end(), std::inserter(textures, textures.end()), [](auto& pair)
			{
				return TextureUsageInstancePair(pair.first, GetTextureInstance(pair.second));
			}
		);
		std::transform(overrideTextures.begin(), overrideTextures.end(), std::inserter(textures, textures.end()), [](auto& pair)
			{
				return TextureUsageInstancePair(pair.first, GetTextureInstance(pair.second));
			}
		);
		if (textures.size() > 0ULL) samplers = material->samplers();

		vertexShaderUUID = material->shader_vs();
		pixelShaderUUID = material->shader_ps();

		vertexClass = vClass;
		shadowed = isShadowed;
		CreateMaterialShaderDefines();
		CreateShaderInstances();
		LoadVariablesMapping();
	}

	void MaterialInstance::CreateMaterialShaderDefines()
	{
		defines.clear();

		//OutputDebugStringA((instanceName + ": buildDefines:" + materialUUID + "\n").c_str());

		std::vector<std::string> vertexClassDefines = VertexClassDefines.at(vertexClass);

		std::shared_ptr<MaterialJson> mat = GetMaterialTemplate(materialUUID);
		TextureShaderUsageMap texMap = mat->textures();

		if (texMap.size() == 0UL)
		{
			//remove textures components 
			std::copy_if(vertexClassDefines.begin(), vertexClassDefines.end(), std::back_inserter(defines), [](auto& def)
				{
					return !VertexTextureCompoentsString.contains(def);
				}
			);
		}
		else
		{
			std::move(vertexClassDefines.begin(), vertexClassDefines.end(), std::back_inserter(defines));
			for (auto& [texType, texUUID] : texMap)
			{
				defines.push_back(textureShaderUsageToShaderDefine.at(texType));

				std::shared_ptr<TextureJson> tex = GetTextureTemplate(texUUID);
				if (NonLinearDxgiFormats.contains(tex->format()))
				{
					std::string srgbTexDefine = textureShaderUsageInGammaSpaceToShaderDefine.at(texType);
					defines.push_back(srgbTexDefine);
				}
			}
		}

		if (shadowed)
		{
			defines.push_back(textureShaderUsageToShaderDefine.at(TextureShaderUsage_ShadowMaps));
		}
		/*
		if (ibl)
		{
			defines.push_back(textureShaderUsageToShaderDefine.at(TextureShaderUsage_IBLIrradiance));
			defines.push_back(textureShaderUsageToShaderDefine.at(TextureShaderUsage_IBLPreFilteredEnvironment));
			defines.push_back(textureShaderUsageToShaderDefine.at(TextureShaderUsage_IBLBRDFLUT));
		}
		*/
	}

	void MaterialInstance::CreateShaderInstances()
	{
		using namespace ShaderCompiler;
		Source compVS = { .shaderType = VERTEX_SHADER, .shaderUUID = vertexShaderUUID, .defines = defines };
		Source compPS = { .shaderType = PIXEL_SHADER, .shaderUUID = pixelShaderUUID, .defines = defines };
		std::string vertexShaderInstanceUUID = vertexShaderUUID + std::to_string(std::hash<Source>()(compVS));
		std::string pixelShaderInstanceUUID = pixelShaderUUID + std::to_string(std::hash<Source>()(compPS));
		auto onVSShaderChange = [this](std::shared_ptr<JObject> vsShader)
			{
				std::shared_ptr<MaterialJson> mat = GetMaterialTemplate(materialUUID);
				mat->flag(MaterialJson::Update_shader_vs);
			};
		auto onPSShaderChange = [this](std::shared_ptr<JObject> psShader)
			{
				std::shared_ptr<MaterialJson> mat = GetMaterialTemplate(materialUUID);
				mat->flag(MaterialJson::Update_shader_ps);
			};
		vertexShader = GetShaderInstance(vertexShaderInstanceUUID, [this, vertexShaderInstanceUUID, compVS, onVSShaderChange]
			{
				return std::make_shared<ShaderInstance>(vertexShaderInstanceUUID, compVS.shaderUUID, compVS, instanceUUID, onVSShaderChange);
			}
		);
		pixelShader = GetShaderInstance(pixelShaderInstanceUUID, [this, pixelShaderInstanceUUID, compPS, onPSShaderChange]
			{
				return std::make_shared<ShaderInstance>(pixelShaderInstanceUUID, compPS.shaderUUID, compPS, instanceUUID, onPSShaderChange);
			}
		);
	}

	void MaterialInstance::Destroy()
	{
		using namespace ShaderCompiler;

		std::shared_ptr<ShaderJson> vsShaderJson = GetShaderTemplate(vertexShaderUUID);
		if (vsShaderJson)
			vsShaderJson->UnbindChangeCallback(instanceUUID);

		std::shared_ptr<ShaderJson> psShaderJson = GetShaderTemplate(pixelShaderUUID);
		if (psShaderJson)
			psShaderJson->UnbindChangeCallback(instanceUUID);

		if (RemoveShaderInstance(vertexShader->instanceUUID, vertexShader))
		{
			vertexShader = nullptr;
		}
		if (RemoveShaderInstance(pixelShader->instanceUUID, pixelShader))
		{
			pixelShader = nullptr;
		}
		for (auto& [type, tex] : textures)
		{
			RemoveTextureInstance(tex->materialTexture, tex);
			//DestroyTextureInstance(tex);
		}
		textures.clear();
	}

	void MaterialInstance::LoadVariablesMapping()
	{
		//do i really need this?
		unsigned int numConstantsBuffers = static_cast<unsigned int>(
			max(vertexShader->cbufferSize.size(), pixelShader->cbufferSize.size())
			);

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
		std::shared_ptr<MaterialJson> material = GetMaterialTemplate(materialUUID);
		std::shared_ptr<ShaderJson> shader_vs = GetShaderTemplate(material->shader_vs());
		std::shared_ptr<ShaderJson> shader_ps = GetShaderTemplate(material->shader_ps());

		nlohmann::json materialMappedValues = nlohmann::json::array();
		nlohmann::json shaderVSMappedValues = nlohmann::json::array();
		nlohmann::json shaderPSMappedValues = nlohmann::json::array();

		std::map<std::string, nlohmann::json> mappedValues;

		//mapped values comes from the material, vertex and pixel shaders
		if (material->contains("mappedValues")) { materialMappedValues = material->at("mappedValues"); }
		if (shader_vs->contains("mappedValues")) { shaderVSMappedValues = shader_vs->at("mappedValues"); }
		if (shader_ps->contains("mappedValues")) { shaderPSMappedValues = shader_ps->at("mappedValues"); }
		for (nlohmann::json mappedValue : materialMappedValues) { mappedValues.insert_or_assign(mappedValue.at("variable"), mappedValue); }
		for (nlohmann::json mappedValue : shaderVSMappedValues) { mappedValues.insert_or_assign(mappedValue.at("variable"), mappedValue); }
		for (nlohmann::json mappedValue : shaderPSMappedValues) { mappedValues.insert_or_assign(mappedValue.at("variable"), mappedValue); }

		variablesMapping.clear();

		for (auto& [varName, mappedValue] : mappedValues)
		{
			if (vsVars.contains(varName))
			{
				MaterialVariablesTypes variableType = StringToMaterialVariablesTypes.at(mappedValue.at("variableType"));
				ShaderConstantsBufferVariable& mapping = vsVars.at(varName);
				variablesMapping.insert_or_assign(varName, MaterialVariableMapping({ .variableType = variableType, .mapping = mapping }));
				continue;
			}

			if (psVars.contains(varName))
			{
				MaterialVariablesTypes variableType = StringToMaterialVariablesTypes.at(mappedValue.at("variableType"));
				ShaderConstantsBufferVariable& mapping = psVars.at(varName);
				variablesMapping.insert_or_assign(varName, MaterialVariableMapping({ .variableType = variableType, .mapping = mapping }));
				continue;
			}
		}

		auto constantsBufferContains = [this](std::string varName)
			{
				ShaderConstantsBufferVariablesMap& vsVars = vertexShader->constantsBuffersVariables;
				ShaderConstantsBufferVariablesMap& psVars = pixelShader->constantsBuffersVariables;

				return vsVars.contains(varName) || psVars.contains(varName);
			};

		auto getConstantsBufferVariable = [this](std::string varName)
			{
				ShaderConstantsBufferVariablesMap& vsVars = vertexShader->constantsBuffersVariables;
				ShaderConstantsBufferVariablesMap& psVars = pixelShader->constantsBuffersVariables;
				return (vsVars.contains(varName) ? vsVars.at(varName) : psVars.at(varName));
			};

		std::set<std::string> unmapped;
		for (auto& [varName, mappedValue] : mappedValues)
		{
			if (!constantsBufferContains(varName)) continue;

			auto def = getConstantsBufferVariable(varName);

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
			if (!constantsBufferContains(varName)) continue;

			//auto& matdef = material.at("mappedValues").at(matVarIndex.at(varName));
			auto& matdef = mappedValues.at(varName);
			auto& variableType = StringToMaterialVariablesTypes.at(std::string(matdef.at("variableType")));
			auto mapping = getConstantsBufferVariable(varName);

			variablesMapping.insert_or_assign(varName, MaterialVariableMapping({ .variableType = variableType, .mapping = mapping }));
		}
	}

	//READ&GET
	bool MaterialInstance::ShaderInstanceHasRegister(std::function<int(std::shared_ptr<ShaderInstance>&)> getRegister)
	{
		return (getRegister(vertexShader) != -1) || (getRegister(pixelShader) != -1);
	}

	void MaterialInstance::SetUAVRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& slot)
	{
		for (auto& [name, uavParam] : pixelShader->uavParameters)
		{
			if (uav.contains(uavParam.registerId))
			{
				commandList->SetGraphicsRootDescriptorTable(slot, uav.at(uavParam.registerId));
				slot++;
			}
		}
	}

	void MaterialInstance::SetSRVRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& slot)
	{
		for (auto& [textureType, texParam] : pixelShader->srvTexParameters)
		{
			if (texParam.numSRV == 0xFFFFFFFF || iblUsageTexture.contains(textureType)) continue;
			commandList->SetGraphicsRootDescriptorTable(slot, textures.at(textureType)->gpuHandle);
			slot++;
		}
	}

	void DestroyMaterialInstance(std::shared_ptr<MaterialInstance>& materialInstance)
	{
		RemoveMaterialInstance(materialInstance->instanceUUID, materialInstance);
	}
}
