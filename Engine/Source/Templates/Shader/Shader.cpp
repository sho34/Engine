#include "pch.h"
#include "Shader.h"
#include "../../Shaders/Compiler/ShaderCompiler.h"
#include <NoStd.h>
#include <nlohmann/json.hpp>
#if defined(_EDITOR)
#include "../../Editor/Editor.h"
#else
#include "../Templates.h"
#include "../../Scene/Scene.h"
#endif
#include <Application.h>
#include <Editor.h>
#include "../TemplateDef.h"

namespace Templates {

	TEMPDEF_TUPLE(Shader);
	TEMPDEF_GETTEMPLATES(Shader);

	namespace Shader
	{
#if defined(_EDITOR)
		nlohmann::json creationJson;
		unsigned int popupModalId = 0U;
#endif
		static nostd::RefTracker<Source, std::shared_ptr<ShaderInstance>> refTracker;
		ShaderIncludesDependencies dependencies;
	};

	void ShaderInstance::CopyFrom(std::shared_ptr<ShaderInstance>& src) {
		shaderSource = src->shaderSource;
		vsSemantics = src->vsSemantics;
		constantsBuffersParameters = src->constantsBuffersParameters;
		constantsBuffersVariables = src->constantsBuffersVariables;
		uavParameters = src->uavParameters;
		srvCSParameters = src->srvCSParameters;
		srvTexParameters = src->srvTexParameters;
		samplersParameters = src->samplersParameters;
		cbufferSize = src->cbufferSize;
		cameraCBVRegister = src->cameraCBVRegister;
		lightCBVRegister = src->lightCBVRegister;
		animationCBVRegister = src->animationCBVRegister;
		lightsShadowMapCBVRegister = src->lightsShadowMapCBVRegister;
		lightsShadowMapSRVRegister = src->lightsShadowMapSRVRegister;
		byteCode = src->byteCode;
	}

	void ShaderInstance::CreateVSSemantics(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc)
	{
		if (shaderSource.shaderType != VERTEX_SHADER) return;
		for (unsigned int paramIdx = 0; paramIdx < desc.InputParameters; paramIdx++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC signatureDesc{};
			reflection->GetInputParameterDesc(paramIdx, &signatureDesc);
			vsSemantics.push_back(signatureDesc.SemanticName);
		}
	}

