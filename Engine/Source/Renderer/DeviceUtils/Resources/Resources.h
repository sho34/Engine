#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils::Resources
{
	void UpdateBufferResource(ComPtr<ID3D12Device2>& device, ComPtr<ID3D12GraphicsCommandList2>& commandList, ComPtr<ID3D12Resource>& pDestinationResource, ComPtr<ID3D12Resource>& pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData);
	void CreateTextureResource(ComPtr<ID3D12Device2>& d3dDevice, ComPtr<ID3D12GraphicsCommandList2>& commandList, const LPWSTR path, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& textureUpload, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	void CreateTextureArrayResource(ComPtr<ID3D12Device2>& d3dDevice, ComPtr<ID3D12GraphicsCommandList2>& commandList, const LPWSTR path, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& textureUpload, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, UINT numFrames, DXGI_FORMAT textureFormat = DXGI_FORMAT_BC7_UNORM_SRGB);
};

