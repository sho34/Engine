#pragma once
#include "../DeviceUtils/RenderToTexture/RenderToTexture.h"
#include "../DeviceUtils/DescriptorHeap/DescriptorHeap.h"
#include <DirectXColors.h>
#include <functional>

namespace DeviceUtils
{
	struct SwapChainPass
	{
		std::string name;
		unsigned int width;
		unsigned int height;
		D3D12_VIEWPORT screenViewport;
		D3D12_RECT scissorRect;
		std::shared_ptr<DeviceUtils::DescriptorHeap> rtvDescriptorHeap;
		std::vector<CComPtr<ID3D12Resource>> renderTargets;

		DXGI_FORMAT depthStencilFormat;
		CComPtr<ID3D12DescriptorHeap> depthStencilViewDescriptorHeap;
		CComPtr<ID3D12Resource> depthStencilTexture;

		void Pass(std::function<void()> renderCallback, bool clearRTV = true, XMVECTORF32 clearColor = DirectX::Colors::Black);
		void BeginRenderPass(CComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap, bool clearRTV = true, XMVECTORF32 clearColor = DirectX::Colors::Black);
		void CopyFromRenderToTexture(const std::shared_ptr<RenderToTexture>& renderToTexture);
		void EndRenderPass();
		void ReleaseResources();
		void Resize(unsigned int width, unsigned int height);
	};

	std::shared_ptr<SwapChainPass> CreateRenderPass(const std::string name, std::shared_ptr<DeviceUtils::DescriptorHeap>& descriptorHeap, DXGI_FORMAT depthStencilFormat);
}