	void ShaderInstance::CreateResourcesBinding(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc)
	{
		using namespace Animation;
		using namespace Scene;

		for (unsigned int paramIdx = 0; paramIdx < desc.BoundResources; paramIdx++)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
			reflection->GetResourceBindingDesc(paramIdx, &bindDesc);

			std::string resourceName(bindDesc.Name);

			cameraCBVRegister = (resourceName == CameraConstantBufferName) ? bindDesc.BindPoint : cameraCBVRegister;
			lightCBVRegister = (resourceName == LightConstantBufferName) ? bindDesc.BindPoint : lightCBVRegister;
			animationCBVRegister = (resourceName == AnimationConstantBufferName) ? bindDesc.BindPoint : animationCBVRegister;
			lightsShadowMapCBVRegister = (resourceName == ShadowMapConstantBufferName) ? bindDesc.BindPoint : lightsShadowMapCBVRegister;
			lightsShadowMapSRVRegister = (resourceName == ShadowMapLightsShaderResourveViewName) ? bindDesc.BindPoint : lightsShadowMapSRVRegister;

			if (bindDesc.Type == D3D_SIT_CBUFFER)
			{
				constantsBuffersParameters.insert_or_assign(resourceName, ShaderConstantsBufferParameter({ .registerId = bindDesc.BindPoint, .numConstantsBuffers = bindDesc.BindCount }));
			}
			else if (bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED)
			{
				uavParameters.insert_or_assign(resourceName, ShaderUAVParameter({ .registerId = bindDesc.BindPoint, .numUAV = bindDesc.BindCount }));
			}
			else if (bindDesc.Type == D3D_SIT_STRUCTURED)
			{
				srvCSParameters.insert_or_assign(resourceName, ShaderSRVParameter({ .registerId = bindDesc.BindPoint, .numSRV = bindDesc.BindCount }));
			}
			else if (bindDesc.Type == D3D_SIT_TEXTURE)
			{
				srvTexParameters.insert_or_assign(strToTextureType.at(resourceName), ShaderSRVParameter({ .registerId = bindDesc.BindPoint, .numSRV = bindDesc.BindCount > 0 ? bindDesc.BindCount : bindDesc.NumSamples })); //if N > 0 -> N else -1
#if defined(_EDITOR)
				TextureType textureType = strToTextureType.at(resourceName);
				if (materialTexturesTypes.contains(textureType))
				{
					auto& textureTypes = std::get<3>(GetShaderTemplates().at(shaderSource.shaderUUID));
					textureTypes.insert(textureType);
				}
#endif
			}
			else if (bindDesc.Type == D3D_SIT_SAMPLER)
			{
				samplersParameters.insert_or_assign(resourceName, ShaderSamplerParameter({ .registerId = bindDesc.BindPoint, .numSamplers = bindDesc.BindCount }));
#if defined(_EDITOR)
				auto& numSamplers = std::get<4>(GetShaderTemplates().at(shaderSource.shaderUUID));
				numSamplers++;
#endif
			}
		}
	}

	void ShaderInstance::CreateConstantsBuffersVariables(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc)
	{
		using namespace Animation;
		using namespace Scene;
		using namespace Templates;

		for (unsigned int paramIdx = 0; paramIdx < desc.ConstantBuffers; paramIdx++)
		{
			auto cbReflection = reflection->GetConstantBufferByIndex(paramIdx);
			D3D12_SHADER_BUFFER_DESC paramDesc{};
			cbReflection->GetDesc(&paramDesc);

			std::string cbufferName(paramDesc.Name);
			bool isCameraParam = cbufferName == CameraConstantBufferName;
			bool isLightParam = cbufferName == LightConstantBufferName;
			bool isAnimationParam = cbufferName == AnimationConstantBufferName;
			bool isShadowMapParam = cbufferName == ShadowMapConstantBufferName;
			if (isCameraParam || isLightParam || isAnimationParam || isShadowMapParam) continue;

			bool skipVar = false;
			for (unsigned int varIdx = 0; varIdx < paramDesc.Variables; varIdx++)
			{
				ID3D12ShaderReflectionVariable* varReflection = cbReflection->GetVariableByIndex(varIdx);
				D3D12_SHADER_VARIABLE_DESC varDesc;
				varReflection->GetDesc(&varDesc);

				if (!(varDesc.uFlags & D3D_SVF_USED)) continue;

				ID3D12ShaderReflectionType* varType = varReflection->GetType();
				D3D12_SHADER_TYPE_DESC varTypeDesc;
				varType->GetDesc(&varTypeDesc);

				if (!HLSLVariableClassAllowedTypes.contains(varTypeDesc.Class))
				{
					skipVar = true;
					break;
				}

				std::string varName(varDesc.Name);
				constantsBuffersVariables.insert_or_assign(varName, ShaderConstantsBufferVariable({ .bufferIndex = paramIdx, .size = varDesc.Size, .offset = varDesc.StartOffset }));

				std::string varClass = varTypeDesc.Name;
				bool useNamePattern = false;
				for (auto it = HLSLVariablePatternToMaterialVariableTypes.begin(); it != HLSLVariablePatternToMaterialVariableTypes.end(); it++)
				{
					std::string tieClass = std::get<0>(it->first);
					if (varClass != tieClass) continue;

					std::string tiePattern = std::get<1>(it->first);

					std::string lowerVarName = varName;
					nostd::ToLower(lowerVarName);

					if (lowerVarName.find(tiePattern) == std::string::npos) continue;

#if defined(_EDITOR)
					SetShaderMappedVariable(shaderSource.shaderUUID, varName, it->second);
#endif

					useNamePattern = true;
					break;
				}

				if (useNamePattern) continue;

#if defined(_EDITOR)
				SetShaderMappedVariable(shaderSource.shaderUUID, varName, HLSLVariableClassToMaterialVariableTypes.at(varClass));
#endif

			}
			if (!skipVar)
			{
				cbufferSize.push_back(paramDesc.Size);
			}
		}
	}

	void ShaderInstance::CreateByteCode(const ComPtr<IDxcResult>& result)
	{
		ComPtr<IDxcBlob> code;
		result->GetResult(&code);

		byteCode.resize(code->GetBufferSize());
		memcpy_s(byteCode.data(), code->GetBufferSize(), code->GetBufferPointer(), code->GetBufferSize());
	}

	void ShaderInstance::BindChange(std::function<void()> changeListener)
	{
		changesCallbacks.push_back(changeListener);
	}

	void ShaderInstance::NotifyChanges()
	{
		for (auto cb : changesCallbacks)
		{
			cb();
		}
	}

	//CREATE
	void CreateShader(nlohmann::json json)
	{
		std::string uuid = json.at("uuid");

		if (GetShaderTemplates().contains("uuid"))
		{
			assert(!!!"shader creation collision");
		}
		ShaderTemplate t;

		std::string& name = std::get<0>(t);
		name = json.at("name");

		nlohmann::json& data = std::get<1>(t);
		data = json;
		data.erase("name");
		data.erase("uuid");

		GetShaderTemplates().insert_or_assign(uuid, t);
	}

	//READ&GET
	TEMPDEF_GET(Shader);

