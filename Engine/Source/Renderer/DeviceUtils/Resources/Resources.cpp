#include "pch.h"
#include "Resources.h"
#include "../../../Common/DirectXHelper.h"
#include <d3d12.h>
#include <atlbase.h>

namespace DeviceUtils {

	static std::mutex loadBufferResourceMutex;
	void UpdateBufferResource(CComPtr<ID3D12Device2>& device, CComPtr<ID3D12GraphicsCommandList2>& commandList,
		CComPtr<ID3D12Resource>& pDestinationResource, CComPtr<ID3D12Resource>& pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData)
	{
		std::lock_guard<std::mutex> lock(loadBufferResourceMutex);

		size_t bufferSize = numElements * elementSize;

		// Create a committed resource for the GPU resource in a default heap.
		const CD3DX12_HEAP_PROPERTIES defaultHeapTypeProperties(D3D12_HEAP_TYPE_DEFAULT);
		const CD3DX12_RESOURCE_DESC defaultHeapBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE);
		DX::ThrowIfFailed(device->CreateCommittedResource(
			&defaultHeapTypeProperties,
			D3D12_HEAP_FLAG_NONE,
			&defaultHeapBufferDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&pDestinationResource)));

		//NAME_D3D12_OBJECT(pDestinationResource);

		// Create a committed resource for the upload.
		if (bufferData)
		{
			const CD3DX12_HEAP_PROPERTIES uploadHeapTypeProperties(D3D12_HEAP_TYPE_UPLOAD);
			const CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
			DX::ThrowIfFailed(device->CreateCommittedResource(
				&uploadHeapTypeProperties,
				D3D12_HEAP_FLAG_NONE,
				&uploadBufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&pIntermediateResource)));

			//NAME_D3D12_OBJECT(pIntermediateResource);

			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = bufferData;
			subresourceData.RowPitch = bufferSize;
			subresourceData.SlicePitch = subresourceData.RowPitch;

			UpdateSubresources(commandList,
				pDestinationResource, pIntermediateResource,
				0, 0, 1, &subresourceData);
		}
	}
}