#pragma once

#include "../../framework.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace Concurrency;

struct Renderer;

namespace ShaderCompiler {
	struct CustomIncludeHandler : IDxcIncludeHandler {

		std::wstring shaderName;
		std::wstring ShaderRootFolder;
		ComPtr<IDxcUtils> pUtils;
		std::unordered_set<std::wstring> IncludedFiles;

		CustomIncludeHandler(std::wstring shaderName, std::wstring ShaderRootFolder, ComPtr<IDxcUtils> pUtils) {
			this->shaderName = shaderName;
			this->ShaderRootFolder = ShaderRootFolder;
			this->pUtils = pUtils;
		};

		HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override;
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override { return E_NOINTERFACE; }
		ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }
		ULONG STDMETHODCALLTYPE Release(void) override { return 0; }
	};

	typedef std::vector<byte> ShaderByteCode;
	typedef std::shared_ptr<ShaderByteCode> ShaderByteCodeRef;

	struct ShaderParameters {
		const std::wstring entryPoint;
		const std::wstring target;
		bool operator<(const ShaderParameters& other) const {
			return std::tie(entryPoint, target) < std::tie(other.entryPoint, other.target);
		}
	};

	typedef Concurrency::task<void> LoadShaderFn(void*, ComPtr<ID3D12Device2>);
	struct CompilerTarget {
		void* renderable;
		LoadShaderFn* loadShaderFn;
	};
	typedef std::vector<CompilerTarget> ShaderTargets;

	typedef std::map<ShaderParameters, ShaderTargets> ShaderParameterTargets;
	typedef std::map<const std::wstring, ShaderParameterTargets> ShadersInstances;
	static ShadersInstances instances;

	typedef std::map<ShaderParameters, std::shared_ptr<ShaderByteCode>> ShaderParameterByteCode;
	typedef std::map<const std::wstring, ShaderParameterByteCode> ShadersByteCodes;
	static ShadersByteCodes shaderByteCodes;

	typedef std::unordered_set<std::wstring> ShaderDependencies;
	typedef std::map<const std::wstring, ShaderDependencies> ShaderIncludesDependencies;
	static ShaderIncludesDependencies dependencies;

	ShaderByteCode& GetByteCode(const std::wstring& shaderName, const std::wstring& entryPoint, const std::wstring& target);
	Concurrency::task<ShaderByteCode> Compile(const std::wstring& shaderName, const std::wstring& entryPoint, const std::wstring& target);
	Concurrency::task<void> Bind(ComPtr<ID3D12Device2> d3dDevice, void* renderable, ShaderByteCodeRef& byteCodeRef, LoadShaderFn* loadShaderFn, const std::wstring& shaderName, const std::wstring& entryPoint, const std::wstring& target);

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
	
	void MonitorChanges(std::wstring folder, std::shared_ptr<Renderer> renderer);
};