#if defined(_EDITOR)
	std::map<std::string, MaterialVariablesTypes> GetShaderMappeableVariables(std::string uuid)
	{
		return std::get<2>(GetShaderTemplates().at(uuid));
	}

	std::set<TextureType> GetShaderTextureParameters(std::string uuid)
	{
		return std::get<3>(GetShaderTemplates().at(uuid));
	}

	unsigned int GetShaderSamplerParameters(std::string uuid)
	{
		return std::get<4>(GetShaderTemplates().at(uuid));
	}
#endif

	std::shared_ptr<ShaderInstance> GetShaderInstance(Source params)
	{
		using namespace Shader;
		return refTracker.AddRef(params, [&params]()
			{
				using namespace ShaderCompiler;

				std::shared_ptr<ShaderInstance> newInstance;
				size_t hash = std::hash<Source>()(params);
				newInstance = LoadShaderInstanceFromBinary(hash);

#if defined(_DEVELOPMENT)
				bool saveBinary = false;
				if (newInstance == nullptr)
				{
					newInstance = Compile(params, dependencies);
					saveBinary = true;
				}
#endif

				if (newInstance != nullptr)
				{
#if defined(_DEVELOPMENT)
					if (saveBinary)
					{
						//SaveShaderInstanceFromBinary(newInstance);
					}
#endif
					return newInstance;
				}
				else
				{
					return refTracker.FindValue(params);
				}
			}
		);
	}

	std::string GetShaderBinaryFileName(size_t hash)
	{
		return std::to_string(hash) +
#if defined(_DEBUG)
			"_d" +
#endif
			".bin";
	}

	std::shared_ptr<ShaderInstance> LoadShaderInstanceFromBinary(size_t hash)
	{
		std::filesystem::path shaderFile = defaultShadersBinariesFolder + GetShaderBinaryFileName(hash);

		if (!std::filesystem::exists(shaderFile))
		{
			return nullptr;
		}

		std::ifstream file(shaderFile, std::ios_base::binary);
		std::shared_ptr<ShaderInstance> instance = std::make_shared<ShaderInstance>();
		LoadShaderSource(file, instance->shaderSource);
		LoadShaderVSSemantics(file, instance->vsSemantics);
		LoadShaderConstantsBufferParameters(file, instance->constantsBuffersParameters);
		LoadShaderConstantsBufferVariables(file, instance->constantsBuffersVariables);
		LoadShaderComputeParameters(file, instance->srvCSParameters);
		LoadShaderTextureParameters(file, instance->srvTexParameters);
		LoadShaderSamplerParameters(file, instance->samplersParameters);
		LoadShaderBufferSizes(file, instance->cbufferSize);
		LoadShaderCBVRegister(file, instance->cameraCBVRegister);
		LoadShaderCBVRegister(file, instance->lightCBVRegister);
		LoadShaderCBVRegister(file, instance->animationCBVRegister);
		LoadShaderCBVRegister(file, instance->lightsShadowMapCBVRegister);
		LoadShaderCBVRegister(file, instance->lightsShadowMapSRVRegister);
		LoadShaderByteCode(file, instance->byteCode);
		file.close();
		return instance;
	}

