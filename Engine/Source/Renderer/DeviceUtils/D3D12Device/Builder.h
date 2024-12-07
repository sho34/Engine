#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils::D3D12Device {
	ComPtr<IDXGIAdapter4> GetAdapter();
	ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter);
	ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device2> device);
	ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hwnd, ComPtr<ID3D12CommandQueue> commandQueue, UINT bufferCount);
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device2> device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<ID3D12Device2> device);
	ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandAllocator);
	ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> device);
	HANDLE CreateEventHandle();

	//D3D11On12
	void CreateD3D11On12Device(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D11Device>& d3d11Device, ComPtr<ID3D11On12Device>& d3d11on12Device, ComPtr<ID3D11DeviceContext>& d3d11DeviceContext, ComPtr<IDXGIDevice>& dxgiDevice);
	void CreateD2D1Device(ComPtr<IDXGIDevice> dxgiDevice, ComPtr<ID2D1Factory6>& d2d1Factory, ComPtr<ID2D1Device5>& d2d1Device, ComPtr<ID2D1DeviceContext5>& d2d1DeviceContext);
}

