#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils::D3D12Device
{

	UINT64 Signal(CComPtr<ID3D12CommandQueue> commandQueue, CComPtr<ID3D12Fence> fence, UINT64& fenceValue);
	void WaitForFenceValue(CComPtr<ID3D12Fence> fence, UINT64 fenceValue, HANDLE fenceEvent);
	void Flush(CComPtr<ID3D12CommandQueue> commandQueue, CComPtr<ID3D12Fence> fence, UINT64& fenceValue, HANDLE fenceEvent);
	
};