#if defined(_DEVELOPMENT)
	void SaveShaderInstanceFromBinary(std::shared_ptr<ShaderInstance> instance)
	{
		//first create the directory if needed
		std::filesystem::path directory(defaultShadersBinariesFolder);
		std::filesystem::create_directory(directory);

		size_t hash = std::hash<Source>()(instance->shaderSource);
		std::filesystem::path shaderFile = directory;
		shaderFile.append(GetShaderBinaryFileName(hash));

		std::ofstream file(shaderFile, std::ios_base::binary);
		WriteShaderSource(file, instance->shaderSource);
		WriteShaderVSSemantics(file, instance->vsSemantics);
		WriteShaderConstantsBufferParameters(file, instance->constantsBuffersParameters);
		WriteShaderConstantsBufferVariables(file, instance->constantsBuffersVariables);
		WriteShaderComputeParameters(file, instance->srvCSParameters);
		WriteShaderTextureParameters(file, instance->srvTexParameters);
		WriteShaderSamplerParameters(file, instance->samplersParameters);
		WriteShaderBufferSizes(file, instance->cbufferSize);
		WriteShaderCBVRegister(file, instance->cameraCBVRegister);
		WriteShaderCBVRegister(file, instance->lightCBVRegister);
		WriteShaderCBVRegister(file, instance->animationCBVRegister);
		WriteShaderCBVRegister(file, instance->lightsShadowMapCBVRegister);
		WriteShaderCBVRegister(file, instance->lightsShadowMapSRVRegister);
		WriteShaderByteCode(file, instance->byteCode);
		file.close();
	}
#endif

	void LoadShaderSource(std::ifstream& file, Source& source)
	{
		file.read(reinterpret_cast<char*>(&source.shaderType), sizeof(source.shaderType));
		source.shaderUUID.resize(36);
		file.read(source.shaderUUID.data(), 36);
		source.defines = nostd::readVectorFromIfstream(file);
	}

#if defined(_DEVELOPMENT)
	void WriteShaderSource(std::ofstream& file, Source& source)
	{
		file.write(reinterpret_cast<char*>(&source.shaderType), sizeof(source.shaderType));
		file.write(source.shaderUUID.c_str(), source.shaderUUID.size());
		nostd::writeVectorToOfsream(file, source.defines);
	}
#endif

	void LoadShaderVSSemantics(std::ifstream& file, std::vector<std::string>& vsSemantics)
	{
		vsSemantics = nostd::readVectorFromIfstream(file);
	}

#if defined(_DEVELOPMENT)
	void WriteShaderVSSemantics(std::ofstream& file, std::vector<std::string>& vsSemantics)
	{
		nostd::writeVectorToOfsream(file, vsSemantics);
	}
#endif

	void LoadShaderConstantsBufferParameters(std::ifstream& file, ShaderConstantsBufferParametersMap& constantsBuffersParameters)
	{
		nostd::loadMapFromIfstream(file, constantsBuffersParameters, [](std::ifstream& file)
			{
				size_t size;
				file.read(reinterpret_cast<char*>(&size), sizeof(size));
				ShaderConstantsBufferParametersPair pair;
				pair.first.resize(size);
				file.read(pair.first.data(), size);
				file.read(reinterpret_cast<char*>(&pair.second.registerId), sizeof(pair.second.registerId));
				file.read(reinterpret_cast<char*>(&pair.second.numConstantsBuffers), sizeof(pair.second.numConstantsBuffers));
				return pair;
			}
		);
	}

#if defined(_DEVELOPMENT)
	void WriteShaderConstantsBufferParameters(std::ofstream& file, ShaderConstantsBufferParametersMap& constantsBuffersParameters)
	{
		nostd::writeMapToOfstream(file, constantsBuffersParameters, [](std::ofstream& file, auto& pair)
			{
				std::string str = pair.first;
				size_t size = str.size();
				file.write(reinterpret_cast<char*>(&size), sizeof(size));
				file.write(str.c_str(), size);
				file.write(reinterpret_cast<char*>(&pair.second.registerId), sizeof(pair.second.registerId));
				file.write(reinterpret_cast<char*>(&pair.second.numConstantsBuffers), sizeof(pair.second.numConstantsBuffers));
			}
		);
	}
#endif

	void LoadShaderConstantsBufferVariables(std::ifstream& file, ShaderConstantsBufferVariablesMap& constantsBuffersVariables)
	{
		nostd::loadMapFromIfstream(file, constantsBuffersVariables, [](std::ifstream& file)
			{
				size_t size;
				file.read(reinterpret_cast<char*>(&size), sizeof(size));
				ShaderConstantsBufferVariablesPair pair;
				pair.first.resize(size);
				file.read(pair.first.data(), size);
				file.read(reinterpret_cast<char*>(&pair.second.bufferIndex), sizeof(pair.second.bufferIndex));
				file.read(reinterpret_cast<char*>(&pair.second.size), sizeof(pair.second.size));
				file.read(reinterpret_cast<char*>(&pair.second.offset), sizeof(pair.second.offset));
				return pair;
			}
		);
	}

