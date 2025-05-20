#include "pch.h"
#include "Resources.h"
#include "../../../Common/DirectXHelper.h"
#include <d3d12.h>
#include <atlbase.h>
#include "../../../d3dx12.h"

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
				IID_PPV_ARGS(&pIntermediateResource)
			));

			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = bufferData;
			subresourceData.RowPitch = bufferSize;
			subresourceData.SlicePitch = subresourceData.RowPitch;

			UpdateSubresources(commandList,
				pDestinationResource, pIntermediateResource,
				0, 0, 1, &subresourceData);
		}
	}

	void TransitionResource(CComPtr<ID3D12GraphicsCommandList2> commandList, CComPtr<ID3D12Resource> pSource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(pSource, beforeState, afterState);
		commandList->ResourceBarrier(1, &barrier);
	}

	void UAVResource(CComPtr<ID3D12GraphicsCommandList2> commandList, CComPtr<ID3D12Resource> pSource)
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(pSource);
		commandList->ResourceBarrier(1, &barrier);
	}

	HRESULT CaptureTexture(CComPtr<ID3D12Device2> device,
		CComPtr<ID3D12CommandQueue> pCommandQ,
		CComPtr<ID3D12Resource> pSource,
		UINT64 srcPitch,
		const D3D12_RESOURCE_DESC& desc,
		CComPtr<ID3D12Resource>& pStaging,
		D3D12_RESOURCE_STATES beforeState,
		D3D12_RESOURCE_STATES afterState) noexcept
	{
		if (pStaging)
		{
			pStaging = nullptr;
		}

		//if (!pCommandQ || !pSource || !pStaging)
			//return E_INVALIDARG;

		if (desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{
			OutputDebugStringA("ERROR: ScreenGrab does not support 1D or volume textures. Consider using DirectXTex instead.\n");
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}

		if (desc.DepthOrArraySize > 1 || desc.MipLevels > 1)
		{
			OutputDebugStringA("WARNING: ScreenGrab does not support 2D arrays, cubemaps, or mipmaps; only the first surface is written. Consider using DirectXTex instead.\n");
		}

		if (srcPitch > UINT32_MAX)
			return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

		const UINT numberOfPlanes = D3D12GetFormatPlaneCount(device, desc.Format);
		if (numberOfPlanes != 1)
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

		D3D12_HEAP_PROPERTIES sourceHeapProperties;
		HRESULT hr = pSource->GetHeapProperties(&sourceHeapProperties, nullptr);
		if (SUCCEEDED(hr) && sourceHeapProperties.Type == D3D12_HEAP_TYPE_READBACK)
		{
			// Handle case where the source is already a staging texture we can use directly
			pStaging = pSource;
			//pSource->AddRef();
			return S_OK;
		}

		// Create a command allocator
		CComPtr<ID3D12CommandAllocator> commandAlloc;
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAlloc));
		if (FAILED(hr))
			return hr;

		commandAlloc->SetName(L"ScreenGrab");

		// Spin up a new command list
		CComPtr<ID3D12GraphicsCommandList2> commandList;
		hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAlloc, nullptr, IID_PPV_ARGS(&commandList));
		if (FAILED(hr))
			return hr;

		commandList->SetName(L"ScreenGrab");

		// Create a fence
		CComPtr<ID3D12Fence> fence;
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		if (FAILED(hr))
			return hr;

		fence->SetName(L"ScreenGrab");
		//
		//unsigned int bitMask = srcPitch & 0xFF;
		//bool ass = (bitMask) == 0;
		//
		//assert((srcPitch & 0xFF) == 0);

		const CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		const CD3DX12_HEAP_PROPERTIES readBackHeapProperties(D3D12_HEAP_TYPE_READBACK);

		// Readback resources must be buffers
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.Height = 1;
		bufferDesc.Width = srcPitch * desc.Height;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.MipLevels = 1;
		bufferDesc.SampleDesc.Count = 1;

		CComPtr<ID3D12Resource> copySource(pSource);
		D3D12_RESOURCE_STATES beforeStateSource = beforeState;
		if (desc.SampleDesc.Count > 1)
		{
			TransitionResource(commandList, pSource, beforeState, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);

			// MSAA content must be resolved before being copied to a staging texture
			auto descCopy = desc;
			descCopy.SampleDesc.Count = 1;
			descCopy.SampleDesc.Quality = 0;
			descCopy.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

			CComPtr<ID3D12Resource> pTemp;
			hr = device->CreateCommittedResource(
				&defaultHeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&descCopy,
				D3D12_RESOURCE_STATE_RESOLVE_DEST,
				nullptr,
				IID_PPV_ARGS(&pTemp));
			if (FAILED(hr))
				return hr;

			assert(pTemp);

			pTemp->SetName(L"ScreenGrab temporary");

			//const DXGI_FORMAT fmt = EnsureNotTypeless(desc.Format);
			const DXGI_FORMAT fmt = desc.Format;

			D3D12_FEATURE_DATA_FORMAT_SUPPORT formatInfo = { fmt, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
			hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatInfo, sizeof(formatInfo));
			if (FAILED(hr))
				return hr;

			if (!(formatInfo.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D))
				return E_FAIL;

			for (UINT item = 0; item < desc.DepthOrArraySize; ++item)
			{
				for (UINT level = 0; level < desc.MipLevels; ++level)
				{
					const UINT index = D3D12CalcSubresource(level, item, 0, desc.MipLevels, desc.DepthOrArraySize);
					commandList->ResolveSubresource(pTemp, index, pSource, index, fmt);
				}
			}

			copySource = pTemp;
			beforeState = D3D12_RESOURCE_STATE_RESOLVE_DEST;
		}
		else
		{
			beforeStateSource = D3D12_RESOURCE_STATE_COPY_SOURCE;
		}

		// Create a staging texture
		hr = device->CreateCommittedResource(
			&readBackHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&pStaging));
		if (FAILED(hr))
		{
			if (pStaging)
			{
				pStaging = nullptr;
			}
			return hr;
		}

		pStaging->SetName(L"ScreenGrab staging");

		assert(pStaging);

		// Transition the resource if necessary
		TransitionResource(commandList, copySource, beforeState, D3D12_RESOURCE_STATE_COPY_SOURCE);

		// Get the copy target location
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint = {};
		bufferFootprint.Footprint.Width = static_cast<UINT>(desc.Width);
		bufferFootprint.Footprint.Height = desc.Height;
		bufferFootprint.Footprint.Depth = 1;
		bufferFootprint.Footprint.RowPitch = static_cast<UINT>(srcPitch);
		bufferFootprint.Footprint.Format = desc.Format;

		const CD3DX12_TEXTURE_COPY_LOCATION copyDest(pStaging, bufferFootprint);
		const CD3DX12_TEXTURE_COPY_LOCATION copySrc(copySource, 0);

		// Copy the texture
		commandList->CopyTextureRegion(&copyDest, 0, 0, 0, &copySrc, nullptr);

		// Transition the source resource to the next state
		TransitionResource(commandList, pSource, beforeStateSource, afterState);

		hr = commandList->Close();
		if (FAILED(hr))
		{
			pStaging = nullptr;
			return hr;
		}

		// Execute the command list
		//pCommandQ->ExecuteCommandLists(1, CommandListCast(&commandList));
		ID3D12CommandList* const commandLists[] = { commandList };
		pCommandQ->ExecuteCommandLists(_countof(commandLists), commandLists);

		// Signal the fence
		hr = pCommandQ->Signal(fence, 1);
		if (FAILED(hr))
		{
			pStaging = nullptr;
			return hr;
		}

		// Block until the copy is complete
		while (fence->GetCompletedValue() < 1)
			SwitchToThread();

		return S_OK;
	}

}