#include "pch.h"
#include "Shader.h"
#include "../../Shaders/Compiler/ShaderCompiler.h"
#include "../../pch/NoStd.h"
#include <nlohmann/json.hpp>
#if defined(_EDITOR)
#include "../../Editor/Editor.h"
#include "../../pch/Application.h"
#endif

namespace Templates {

	std::map<std::string, nlohmann::json> shaderTemplates;

#if defined(_EDITOR)
	std::map<std::string, std::map<std::string, MaterialVariablesTypes>> shaderConstantVariablesTypes;
#endif

	nostd::RefTracker<Source, std::shared_ptr<ShaderInstance>> refTracker;
	ShaderIncludesDependencies dependencies;

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
				//texturesParameters.insert_or_assign(resourceName, ShaderTextureParameter({ .registerId = bindDesc.BindPoint, .numTextures = bindDesc.BindCount > 0 ? bindDesc.BindCount : bindDesc.NumSamples })); //if N > 0 -> N else -1
			}
			else if (bindDesc.Type == D3D_SIT_SAMPLER)
			{
				samplersParameters.insert_or_assign(resourceName, ShaderSamplerParameter({ .registerId = bindDesc.BindPoint, .numSamplers = bindDesc.BindCount }));
			}
		}
	}

	void ShaderInstance::CreateConstantsBuffersVariables(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc)
	{
		using namespace Animation;
		using namespace Scene;

		for (unsigned int paramIdx = 0; paramIdx < desc.ConstantBuffers; paramIdx++) {
			auto cbReflection = reflection->GetConstantBufferByIndex(paramIdx);
			D3D12_SHADER_BUFFER_DESC paramDesc{};
			cbReflection->GetDesc(&paramDesc);

			std::string cbufferName(paramDesc.Name);
			bool isCameraParam = cbufferName == CameraConstantBufferName;
			bool isLightParam = cbufferName == LightConstantBufferName;
			bool isAnimationParam = cbufferName == AnimationConstantBufferName;
			bool isShadowMapParam = cbufferName == ShadowMapConstantBufferName;
			if (isCameraParam || isLightParam || isAnimationParam || isShadowMapParam) continue;

			for (unsigned int varIdx = 0; varIdx < paramDesc.Variables; varIdx++) {
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
					std::transform(lowerVarName.begin(), lowerVarName.end(), lowerVarName.begin(),
						[](unsigned char c) { return std::tolower(c); });

					if (lowerVarName.find(tiePattern) == std::string::npos) continue;

					SetShaderMappedVariable(shaderSource.shaderName, varName, it->second);

					useNamePattern = true;
					break;
				}

				if (useNamePattern) continue;

				SetShaderMappedVariable(shaderSource.shaderName, varName, HLSLVariableClassToMaterialVariableTypes.at(varClass));
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

	//NOTIFICATIONS

	//CREATE
	void CreateShader(std::string name, nlohmann::json json)
	{
		if (shaderTemplates.contains(name)) return;
		shaderTemplates.insert_or_assign(name, json);
	}

	//READ&GET
	nlohmann::json GetShaderTemplate(std::string name)
	{
		return shaderTemplates.at(name);
	}

	std::shared_ptr<ShaderInstance> GetShaderInstance(Source params)
	{
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
		return nostd::GetKeysFromMap(shaderTemplates);
	}
	//UPDATE
	void BuildShader(std::string shaderFile)
	{
		for (auto it = refTracker.instances.begin(); it != refTracker.instances.end(); it++)
		{
			Source source = it->first;
			std::shared_ptr<ShaderInstance> instance = it->second;

			if (source.shaderName != shaderFile) continue;

			using namespace ShaderCompiler;
			std::shared_ptr<ShaderInstance> test = Compile(source, dependencies);
			if (test == nullptr)
			{
				OutputDebugStringA((std::string("Error found compiling ") + source.to_string()).c_str());
				return;
			}
		}

		for (auto& [source, binary] : refTracker.instances)
		{
			if (source.shaderName != shaderFile) continue;
			binary->NotifyChanges();
		}
	}

	void BuildShaderFromDependency(std::string dependency)
	{
		std::set<std::string> shaderFiles;
		for (auto& [src, deps] : dependencies) {
			if (deps.contains(dependency)) {
				shaderFiles.insert(src.shaderName);
			}
		}
		for (auto shaderFile : shaderFiles) {
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
		shaderTemplates.clear();
	}

	void DestroyShaderBinary(std::shared_ptr<ShaderInstance>& shaderBinary)
	{
		Source key = refTracker.FindKey(shaderBinary);
		refTracker.RemoveRef(key, shaderBinary);
	}

	//EDITOR
#if defined(_EDITOR)

	void SetShaderMappedVariable(std::string shaderName, std::string varName, MaterialVariablesTypes type)
	{
		shaderConstantVariablesTypes[shaderName].insert_or_assign(varName, type);
	}

	std::map<std::string, MaterialVariablesTypes> GetShaderMappeableVariables(std::string shaderName)
	{
		if (!shaderConstantVariablesTypes.contains(shaderName)) return std::map<std::string, MaterialVariablesTypes>();
		return shaderConstantVariablesTypes.at(shaderName);
	}



	void DrawShaderPanel(std::string& shader, ImVec2 pos, ImVec2 size, bool pop)
	{
		nlohmann::json& mat = shaderTemplates.at(shader);

		std::string fileName = mat.contains("fileName") ? std::string(mat.at("fileName")) : shader;
		if (ImGui::Button(ICON_FA_ELLIPSIS_H))
		{
			Editor::OpenFile([&mat](std::filesystem::path shaderPath)
				{
					std::filesystem::path hlslFilePath = shaderPath;
					hlslFilePath.replace_extension(".hlsl");
					mat.at("fileName") = hlslFilePath.stem().string();
				}
			, defaultShadersFolder, "Shader files. (*.hlsl)", "*.hlsl");
		}
		ImGui::SameLine();
		ImGui::InputText("fileName", fileName.data(), fileName.size(), ImGuiInputTextFlags_ReadOnly);

		std::set<std::string> existingVariables;
		if (mat.contains("mappedValues"))
		{
			unsigned int sz = static_cast<unsigned int>(mat.at("mappedValues").size());
			for (unsigned int i = 0; i < sz; i++)
			{
				existingVariables.insert(mat.at("mappedValues").at(i).at("variable"));
			}
		}

		std::vector<std::string> selectables = { " " };
		std::map<std::string, MaterialVariablesTypes> mappeables = GetShaderMappeableVariables(fileName.data());
		for (auto it = mappeables.begin(); it != mappeables.end(); it++)
		{
			if (existingVariables.contains(it->first)) continue;
			selectables.push_back(it->first);
		}

		//draw a combo for adding attributes to the object
		ImGui::Text("Mapped Values");
		if (selectables.size() > 1)
		{
			ImGui::PushID("mapped-values-add-combo");
			DrawComboSelection(selectables[0], selectables, [&mat, mappeables](std::string variable)
				{
					if (!mat.contains("mappedValues")) { mat["mappedValues"] = nlohmann::json::array(); }
					MaterialVariablesMappedJsonInitializer.at(mappeables.at(variable))(mat["mappedValues"], variable);
				}, ""
			);
			ImGui::PopID();
		}

		if (mat.contains("mappedValues"))
		{
			unsigned int sz = static_cast<unsigned int>(mat.at("mappedValues").size());
			for (unsigned int i = 0; i < sz; i++)
			{
				nlohmann::json& mappedV = mat.at("mappedValues").at(i);
				ImMaterialVariablesDraw.at(StrToMaterialVariablesTypes.at(std::string(mappedV.at("variableType"))))(i, mappedV);
			}
		}
	}

	/*
	nlohmann::json json()
	{
		nlohmann::json j = nlohmann::json({});

		//for (auto& [name, shader] : shaderTemplates) {
		//	if (shader->defaultValues.systemCreated) continue;
		//	j[name] = nlohmann::json({});
		//	j[name]["fileName"] = shader->defaultValues.shaderFileName;
		//	j[name]["mappedValues"] = TransformMaterialValueMappingToJson(shader->defaultValues.mappedValues);
		//}
		return j;
	}
	*/
#endif

}