#if defined(_DEVELOPMENT)
	void WriteShaderConstantsBufferVariables(std::ofstream& file, ShaderConstantsBufferVariablesMap& constantsBuffersVariables)
	{
		nostd::writeMapToOfstream(file, constantsBuffersVariables, [](std::ofstream& file, auto& pair)
			{
				std::string str = pair.first;
				size_t size = str.size();
				file.write(reinterpret_cast<char*>(&size), sizeof(size));
				file.write(str.c_str(), size);
				file.write(reinterpret_cast<char*>(&pair.second.bufferIndex), sizeof(pair.second.bufferIndex));
				file.write(reinterpret_cast<char*>(&pair.second.size), sizeof(pair.second.size));
				file.write(reinterpret_cast<char*>(&pair.second.offset), sizeof(pair.second.offset));
			}
		);
	}
#endif

	void LoadShaderComputeParameters(std::ifstream& file, ShaderSRVCSParametersMap& srvCSParameters)
	{
		nostd::loadMapFromIfstream(file, srvCSParameters, [](std::ifstream& file)
			{
				size_t size;
				file.read(reinterpret_cast<char*>(&size), sizeof(size));
				ShaderSRVCSParametersPair pair;
				pair.first.resize(size);
				file.read(pair.first.data(), size);
				file.read(reinterpret_cast<char*>(&pair.second.registerId), sizeof(pair.second.registerId));
				file.read(reinterpret_cast<char*>(&pair.second.numSRV), sizeof(pair.second.numSRV));
				return pair;
			}
		);
	}

#if defined(_DEVELOPMENT)
	void WriteShaderComputeParameters(std::ofstream& file, ShaderSRVCSParametersMap& srvCSParameters)
	{
		nostd::writeMapToOfstream(file, srvCSParameters, [](std::ofstream& file, auto& pair)
			{
				std::string str = pair.first;
				size_t size = str.size();
				file.write(reinterpret_cast<char*>(&size), sizeof(size));
				file.write(str.c_str(), size);
				file.write(reinterpret_cast<char*>(&pair.second.registerId), sizeof(pair.second.registerId));
				file.write(reinterpret_cast<char*>(&pair.second.numSRV), sizeof(pair.second.numSRV));
			}
		);
	}
#endif

	void LoadShaderTextureParameters(std::ifstream& file, ShaderSRVTexParametersMap& srvTexParameters)
	{
		nostd::loadMapFromIfstream(file, srvTexParameters, [](std::ifstream& file)
			{
				ShaderSRVTexParametersPair pair;
				file.read(reinterpret_cast<char*>(&pair.first), sizeof(pair.first));
				file.read(reinterpret_cast<char*>(&pair.second.registerId), sizeof(pair.second.registerId));
				file.read(reinterpret_cast<char*>(&pair.second.numSRV), sizeof(pair.second.numSRV));
				return pair;
			}
		);
	}

#if defined(_DEVELOPMENT)
	void WriteShaderTextureParameters(std::ofstream& file, ShaderSRVTexParametersMap& srvParameters)
	{
		nostd::writeMapToOfstream(file, srvParameters, [](std::ofstream& file, auto& pair)
			{
				TextureType type = pair.first;
				file.write(reinterpret_cast<char*>(&type), sizeof(type));
				file.write(reinterpret_cast<char*>(&pair.second.registerId), sizeof(pair.second.registerId));
				file.write(reinterpret_cast<char*>(&pair.second.numSRV), sizeof(pair.second.numSRV));
			}
		);
	}
#endif

	void LoadShaderSamplerParameters(std::ifstream& file, ShaderSamplerParametersMap& samplersParameters)
	{
		nostd::loadMapFromIfstream(file, samplersParameters, [](std::ifstream& file)
			{
				size_t size;
				file.read(reinterpret_cast<char*>(&size), sizeof(size));
				ShaderSamplerParametersPair pair;
				pair.first.resize(size);
				file.read(pair.first.data(), size);
				file.read(reinterpret_cast<char*>(&pair.second.registerId), sizeof(pair.second.registerId));
				file.read(reinterpret_cast<char*>(&pair.second.numSamplers), sizeof(pair.second.numSamplers));
				return pair;
			}
		);
	}

