#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils::Resources
{
	void UpdateBufferResource(CComPtr<ID3D12Device2>& device, CComPtr<ID3D12GraphicsCommandList2>& commandList, CComPtr<ID3D12Resource>& pDestinationResource, CComPtr<ID3D12Resource>& pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData);
	void CreateTextureResource(CComPtr<ID3D12Device2>& d3dDevice, CComPtr<ID3D12GraphicsCommandList2>& commandList, const LPWSTR path, CComPtr<ID3D12Resource>& texture, CComPtr<ID3D12Resource>& textureUpload, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	void CreateTextureArrayResource(CComPtr<ID3D12Device2>& d3dDevice, CComPtr<ID3D12GraphicsCommandList2>& commandList, const LPWSTR path, CComPtr<ID3D12Resource>& texture, CComPtr<ID3D12Resource>& textureUpload, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, UINT numFrames, DXGI_FORMAT textureFormat = DXGI_FORMAT_BC7_UNORM_SRGB);
	void DestroyTextureResources();
};

