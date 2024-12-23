#include "pch.h"
#include "ShaderCompiler.h"
#include "../../Common/DirectXHelper.h"
#include "../../Renderer/Renderer.h"
#include "../../Scene/Lights/Lights.h"
#include "../../Scene/Camera/Camera.h"
#include "../../Animation/Animated.h"
#include <d3d12shader.h>

namespace ShaderCompiler {
	
	//Source to Shader
	static std::map<Source, ShaderCompilerOutputPtr> shaderOutputRefs;
	//Dependencies of hlsl+shaderType(Source) file to many .h files
	typedef std::map<Source, ShaderDependencies> ShaderIncludesDependencies;
	static ShaderIncludesDependencies dependencies;
	//Notifications 
	static TemplatesNotification<ShaderCompilerOutputPtr*> shaderCompilationNotifications;

	HRESULT __stdcall CustomIncludeHandler::LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource)
	{
		ComPtr<IDxcBlobEncoding> pEncoding;
		std::filesystem::path path(pFilename);
		std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path);
		std::wstring wpath = canonicalPath.c_str();

		if (includedFiles.find(wpath) != includedFiles.end())
		{
			// Return empty string blob if this file has been included before
			static const char nullStr[] = " ";
			pUtils->CreateBlobFromPinned(nullStr, ARRAYSIZE(nullStr), DXC_CP_ACP, pEncoding.GetAddressOf());
			*ppIncludeSource = pEncoding.Detach();
			return S_OK;
		}

		const std::wstring filename = shaderRootFolder + pFilename;
		HRESULT hr = pUtils->LoadFile(filename.c_str(), nullptr, pEncoding.GetAddressOf());
		if (SUCCEEDED(hr))
		{
			includedFiles.insert(wpath);
			*ppIncludeSource = pEncoding.Detach();
		}
		return hr;
	}

	static std::mutex compileMutex;
	Concurrency::task<ShaderCompilerOutputPtr> ShaderCompiler::Compile(Source& params) {
		std::lock_guard<std::mutex> lock(compileMutex);
		//read the shader
		const std::wstring shaderRootFolder = L"Shaders/";
		const std::wstring filename = shaderRootFolder + params.shaderName + L".hlsl";
		ShaderByteCode shaderSource;
		DX::ReadDataAsync(filename.c_str()).then([&shaderSource](std::vector<byte> fileData) {
			shaderSource = fileData;
		}).wait();

		//create a blob for the shader source code
		ComPtr<IDxcBlobEncoding> pSource;
		pUtils->CreateBlob(&shaderSource[0], (UINT32)shaderSource.size(), CP_UTF8, pSource.GetAddressOf());

		//build the arguments
		std::vector<LPCWSTR> arguments;
		arguments.push_back(L"-E");// -E for the entry point (eg. 'main')
		arguments.push_back(ShaderT::shaderEntryPoint[params.shaderType].c_str());
		arguments.push_back(L"-T");// -T for the target profile (eg. 'ps_6_6')
		arguments.push_back(ShaderT::shaderTarget[params.shaderType].c_str());

#ifndef _DEBUG
		arguments.push_back(L"-Qstrip_debug"); //remove debug info(PDB)
		arguments.push_back(L"-Qstrip_reflect"); // Strip reflection data and pdbs (see later)
#else
		arguments.push_back(L"-Qembed_debug"); //add debug info(PDB)
#endif

		arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
#ifdef _DEBUG
		arguments.push_back(DXC_ARG_DEBUG); //-Zi
#endif

		arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR);

		DxcBuffer sourceBuffer{ .Ptr = pSource->GetBufferPointer(), .Size = pSource->GetBufferSize(), .Encoding = 0 };

		//compile the shader
		ComPtr<IDxcResult> pCompileResult;
		dependencies[params].clear();
		ShaderCompiler::CustomIncludeHandler includeHandler(dependencies[params], shaderRootFolder, pUtils);
		DX::ThrowIfFailed(pCompiler->Compile(&sourceBuffer, arguments.data(), (UINT32)arguments.size(), &includeHandler, IID_PPV_ARGS(pCompileResult.GetAddressOf())));

		//get the errors
		ComPtr<IDxcBlobUtf8> pErrors;
		pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(pErrors.GetAddressOf()), nullptr);
		if (pErrors && pErrors->GetStringLength() > 0) {
			OutputDebugStringA((char*)pErrors->GetBufferPointer());
			return Concurrency::task_from_result<ShaderCompilerOutputPtr>(shaderOutputRefs[params]); //return the old shader if there were errors
		}

		// Get shader reflection data.
		ComPtr<IDxcBlob> reflectionBlob{};
		DX::ThrowIfFailed(pCompileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr));
		const DxcBuffer reflectionBuffer{ .Ptr = reflectionBlob->GetBufferPointer(), .Size = reflectionBlob->GetBufferSize(), .Encoding = 0, };

		//create the reflection
		ComPtr<ID3D12ShaderReflection> shaderReflection{};
		pUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection));

		//get a description of the shader
		D3D12_SHADER_DESC shaderDesc{};
		shaderReflection->GetDesc(&shaderDesc);

		CBufferParametersDefinitionPtr cbufferParametersDef = std::make_shared<CBufferParametersDefinition>();
		TextureParametersDefinitionPtr texturesParametersDef = std::make_shared<TextureParametersDefinition>();
		SamplerParametersDefinitionPtr samplersParametersDef = std::make_shared<SamplerParametersDefinition>();

		using namespace Scene;

		INT cameraCBVRegister = -1;
		INT lightCBVRegister = -1;
		INT animationCBVRegister = -1;
		INT lightsShadowMapCBVRegister = -1;
		INT lightsShadowMapSRVRegister = -1;

		for (uint32_t paramIdx = 0; paramIdx < shaderDesc.BoundResources; paramIdx++) {
			D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
			shaderReflection->GetResourceBindingDesc(paramIdx, &bindDesc);

			std::string resourceName(bindDesc.Name);
			std::wstring resourceNameW(resourceName.begin(), resourceName.end());

			cameraCBVRegister = (resourceNameW == Camera::CameraConstantBufferName)? bindDesc.BindPoint : cameraCBVRegister;
			lightCBVRegister = (resourceNameW == Lights::LightConstantBufferName)? bindDesc.BindPoint : lightCBVRegister;
			animationCBVRegister = (resourceNameW == Animation::AnimationConstantBufferName) ? bindDesc.BindPoint : animationCBVRegister;
			lightsShadowMapCBVRegister = (resourceNameW == Lights::ShadowMapConstantBufferName) ? bindDesc.BindPoint : lightsShadowMapCBVRegister;
			lightsShadowMapSRVRegister = (resourceNameW == Lights::ShadowMapLightsShaderResourveViewName) ? bindDesc.BindPoint : lightsShadowMapSRVRegister;

			if (bindDesc.Type == D3D_SIT_CBUFFER) {
				cbufferParametersDef->insert({ resourceNameW, {
					.registerId = bindDesc.BindPoint,
					.numCBuffers = bindDesc.BindCount,
				}});
			} else if (bindDesc.Type == D3D_SIT_TEXTURE) {
				texturesParametersDef->insert({ resourceNameW, {
					.registerId = bindDesc.BindPoint,
					.numTextures = bindDesc.BindCount>0?bindDesc.BindCount:bindDesc.NumSamples, //if N > 0 -> N else -1
				}});
			} else if (bindDesc.Type == D3D_SIT_SAMPLER) {
				samplersParametersDef->insert({ resourceNameW, {
					.registerId = bindDesc.BindPoint,
					.numSamplers = bindDesc.BindCount,
				} });
			}
		}

		CBufferVariablesDefinitionPtr cbufferVariablesDef = std::make_shared<CBufferVariablesDefinition>();
		
		std::set<std::wstring> cbufferToSkip = { Camera::CameraConstantBufferName, Lights::LightConstantBufferName };

		std::vector<size_t> cbufferSize = {};
		for (uint32_t paramIdx = 0; paramIdx < shaderDesc.ConstantBuffers; paramIdx++) {
			auto cbReflection = shaderReflection->GetConstantBufferByIndex(paramIdx);
			D3D12_SHADER_BUFFER_DESC paramDesc{};
			cbReflection->GetDesc(&paramDesc);

			std::string cbufferName(paramDesc.Name);
			std::wstring cbufferNameW(cbufferName.begin(), cbufferName.end());
			bool isCameraParam = cbufferNameW == Camera::CameraConstantBufferName;
			bool isLightParam = cbufferNameW == Lights::LightConstantBufferName;
			bool isAnimationParam = cbufferNameW == Animation::AnimationConstantBufferName;
			bool isShadowMapParam = cbufferNameW == Lights::ShadowMapConstantBufferName;
			if (isCameraParam || isLightParam || isAnimationParam || isShadowMapParam) continue;

			for (UINT varIdx = 0; varIdx < paramDesc.Variables; varIdx++) {
				ID3D12ShaderReflectionVariable* varReflection = cbReflection->GetVariableByIndex(varIdx);
				D3D12_SHADER_VARIABLE_DESC varDesc;
				varReflection->GetDesc(&varDesc);

				std::string varName(varDesc.Name);
				std::wstring varNameW(varName.begin(), varName.end());

				cbufferVariablesDef->insert(std::pair<std::wstring, CBufferVariable>(varNameW, CBufferVariable({
					.bufferIndex = paramIdx,
					.size = varDesc.Size,
					.offset = varDesc.StartOffset,
				})));
			}
			cbufferSize.push_back(paramDesc.Size);
		}

		// Get compilation result
		ComPtr<IDxcBlob> code;
		pCompileResult->GetResult(&code);

		//create a new shader and copy the generated code into it's byteCode
		ShaderCompilerOutputPtr shaderOutput = std::make_shared<ShaderCompilerOutput>(ShaderCompilerOutput({
			.byteCode = std::make_shared<ShaderByteCode>(code->GetBufferSize()),
			.cbufferParametersDef = cbufferParametersDef,
			.texturesParametersDef = texturesParametersDef,
			.samplersParametersDef = samplersParametersDef,
			.cbufferVariablesDef = cbufferVariablesDef,
			.cbufferSize = cbufferSize,
			.shaderType = params.shaderType,
			.cameraCBVRegister = cameraCBVRegister,
			.lightCBVRegister = lightCBVRegister,
			.animationCBVRegister = animationCBVRegister,
			.lightsShadowMapCBVRegister = lightsShadowMapCBVRegister,
			.lightsShadowMapSRVRegister = lightsShadowMapSRVRegister
		}));
		memcpy_s(&(*shaderOutput->byteCode.get())[0], code->GetBufferSize(), code->GetBufferPointer(), code->GetBufferSize());
		return Concurrency::task_from_result<ShaderCompilerOutputPtr>(shaderOutput);
	}

	static std::mutex bindMutex;
	Concurrency::task<void> Bind(const std::wstring& shaderName, ShaderType shaderType, void* target, NotificationCallbacks callbacks)
	{
		return concurrency::create_task([shaderName, shaderType, target, callbacks] {
			std::lock_guard<std::mutex> lock(bindMutex);
			Source params = { shaderName, shaderType };

			ShaderCompilerOutputPtr output = Compile(params).get();
			shaderOutputRefs[params] = output;

			auto shaderOutRef = &shaderOutputRefs[params];

			ChangesNotifications notifications = {
				{ NotificationTarget({ .target = target }), callbacks }
			};
			shaderCompilationNotifications[shaderOutRef] = notifications;
			if (shaderCompilationNotifications.contains(shaderOutRef)) {
				NotifyOnLoadComplete(shaderOutRef, notifications);
			}
		});
	}

	void BuildShaderCompiler() {
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
	}
	
	static CompileQueue changesQueue;
	void MonitorShaderChanges(std::wstring folder) {
		
		std::thread monitor([folder]() {
			HANDLE file = CreateFileW(folder.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);

			if (file == INVALID_HANDLE_VALUE || file == NULL) {
				OutputDebugStringW(L"ERROR: Invalid HANDLE value.\n");
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
							std::wstring shaderFile = std::wstring(event->FileName, event->FileNameLength / sizeof(event->FileName[0]));
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
		});
		
		std::thread compilation([]() {
			using namespace std::chrono_literals;
			while (TRUE) {
				while (changesQueue.size() > 0) {
					std::filesystem::path filePath(changesQueue.front());
					std::wstring fileExtension = filePath.extension();

					std::wstring output = L"Modified: " + filePath.wstring() + L"\n";
					OutputDebugStringW(output.c_str());

					auto buildShader = [fileExtension](const std::wstring& shaderName) {
						Source sources[] = {
							{ .shaderName = shaderName, .shaderType = VERTEX_SHADER },
							{ .shaderName = shaderName, .shaderType = PIXEL_SHADER }
						};

						auto shaderOutputOldRef = &shaderOutputRefs.find(sources[0])->second;

						if (shaderCompilationNotifications.contains(shaderOutputOldRef)) {
							NotifyOnLoadStart<ShaderCompilerOutputPtr*>(shaderCompilationNotifications[shaderOutputOldRef]);
						}

						for (auto& source : sources) {
							auto output = Compile(source).get();
							auto& shaderOutputRef = shaderOutputRefs[source];
							if (shaderOutputRef.get() != output.get()) {
								shaderOutputRef->CopyFrom(output);
							}

							if (shaderCompilationNotifications.contains(&shaderOutputRef)) {
								auto& notifications = shaderCompilationNotifications[&shaderOutputRef];
								NotifyOnLoadComplete(&shaderOutputRef, notifications);
							}
						}
					};

					auto buildShaderFromDependency = [buildShader](const std::wstring& shaderDependency) {
						std::set<std::wstring> shaderNames;
						for (auto& [src, deps] : dependencies) {
							if (deps.contains(shaderDependency)) {
								shaderNames.insert(src.shaderName);
							}
						}
						for (auto shaderName : shaderNames) {
							buildShader(shaderName);
						}
					};

					if (!_wcsicmp(fileExtension.c_str(), L".hlsl")) {
						buildShader(filePath.stem());
					} else if (!_wcsicmp(fileExtension.c_str(), L".h")) {
						buildShaderFromDependency(filePath);
					}

					changesQueue.pop();
				}
				std::this_thread::sleep_for(200ms);
			}
		});
			
		monitor.detach();
		compilation.detach();
	}
}