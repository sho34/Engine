#pragma once

#include <d3d12.h>

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils
{
	void UpdateBufferResource(CComPtr<ID3D12Device2>& device, CComPtr<ID3D12GraphicsCommandList2>& commandList, CComPtr<ID3D12Resource>& pDestinationResource, CComPtr<ID3D12Resource>& pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData);
	HRESULT CaptureTexture(CComPtr<ID3D12Device2> device,
		CComPtr<ID3D12CommandQueue> pCommandQ,
		CComPtr<ID3D12Resource> pSource,
		UINT64 srcPitch,
		const D3D12_RESOURCE_DESC& desc,
		CComPtr<ID3D12Resource>& pStaging,
		D3D12_RESOURCE_STATES beforeState,
		D3D12_RESOURCE_STATES afterState) noexcept;
};

