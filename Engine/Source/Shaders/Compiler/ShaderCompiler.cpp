#include "pch.h"
#include "ShaderCompiler.h"
#include "../../Resources/ShaderByteCode.h"
#include "../../Common/DirectXHelper.h"
#include "../../pch/NoStd.h"
#include <string>
#include <set>
#include <map>

namespace ShaderCompiler {

	std::map<ShaderType, std::wstring> shaderEntryPoint =
	{
		{	ShaderType::VERTEX_SHADER, L"main_vs"	},
		{	ShaderType::PIXEL_SHADER, L"main_ps"	},
	};

	std::map<ShaderType, std::wstring> shaderTarget =
	{
		{	ShaderType::VERTEX_SHADER, L"vs_6_1"	},
		{	ShaderType::PIXEL_SHADER, L"ps_6_1" },
	};

	nostd::RefTracker<Source, std::shared_ptr<ShaderBinary>> refTracker;

	//Dependencies of hlsl+shaderType(Source) file to many .h files
	typedef std::map<Source, ShaderDependencies> ShaderIncludesDependencies;
	ShaderIncludesDependencies dependencies;

	std::shared_ptr<ShaderBinary> ShaderCompiler::GetShaderBinary(Source params)
	{
		return refTracker.AddRef(params, [&params]()
			{
				return Compile(params);
			}
		);
	}

	static std::mutex compileMutex;
	std::shared_ptr<ShaderBinary> ShaderCompiler::Compile(Source params) {

		std::lock_guard<std::mutex> lock(compileMutex);
		//read the shader

		const std::string filename = shaderRootFolder + params.shaderName + ".hlsl";
		ShaderByteCode shaderSource = DX::ReadDataAsync(filename.c_str()).get();

		//create a blob for the shader source code
		ComPtr<IDxcBlobEncoding> pSource;
		pUtils->CreateBlob(&shaderSource[0], (UINT32)shaderSource.size(), CP_UTF8, pSource.GetAddressOf());

		//build the arguments
		std::vector<LPCWSTR> arguments;
		arguments.push_back(L"-E");// -E for the entry point (eg. 'main')
		arguments.push_back(shaderEntryPoint[params.shaderType].c_str());
		arguments.push_back(L"-T");// -T for the target profile (eg. 'ps_6_6')
		arguments.push_back(shaderTarget[params.shaderType].c_str());

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

		std::vector<std::wstring> wdefines;
		std::transform(params.defines.begin(), params.defines.end(), std::back_inserter(wdefines), [](std::string def) { return nostd::StringToWString(def); });

		for (const std::wstring& def : wdefines) {
			arguments.push_back(L"-D");
			arguments.push_back(def.c_str());
		}

		DxcBuffer sourceBuffer{ .Ptr = pSource->GetBufferPointer(), .Size = pSource->GetBufferSize(), .Encoding = 0 };

		//compile the shader
		ComPtr<IDxcResult> pCompileResult;
		dependencies[params].clear();
		CustomIncludeHandler includeHandler(dependencies[params], shaderRootFolder, pUtils);
		DX::ThrowIfFailed(pCompiler->Compile(&sourceBuffer, arguments.data(), (UINT32)arguments.size(), &includeHandler, IID_PPV_ARGS(pCompileResult.GetAddressOf())));

		//get the errors
		ComPtr<IDxcBlobUtf8> pErrors;
		pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(pErrors.GetAddressOf()), nullptr);
		if (pErrors && pErrors->GetStringLength() > 0) {
			OutputDebugStringA((char*)pErrors->GetBufferPointer());
			return refTracker.FindValue(params);
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

		std::shared_ptr<ShaderBinary> shaderBinary = std::make_shared<ShaderBinary>();

		shaderBinary->shaderSource = params;
		shaderBinary->CreateVSSemantics(shaderReflection, shaderDesc);
		shaderBinary->CreateResourcesBinding(shaderReflection, shaderDesc);
		shaderBinary->CreateConstantsBuffersVariables(shaderReflection, shaderDesc);
		shaderBinary->CreateByteCode(pCompileResult);

		return shaderBinary;
	}

	void DestroyShaderBinary(std::shared_ptr<ShaderBinary>& shaderBinary)
	{
		Source key = refTracker.FindKey(shaderBinary);
		refTracker.RemoveRef(key, shaderBinary);
	}

	void BuildShaderCompiler() {
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
	}

	void BuildShader(std::string shaderFile)
	{
		for (auto it = refTracker.instances.begin(); it != refTracker.instances.end(); it++)
		{
			Source source = it->first;
			std::shared_ptr<ShaderBinary> instance = it->second;

			if (source.shaderName != shaderFile) continue;

			using namespace ShaderCompiler;
			std::shared_ptr<ShaderBinary> test = Compile(source);
			if (test == instance)
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

	void DestroyShaderCompiler()
	{
		pCompiler = nullptr;
		pUtils = nullptr;
	}
}