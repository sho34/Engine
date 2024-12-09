#pragma once

#include "../../Renderer.h"

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils::RenderTarget
{

	void UpdateRenderTargetViews(CComPtr<ID3D12Device2>& device, CComPtr<IDXGISwapChain4>& swapChain, CComPtr<ID3D12DescriptorHeap>& descriptorHeap, CComPtr<ID3D12Resource>* renderTargets, UINT bufferCount);
	void UpdateDepthStencilView(CComPtr<ID3D12Device2>& device, CComPtr<ID3D12DescriptorHeap>& descriptorHeap, CComPtr<ID3D12Resource>& depthStencil, UINT width, UINT height);

	//D3D11On12
	void UpdateD2D1RenderTargets(CComPtr<ID3D11On12Device>& d3d11on12Device, CComPtr<ID2D1DeviceContext5>& d2d1DeviceContext, CComPtr<ID3D12Resource>* renderTargets, CComPtr<ID3D11Resource>* wrappedBackBuffers, CComPtr<ID2D1Bitmap1>* d2dRenderTargets, UINT bufferCount);

};

