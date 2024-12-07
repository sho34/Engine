#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils::RenderTarget
{

	void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap, ComPtr<ID3D12Resource> renderTargets[], UINT bufferCount);
	void UpdateDepthStencilView(ComPtr<ID3D12Device2> device, ComPtr<ID3D12DescriptorHeap> descriptorHeap, ComPtr<ID3D12Resource>& depthStencil, UINT width, UINT height);

	//D3D11On12
	void UpdateD2D1RenderTargets(ComPtr<ID3D11On12Device>	d3d11on12Device, ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<ID3D12Resource> renderTargets[], ComPtr<ID3D11Resource> wrappedBackBuffers[], ComPtr<ID2D1Bitmap1> d2dRenderTargets[], UINT bufferCount);

};

