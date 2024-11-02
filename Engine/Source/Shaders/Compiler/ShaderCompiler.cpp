#include "ShaderCompiler.h"
#include "../../Common/DirectXHelper.h"
#include "../../Renderer/Renderer.h"

HRESULT __stdcall ShaderCompiler::CustomIncludeHandler::LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource)
{
	ComPtr<IDxcBlobEncoding> pEncoding;
	std::filesystem::path path(pFilename);
	std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path);
	std::wstring wpath = canonicalPath.c_str();

	if (IncludedFiles.find(wpath) != IncludedFiles.end())
	{
		// Return empty string blob if this file has been included before
		static const char nullStr[] = " ";
		pUtils->CreateBlobFromPinned(nullStr, ARRAYSIZE(nullStr), DXC_CP_ACP, pEncoding.GetAddressOf());
		*ppIncludeSource = pEncoding.Detach();
		return S_OK;
	}

	const std::wstring filename = ShaderRootFolder + pFilename;
	HRESULT hr = pUtils->LoadFile(filename.c_str(), nullptr, pEncoding.GetAddressOf());
	if (SUCCEEDED(hr))
	{
		IncludedFiles.insert(wpath);
		*ppIncludeSource = pEncoding.Detach();
		auto fileDependencies = dependencies.find(path);
		if (fileDependencies == dependencies.end()) {
			dependencies.insert({ wpath, ShaderDependencies() });
			fileDependencies = dependencies.find(wpath);
		}
		fileDependencies->second.insert(this->shaderName);
	}
	return hr;
}

ShaderCompiler::ShaderByteCode& ShaderCompiler::GetByteCode(const std::wstring& shaderName, const std::wstring& entryPoint, const std::wstring& target)
{
	ShaderParameters params = { entryPoint, target };
	auto byteCodeInstances = shaderByteCodes.find(shaderName);
	auto byteCodeTarget = byteCodeInstances->second.find(params);
	return *byteCodeTarget->second.get();
}

Concurrency::task<ShaderCompiler::ShaderByteCode> ShaderCompiler::Compile(const std::wstring& shaderName, const std::wstring& entryPoint, const std::wstring& target)
{
	const std::wstring ShaderRootFolder = L"Shaders/";
	const std::wstring filename = ShaderRootFolder + shaderName + L".hlsl";
	std::vector<byte> shaderSource;
	auto createShaderTask = DX::ReadDataAsync(filename.c_str()).then([&shaderSource](std::vector<byte> fileData) {
		shaderSource = fileData;
	}).wait();

	ComPtr<IDxcUtils> pUtils;
	ComPtr<IDxcCompiler3> pCompiler;
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

	ComPtr<IDxcBlobEncoding> pSource;
	pUtils->CreateBlob(&shaderSource[0], (UINT32)shaderSource.size(), CP_UTF8, pSource.GetAddressOf());

	std::vector<LPCWSTR> arguments;
	// -E for the entry point (eg. 'main')
	arguments.push_back(L"-E");
	arguments.push_back(entryPoint.c_str());

	// -T for the target profile (eg. 'ps_6_6')
	arguments.push_back(L"-T");
	arguments.push_back(target.c_str());

	// Strip reflection data and pdbs (see later)
	arguments.push_back(L"-Qstrip_debug");
	arguments.push_back(L"-Qstrip_reflect");

	arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
	arguments.push_back(DXC_ARG_DEBUG); //-Zi

	DxcBuffer sourceBuffer;
	sourceBuffer.Ptr = pSource->GetBufferPointer();
	sourceBuffer.Size = pSource->GetBufferSize();
	sourceBuffer.Encoding = 0;

	ComPtr<IDxcResult> pCompileResult;
	ShaderCompiler::CustomIncludeHandler includeHandler(shaderName, ShaderRootFolder, pUtils);
	DX::ThrowIfFailed(pCompiler->Compile(&sourceBuffer, arguments.data(), (UINT32)arguments.size(), &includeHandler, IID_PPV_ARGS(pCompileResult.GetAddressOf())));

	ComPtr<IDxcBlobUtf8> pErrors;
	pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(pErrors.GetAddressOf()), nullptr);
	if (pErrors && pErrors->GetStringLength() > 0) {
		OutputDebugStringA((char*)pErrors->GetBufferPointer());
		return Concurrency::task_from_result<ShaderByteCode>(GetByteCode(shaderName, entryPoint, target));
	} else {
		// Get compilation result
		ComPtr<IDxcBlob> code;
		pCompileResult->GetResult(&code);
		std::vector<byte> shaderByteCode(code->GetBufferSize());
		memcpy_s(&shaderByteCode[0], code->GetBufferSize(), code->GetBufferPointer(), code->GetBufferSize());
		return Concurrency::task_from_result<ShaderByteCode>(shaderByteCode);
	}
}