#if defined(_DEVELOPMENT)
	void WriteShaderSamplerParameters(std::ofstream& file, ShaderSamplerParametersMap& samplersParameters)
	{
		nostd::writeMapToOfstream(file, samplersParameters, [](std::ofstream& file, auto& pair)
			{
				std::string str = pair.first;
				size_t size = str.size();
				file.write(reinterpret_cast<char*>(&size), sizeof(size));
				file.write(str.c_str(), size);
				file.write(reinterpret_cast<char*>(&pair.second.registerId), sizeof(pair.second.registerId));
				file.write(reinterpret_cast<char*>(&pair.second.numSamplers), sizeof(pair.second.numSamplers));
			}
		);
	}
#endif

	void LoadShaderBufferSizes(std::ifstream& file, std::vector<size_t>& cbufferSize)
	{
		size_t size;
		file.read(reinterpret_cast<char*>(&size), sizeof(size));
		if (size > 0ULL)
		{
			cbufferSize.resize(size);
			file.read(reinterpret_cast<char*>(cbufferSize.data()), size * sizeof(cbufferSize[0]));
		}
	}

#if defined(_DEVELOPMENT)
	void WriteShaderBufferSizes(std::ofstream& file, std::vector<size_t>& cbufferSize)
	{
		size_t size = cbufferSize.size();
		file.write(reinterpret_cast<char*>(&size), sizeof(size));
		if (size > 0ULL)
		{
			file.write(reinterpret_cast<char*>(cbufferSize.data()), sizeof(cbufferSize[0]) * size);
		}
	}
#endif

	void LoadShaderCBVRegister(std::ifstream& file, int& reg)
	{
		file.read(reinterpret_cast<char*>(&reg), sizeof(reg));
	}

#if defined(_DEVELOPMENT)
	void WriteShaderCBVRegister(std::ofstream& file, int& reg)
	{
		file.write(reinterpret_cast<char*>(&reg), sizeof(reg));
	}
#endif

	void LoadShaderByteCode(std::ifstream& file, ShaderByteCode& byteCode)
	{
		size_t size;
		file.read(reinterpret_cast<char*>(&size), sizeof(size));
		if (size > 0ULL)
		{
			byteCode.resize(size);
			file.read(reinterpret_cast<char*>(byteCode.data()), size * sizeof(byteCode[0]));
		}
	}

#if defined(_DEVELOPMENT)
	void WriteShaderByteCode(std::ofstream& file, ShaderByteCode& byteCode)
	{
		size_t size = byteCode.size();
		file.write(reinterpret_cast<char*>(&size), sizeof(size));
		if (size > 0ULL)
		{
			file.write(reinterpret_cast<char*>(byteCode.data()), sizeof(byteCode[0]) * size);
		}
	}
#endif

	TEMPDEF_GETNAMES(Shader);
	TEMPDEF_GETNAME(Shader);
	TEMPDEF_FINDUUIDBYNAME(Shader);
	TEMPDEF_GETUUIDNAMES(Shader);
	TEMPDEF_RELEASE(Shader);

	std::vector<UUIDName> GetShadersUUIDsNamesByType(ShaderType type)
	{
		std::map<std::string, ShaderTemplate> shadersByType;
		std::copy_if(GetShaderTemplates().begin(), GetShaderTemplates().end(), std::inserter(shadersByType, shadersByType.end()), [type](auto pair)
			{
				nlohmann::json& json = std::get<1>(pair.second);
				return StrToShaderType.at(json.at("type")) == type;
			}
		);
		return GetUUIDsNames(shadersByType);
	}

	//UPDATE
#if defined(_EDITOR)

	void SetShaderMappedVariable(std::string uuid, std::string varName, MaterialVariablesTypes type)
	{
		auto& variables = std::get<2>(GetShaderTemplates().at(uuid));
		variables.insert_or_assign(varName, type);
	}
