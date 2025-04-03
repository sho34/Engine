#include "pch.h"
#include "Shader.h"
#include "../../Shaders/Compiler/ShaderCompiler.h"
#include <NoStd.h>
#include <nlohmann/json.hpp>
#if defined(_EDITOR)
#include "../../Editor/Editor.h"
#include <Application.h>
#endif
#include <Editor.h>

namespace Templates {

	//uuid to ShaderTemplates
	std::map<std::string, ShaderTemplate> shaders;

	namespace Shader
	{
#if defined(_EDITOR)
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
		texturesParameters = src->texturesParameters;
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
			else if (bindDesc.Type == D3D_SIT_TEXTURE)
			{
				texturesParameters.insert_or_assign(strToTextureType.at(resourceName), ShaderTextureParameter({ .registerId = bindDesc.BindPoint, .numTextures = bindDesc.BindCount > 0 ? bindDesc.BindCount : bindDesc.NumSamples })); //if N > 0 -> N else -1
#if defined(_EDITOR)
				TextureType textureType = strToTextureType.at(resourceName);
				if (materialTexturesTypes.contains(textureType))
				{
					auto& textureTypes = std::get<3>(shaders.at(shaderSource.shaderUUID));
					textureTypes.insert(textureType);
				}
#endif
			}
			else if (bindDesc.Type == D3D_SIT_SAMPLER)
			{
				samplersParameters.insert_or_assign(resourceName, ShaderSamplerParameter({ .registerId = bindDesc.BindPoint, .numSamplers = bindDesc.BindCount }));
#if defined(_EDITOR)
				auto& numSamplers = std::get<4>(shaders.at(shaderSource.shaderUUID));
				numSamplers++;
#endif
			}
		}
	}

	void ShaderInstance::CreateConstantsBuffersVariables(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc)
	{
		using namespace Animation;
		using namespace Scene;

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

			for (unsigned int varIdx = 0; varIdx < paramDesc.Variables; varIdx++)
			{
				ID3D12ShaderReflectionVariable* varReflection = cbReflection->GetVariableByIndex(varIdx);
				D3D12_SHADER_VARIABLE_DESC varDesc;
				varReflection->GetDesc(&varDesc);

				if (!(varDesc.uFlags & D3D_SVF_USED)) continue;

				std::string varName(varDesc.Name);
				constantsBuffersVariables.insert_or_assign(varName, ShaderConstantsBufferVariable({ .bufferIndex = paramIdx, .size = varDesc.Size, .offset = varDesc.StartOffset }));

				ID3D12ShaderReflectionType* varType = varReflection->GetType();
				D3D12_SHADER_TYPE_DESC varTypeDesc;
				varType->GetDesc(&varTypeDesc);

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

					SetShaderMappedVariable(shaderSource.shaderUUID, varName, it->second);

					useNamePattern = true;
					break;
				}

				if (useNamePattern) continue;

				SetShaderMappedVariable(shaderSource.shaderUUID, varName, HLSLVariableClassToMaterialVariableTypes.at(varClass));
			}
			cbufferSize.push_back(paramDesc.Size);
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

		if (shaders.contains("uuid"))
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

		shaders.insert_or_assign(uuid, t);
	}

	//READ&GET
	nlohmann::json GetShaderTemplate(std::string uuid)
	{
		return std::get<1>(shaders.at(uuid));
	}

#if defined(_EDITOR)
	std::map<std::string, MaterialVariablesTypes> GetShaderMappeableVariables(std::string uuid)
	{
		return std::get<2>(shaders.at(uuid));
	}

	std::set<TextureType> GetShaderTextureParameters(std::string uuid)
	{
		return std::get<3>(shaders.at(uuid));
	}

	unsigned int GetShaderSamplerParameters(std::string uuid)
	{
		return std::get<4>(shaders.at(uuid));
	}
#endif

	std::shared_ptr<ShaderInstance> GetShaderInstance(Source params)
	{
		using namespace Shader;
		return refTracker.AddRef(params, [&params]()
			{
				using namespace ShaderCompiler;
				std::shared_ptr<ShaderInstance> newInstance = Compile(params, dependencies);
				if (newInstance != nullptr)
				{
					return newInstance;
				}
				else
				{
					return refTracker.FindValue(params);
				}
			}
		);
	}

	std::vector<std::string> GetShadersNames() {
		return GetNames(shaders);
	}

	std::string GetShaderName(std::string uuid)
	{
		return std::get<0>(shaders.at(uuid));
	}

	std::vector<UUIDName> GetShadersUUIDsNames()
	{
		return GetUUIDsNames(shaders);
	}

	std::vector<UUIDName> GetShadersUUIDsNamesByType(ShaderType type)
	{
		std::map<std::string, ShaderTemplate> shadersByType;
		std::copy_if(shaders.begin(), shaders.end(), std::inserter(shadersByType, shadersByType.end()), [type](auto pair)
			{
				nlohmann::json& json = std::get<1>(pair.second);
				return StrToShaderType.at(json.at("type"));
			}
		);
		return GetUUIDsNames(shadersByType);
	}

	//UPDATE
#if defined(_EDITOR)

	void SetShaderMappedVariable(std::string uuid, std::string varName, MaterialVariablesTypes type)
	{
		auto& variables = std::get<2>(shaders.at(uuid));
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
	void ReleaseShaderTemplates()
	{
		shaders.clear();
	}

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

		ShaderTemplate& t = shaders.at(uuid);

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
		ShaderTemplate& t = shaders.at(uuid);

		std::string name = std::get<0>(t);
		nlohmann::json& json = std::get<1>(t);
		std::string fileName = (json.contains("path") ? std::string(json.at("path")) : name);
		bool fileSelectionEnabled = (json.contains("systemCreated") && json.at("systemCreated") == true);
		ImDrawFileSelector("##", fileName, [&json](std::filesystem::path shaderPath)
			{
				std::filesystem::path hlslFilePath = shaderPath;
				hlslFilePath.replace_extension(".hlsl");
				json.at("path") = hlslFilePath.stem().string();
			},
			defaultShadersFolder, "Shader files. (*.hlsl)", "*.hlsl", !fileSelectionEnabled
		);

		ImDrawMappedValues(json, GetShaderMappeableVariables(uuid));
	}

	void AttachMaterialToShader(std::string uuid, std::string materialUUID)
	{
		auto& matRef = std::get<5>(shaders.at(uuid));
		matRef.push_back(materialUUID);
	}

	void DetachMaterialsFromShader(std::string uuid)
	{
		for (auto& materialUUID : std::get<5>(shaders.at(uuid)))
		{
			DetachShader(materialUUID);
		}
	}

	void CreateNewShader()
	{
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
		shaders.erase(uuid);
	}

	void DrawShadersPopups()
	{
		Editor::DrawOkPopup(Shader::popupModalId, ShaderPopupModal_CannotDelete, "Cannot delete shader", []
			{
				ImGui::Text("Cannot delete a system created shader");
			}
		);
	}

	void WriteShadersJson(nlohmann::json& json)
	{
		WriteTemplateJson(json, shaders);
	}

#endif

}