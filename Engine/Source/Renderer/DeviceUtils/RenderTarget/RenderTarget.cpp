#include "pch.h"
#include "RenderTarget.h"
#include "../../../Common/DirectXHelper.h"

namespace DeviceUtils::RenderTarget {

	void UpdateRenderTargetViews(CComPtr<ID3D12Device2>& device, CComPtr<IDXGISwapChain4>& swapChain, CComPtr<ID3D12DescriptorHeap>& descriptorHeap, CComPtr<ID3D12Resource>* renderTargets, UINT bufferCount)
	{
		auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT i = 0; i < bufferCount; ++i) {
			CComPtr<ID3D12Resource> backBuffer;
			DX::ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
			device->CreateRenderTargetView(backBuffer, nullptr, rtvHandle);
			renderTargets[i] = backBuffer;
			rtvHandle.Offset(rtvDescriptorSize);
		}
	}

	void UpdateDepthStencilView(CComPtr<ID3D12Device2>& device, CComPtr<ID3D12DescriptorHeap>& descriptorHeap, CComPtr<ID3D12Resource>& depthStencil, UINT width, UINT height)
	{
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		optimizedClearValue.DepthStencil = { 1.0f, 0 };

		const CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		const CD3DX12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
			1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		DX::ThrowIfFailed(device->CreateCommittedResource(
			&depthHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&optimizedClearValue,
			IID_PPV_ARGS(&depthStencil)
		));
		//NAME_D3D12_OBJECT(depthStencil);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
		dsv.Format = DXGI_FORMAT_D32_FLOAT;
		dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsv.Texture2D.MipSlice = 0;
		dsv.Flags = D3D12_DSV_FLAG_NONE;

		device->CreateDepthStencilView(depthStencil, &dsv, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}

	void UpdateD2D1RenderTargets(CComPtr<ID3D11On12Device>& d3d11on12Device, CComPtr<ID2D1DeviceContext5>& d2d1DeviceContext, CComPtr<ID3D12Resource>* renderTargets, CComPtr<ID3D11Resource> *wrappedBackBuffers, CComPtr<ID2D1Bitmap1> *d2dRenderTargets, UINT bufferCount) {
		D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), 144.0f, 144.0f
		);

		for (UINT i = 0; i < bufferCount; i++) {
			D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
			DX::ThrowIfFailed(d3d11on12Device->CreateWrappedResource(
				renderTargets[i], &d3d11Flags,
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT,
				IID_PPV_ARGS(&wrappedBackBuffers[i])
			));

			CComPtr<IDXGISurface> surface;
			DX::ThrowIfFailed(wrappedBackBuffers[i].QueryInterface(&surface));
			DX::ThrowIfFailed(d2d1DeviceContext->CreateBitmapFromDxgiSurface(surface, &bitmapProperties, &d2dRenderTargets[i]));
		}
	}

}