#endif

	void BuildShader(std::string shaderFile)
	{
		using namespace Shader;
		std::vector<std::shared_ptr<ShaderInstance>> binaries;
		for (auto it = refTracker.instances.begin(); it != refTracker.instances.end(); it++)
		{
			Source source = it->first;
			std::shared_ptr<ShaderInstance> binary = it->second;

			nlohmann::json json = GetShaderTemplate(source.shaderUUID);
			std::string path = json.at("path");
			if (shaderFile != path) continue;

			using namespace ShaderCompiler;
			std::shared_ptr<ShaderInstance> test = Compile(source, dependencies);
			if (test == nullptr)
			{
				OutputDebugStringA((std::string("Error found compiling ") + source.to_string()).c_str());
				return;
			}

			binaries.push_back(binary);
		}

		for (auto& binary : binaries)
		{
			binary->NotifyChanges();
		}
	}

	void BuildShaderFromDependency(std::string dependency)
	{
		using namespace Shader;
		std::set<std::string> shaderFiles;
		for (auto& [src, deps] : dependencies)
		{
			//if the file is not in the dependency skip
			if (!deps.contains(dependency)) continue;

			//get the hlsl and skip if already built
			nlohmann::json json = GetShaderTemplate(src.shaderUUID);
			std::string shaderFile = json.at("path");
			if (shaderFiles.contains(shaderFile)) continue;

			//build
			shaderFiles.insert(shaderFile);
			BuildShader(shaderFile);
		}
	}

	static CompilerQueue changesQueue;
	void MonitorShaderChanges(std::string folder) {

		std::thread monitor([folder]()
			{
				HANDLE file = CreateFileA(folder.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);

				if (file == INVALID_HANDLE_VALUE || file == NULL) {
					OutputDebugStringA("ERROR: Invalid HANDLE value.\n");
					ExitProcess(GetLastError());
				}

				OVERLAPPED overlapped;
				overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

				uint8_t change_buf[1024];
				BOOL success = ReadDirectoryChangesW(
					file, change_buf, 1024, TRUE,
					FILE_NOTIFY_CHANGE_LAST_WRITE,
					NULL, &overlapped, NULL);

				while (true) {
					DWORD result = WaitForSingleObject(overlapped.hEvent, 0);

					if (result == WAIT_OBJECT_0) {
						DWORD bytes_transferred;
						GetOverlappedResult(file, &overlapped, &bytes_transferred, FALSE);

						FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)change_buf;

						for (;;) {
							DWORD name_len = event->FileNameLength / sizeof(wchar_t);

							switch (event->Action) {
							case FILE_ACTION_MODIFIED: {
								std::string shaderFile = nostd::WStringToString(std::wstring(event->FileName, event->FileNameLength / sizeof(event->FileName[0])));
								changesQueue.push(shaderFile);
							} break;
							default: {
								printf("Unknown action!\n");
							} break;
							}

							// Are there more events to handle?
							if (event->NextEntryOffset) {
								*((uint8_t**)&event) += event->NextEntryOffset;
							}
							else {
								break;
							}
						}
					}
					// Queue the next event
					BOOL success = ReadDirectoryChangesW(
						file, change_buf, 1024, TRUE,
						FILE_NOTIFY_CHANGE_LAST_WRITE,
						NULL, &overlapped, NULL);
				}
			}
		);

		std::thread compilation([]()
			{
				using namespace std::chrono_literals;
				while (TRUE)
				{
					while (changesQueue.size() > 0)
					{

						std::filesystem::path filePath(changesQueue.front());
						std::string fileExtension = filePath.extension().string();

						std::string output = "Modified: " + filePath.string() + "\n";
						OutputDebugStringA(output.c_str());

						if (!_stricmp(fileExtension.c_str(), ".hlsl")) {
							BuildShader(filePath.stem().string().c_str());
						}
						else if (!_stricmp(fileExtension.c_str(), ".h")) {
							BuildShaderFromDependency(filePath.string().c_str());
						}

						changesQueue.pop();
					}
					std::this_thread::sleep_for(200ms);
				}
			}
		);

		monitor.detach();
		compilation.detach();
	}

	//DELETE
	void DestroyShaderBinary(std::shared_ptr<ShaderInstance>& shaderBinary)
	{
		using namespace Shader;
		Source key = refTracker.FindKey(shaderBinary);
		refTracker.RemoveRef(key, shaderBinary);
	}

	//EDITOR
