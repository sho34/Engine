#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils::D3D12Device
{

	UINT64 Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, UINT64& fenceValue);
	void WaitForFenceValue(ComPtr<ID3D12Fence> fence, UINT64 fenceValue, HANDLE fenceEvent);
	void Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, UINT64& fenceValue, HANDLE fenceEvent);
	
};

