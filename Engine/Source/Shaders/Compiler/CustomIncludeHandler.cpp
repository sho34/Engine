#include "pch.h"
#include "CustomIncludeHandler.h"

HRESULT __stdcall CustomIncludeHandler::LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource)
{
	ComPtr<IDxcBlobEncoding> pEncoding;
	std::filesystem::path path(pFilename);
	std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path);
	std::string wpath = canonicalPath.string();

	if (includedFiles.find(wpath) != includedFiles.end())
	{
		// Return empty string blob if this file has been included before
		static const char nullStr[] = " ";
		pUtils->CreateBlobFromPinned(nullStr, ARRAYSIZE(nullStr), DXC_CP_ACP, pEncoding.GetAddressOf());
		*ppIncludeSource = pEncoding.Detach();
		return S_OK;
	}

	const std::wstring filename = nostd::StringToWString(shaderRootFolder) + pFilename;
	HRESULT hr = pUtils->LoadFile(filename.c_str(), nullptr, pEncoding.GetAddressOf());
	if (SUCCEEDED(hr))
	{
		includedFiles.insert(wpath);
		*ppIncludeSource = pEncoding.Detach();
	}
	return hr;
}