#if defined(_EDITOR)

	void DrawShaderPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop)
	{
		std::string tableName = "shader-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			Shader::DrawEditorInformationAttributes(uuid);
			Shader::DrawEditorAssetAttributes(uuid);
			ImGui::EndTable();
		}
	}

	void Shader::DrawEditorInformationAttributes(std::string uuid)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "shader-information-atts";

		ShaderTemplate& t = GetShaderTemplates().at(uuid);

		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string& currentName = std::get<0>(t);
			nlohmann::json& json = std::get<1>(t);
			int inputTextFlag = (json.contains("systemCreated") && json.at("systemCreated") == true) ? ImGuiInputTextFlags_ReadOnly : 0;
			ImGui::InputText("name", &currentName, inputTextFlag);
			ImGui::EndTable();
		}
	}

	void Shader::DrawEditorAssetAttributes(std::string uuid)
	{
		ShaderTemplate& t = GetShaderTemplates().at(uuid);

		std::string name = std::get<0>(t);
		nlohmann::json& json = std::get<1>(t);
		std::string fileName = (json.contains("path") ? std::string(json.at("path")) : name);
		bool fileSelectionEnabled = (!json.contains("systemCreated") || json.at("systemCreated") == false);
		ImDrawFileSelector("##", fileName, [&json](std::filesystem::path shaderPath)
			{
				std::filesystem::path hlslFilePath = shaderPath;
				hlslFilePath.replace_extension(".hlsl");
				json.at("path") = hlslFilePath.stem().string();
			},
			defaultShadersFolder, "Shader files. (*.hlsl)", "*.hlsl", fileSelectionEnabled
		);

		ImGui::PushID("shader-type");
		{
			ImGui::Text("Type");
			if (fileSelectionEnabled)
			{
				drawFromCombo(json, "type", StrToShaderType);
			}
			else
			{
				std::string type = std::string(json.at("type"));
				ImGui::InputText("##", type.data(), type.size(), ImGuiInputTextFlags_ReadOnly);
			}
		}
		ImGui::PopID();

		ImDrawMappedValues(json, GetShaderMappeableVariables(uuid));
	}

	void AttachMaterialToShader(std::string uuid, std::string materialUUID)
	{
		auto& matRef = std::get<5>(GetShaderTemplates().at(uuid));
		matRef.push_back(materialUUID);
	}

	void DetachMaterialsFromShader(std::string uuid)
	{
		for (auto& materialUUID : std::get<5>(GetShaderTemplates().at(uuid)))
		{
			DetachShader(materialUUID);
		}
	}

	void CreateNewShader()
	{
		Shader::popupModalId = ShaderPopupModal_CreateNew;
		Shader::creationJson = nlohmann::json(
			{
				{ "name", "" },
				{ "path", "" },
				{ "type", ShaderTypeToStr.at(VERTEX_SHADER) },
				{ "uuid", getUUID() }
			}
		);
	}

	void DeleteShader(std::string uuid)
	{
		nlohmann::json json = GetShaderTemplate(uuid);
		if (json.contains("systemCreated") && json.at("systemCreated") == true)
		{
			Shader::popupModalId = ShaderPopupModal_CannotDelete;
			return;
		}

		DetachMaterialsFromShader(uuid);
		GetShaderTemplates().erase(uuid);
	}

	void DrawShadersPopups()
	{
		Editor::DrawOkPopup(Shader::popupModalId, ShaderPopupModal_CannotDelete, "Cannot delete shader", []
			{
				ImGui::Text("Cannot delete a system created shader");
			}
		);

		Editor::DrawCreateWindow(Shader::popupModalId, ShaderPopupModal_CreateNew, "Create new shader", [](auto OnCancel)
			{
				nlohmann::json& json = Shader::creationJson;

				ImGui::PushID("shader-name");
				{
					ImGui::Text("Name");
					ImDrawJsonInputText(json, "name");
				}
				ImGui::PopID();

				ImGui::PushID("shader-type");
				{
					ImGui::Text("Type");
					drawFromCombo(json, "type", StrToShaderType);
				}
				ImGui::PopID();

				std::string parentFolder = defaultShadersFolder;

				ImGui::PushID("shader-path");
				{
					ImDrawJsonFilePicker(json, "path", parentFolder, "Shader files. (*.hlsl)", "*.hlsl");
				}
				ImGui::PopID();

				if (ImGui::Button("Cancel")) { OnCancel(); }
				ImGui::SameLine();

				std::vector<std::string> shaderNames = GetShadersNames();
				bool disabledCreate = json.at("path") == "" || json.at("name") == "" || std::find(shaderNames.begin(), shaderNames.end(), std::string(json.at("name"))) != shaderNames.end();

				if (disabledCreate)
				{
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}

				if (ImGui::Button("Create"))
				{
					Shader::popupModalId = 0;
					CreateShader(json);
				}

				if (disabledCreate)
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}
			}
		);
	}

	void WriteShadersJson(nlohmann::json& json)
	{
		WriteTemplateJson(json, GetShaderTemplates());
	}

#endif

}