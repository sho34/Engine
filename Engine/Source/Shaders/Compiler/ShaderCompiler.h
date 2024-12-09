#pragma once

#include <wrl.h>
#include <map>
#include <unordered_set>
#include <string>
#include <ppltasks.h>
#include <dxcapi.h>
#include <d3d12.h>
#include "ShaderCompilerOutput.h"
#include "../../Templates/Shader.h"
#include "../../Types.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace Concurrency;

struct Renderer;

namespace ShaderCompiler {
	
	//shader compilation source (shaderName + it's shader type)
	struct Source {
		const std::wstring shaderName;
		ShaderType shaderType;
		bool operator<(const Source& other) const {
			return std::tie(shaderName, shaderType) < std::tie(other.shaderName, other.shaderType);
		}
	};
	

	//Dependencies of hlsl+shaderType(Source) file to many .h files
	typedef std::unordered_set<std::wstring> ShaderDependencies;

	//a custom handler to manage include files
	struct CustomIncludeHandler : IDxcIncludeHandler {

		std::wstring shaderRootFolder;
		ComPtr<IDxcUtils> pUtils;

		ShaderDependencies& includedFiles;

		CustomIncludeHandler(ShaderDependencies& includedFiles, std::wstring shaderRootFolder, ComPtr<IDxcUtils> pUtils):includedFiles(includedFiles) {
			this->shaderRootFolder = shaderRootFolder;
			this->pUtils = pUtils;
		};

		HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override;
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override { return E_NOINTERFACE; }
		ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }
		ULONG STDMETHODCALLTYPE Release(void) override { return 0; }
	};

	Concurrency::task<ShaderCompilerOutputPtr> Compile(Source& params);
	Concurrency::task<void> Bind(const std::wstring& shaderName, ShaderType shaderType, void* target, NotificationCallbacks callbacks);
	
	//compile queue definition
	struct CompileQueue
	{
		CompileQueue(){
			queue = std::queue<std::wstring>();
		}

		void push(const std::wstring& value) {
			std::lock_guard<std::mutex> lock(mutex);
			if (queue.size()==0 || queue.front() != value) {
				queue.push(value);
			}
		}

		void pop() {
			std::lock_guard<std::mutex> lock(mutex);
			queue.pop();
		}

		std::wstring front() {
			std::lock_guard<std::mutex> lock(mutex);
			return queue.front();
		}

		size_t size() {
			std::lock_guard<std::mutex> lock(mutex);
			return queue.size();
		}

		std::queue<std::wstring> queue;
		mutable std::mutex mutex;
	};
	
	//compiler and utils
	static ComPtr<IDxcCompiler3> pCompiler;
	static ComPtr<IDxcUtils> pUtils;
	void BuildShaderCompiler();
	void MonitorShaderChanges(std::wstring folder);
};