std::mutex bindMutex;
Concurrency::task<void> ShaderCompiler::Bind(ComPtr<ID3D12Device2> d3dDevice, void* renderable, ShaderByteCodeRef& byteCodeRef, LoadShaderFn* loadShaderFn, const std::wstring& shaderName, const std::wstring& entryPoint, const std::wstring& target)
{
	return concurrency::create_task([d3dDevice, renderable, &byteCodeRef, loadShaderFn, shaderName, entryPoint, target] {
		std::lock_guard<std::mutex> lock(bindMutex);
		ShaderParameters params = { entryPoint, target };

		//map the shader with it's parameters to this renderable object
		auto shaderInstances = instances.find(shaderName);
		if (shaderInstances == instances.end()) {
			instances.insert({ shaderName, ShaderParameterTargets() });
			shaderInstances = instances.find(shaderName);
		}

		auto shaderTarget = shaderInstances->second.find(params);
		if (shaderTarget == shaderInstances->second.end()) {
			shaderInstances->second.insert({ params, ShaderTargets() });
			shaderTarget = shaderInstances->second.find(params);
		}

		shaderTarget->second.push_back({ renderable, loadShaderFn });

		//compile the code
		ShaderByteCodeRef compiledByteCode(new ShaderByteCode(Compile(shaderName, entryPoint, target).get()));

		//push the byte code to the shader tree references
		auto byteCodeInstances = shaderByteCodes.find(shaderName);
		if (byteCodeInstances == shaderByteCodes.end()) {
			shaderByteCodes.insert({ shaderName, ShaderParameterByteCode() });
			byteCodeInstances = shaderByteCodes.find(shaderName);
		}

		auto byteCodeTarget = byteCodeInstances->second.find(params);
		if (byteCodeTarget == byteCodeInstances->second.end()) {
			byteCodeInstances->second.insert({ params, compiledByteCode });
		}

		byteCodeRef = compiledByteCode;
		loadShaderFn(renderable, d3dDevice).wait();
	});
}

ShaderCompiler::CompileQueue changesQueue;
void ShaderCompiler::MonitorChanges(std::wstring folder, std::shared_ptr<Renderer> renderer) {

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

	std::thread compilation([renderer]() {
		using namespace std::chrono_literals;
		while (TRUE) {
			while (changesQueue.size() > 0) {
				std::filesystem::path filePath(changesQueue.front());
				std::wstring fileExtension = filePath.extension();

				std::wstring output = L"Modified: " + filePath.wstring() + L"\n";
				OutputDebugStringW(output.c_str());

				auto buildShader = [fileExtension,renderer](const std::wstring& shaderName) {
					auto shaderInstances = instances.find(shaderName);
					if (shaderInstances == instances.end()) return;

					auto byteCodes = shaderByteCodes.find(shaderName);

					for (auto const& [parameters, targets] : shaderInstances->second)
					{
						auto newCode = Compile(shaderName, parameters.entryPoint, parameters.target).get();
						byteCodes->second.at(parameters)->assign(newCode.begin(), newCode.end());

						for (auto const& target : targets) {
							target.loadShaderFn(target.renderable, renderer->d3dDevice).wait();
						}
					}
				};

				auto buildShaderFromDependency = [buildShader](const std::wstring& shaderDependency) {
					auto shaderDeps = dependencies.find(shaderDependency);
					if (shaderDeps == dependencies.end()) return;

					for (auto const& shaderName : shaderDeps->second) {
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