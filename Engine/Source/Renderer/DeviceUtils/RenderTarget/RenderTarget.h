#pragma once

#include "../../Renderer.h"

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils
{
	void UpdateRenderTargetViews(CComPtr<ID3D12Device2>& device, CComPtr<IDXGISwapChain4>& swapChain, CComPtr<ID3D12DescriptorHeap>& descriptorHeap, CComPtr<ID3D12Resource>* renderTargets, UINT bufferCount);
	void UpdateDepthStencilView(CComPtr<ID3D12Device2>& device, CComPtr<ID3D12DescriptorHeap>& descriptorHeap, CComPtr<ID3D12Resource>& depthStencil, DXGI_FORMAT format, UINT width, UINT height);
};

