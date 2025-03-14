#pragma once

//a custom handler to manage include files
struct CustomIncludeHandler : IDxcIncludeHandler {

	std::string shaderRootFolder;
	ComPtr<IDxcUtils> pUtils;

	ShaderDependencies& includedFiles;

	CustomIncludeHandler(ShaderDependencies& includedFiles, std::string shaderRootFolder, ComPtr<IDxcUtils> pUtils) :includedFiles(includedFiles) {
		this->shaderRootFolder = shaderRootFolder;
		this->pUtils = pUtils;
	};

	HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override;
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override { return E_NOINTERFACE; }
	ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }
	ULONG STDMETHODCALLTYPE Release(void) override { return 0; }
};
