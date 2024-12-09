#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils::D3D12Device {
	CComPtr<IDXGIAdapter4> GetAdapter();
	CComPtr<ID3D12Device2> CreateDevice(CComPtr<IDXGIAdapter4> adapter);
	CComPtr<ID3D12CommandQueue> CreateCommandQueue(CComPtr<ID3D12Device2> device);
	CComPtr<IDXGISwapChain4> CreateSwapChain(HWND hwnd, CComPtr<ID3D12CommandQueue> commandQueue, UINT bufferCount);
	CComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(CComPtr<ID3D12Device2> device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	CComPtr<ID3D12CommandAllocator> CreateCommandAllocator(CComPtr<ID3D12Device2> device);
	CComPtr<ID3D12GraphicsCommandList2> CreateCommandList(CComPtr<ID3D12Device2> device, CComPtr<ID3D12CommandAllocator> commandAllocator);
	CComPtr<ID3D12Fence> CreateFence(CComPtr<ID3D12Device2> device);
	HANDLE CreateEventHandle();

	//D3D11On12
	void CreateD3D11On12Device(CComPtr<ID3D12Device2> d3dDevice, CComPtr<ID3D12CommandQueue> commandQueue, CComPtr<ID3D11Device> d3d11Device, CComPtr<ID3D11On12Device>& d3d11on12Device, CComPtr<ID3D11DeviceContext>& d3d11DeviceContext, CComPtr<IDXGIDevice>& dxgiDevice);
	void CreateD2D1Device(CComPtr<IDXGIDevice> dxgiDevice, CComPtr<ID2D1Factory6>& d2d1Factory, CComPtr<ID2D1Device5>& d2d1Device, CComPtr<ID2D1DeviceContext5>& d2d1DeviceContext);
}

