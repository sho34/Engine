#include "pch.h"
#include "SwapChainPass.h"
#include <Renderer.h>
#include <DirectXHelper.h>
#include <DeviceUtils/RenderTarget/RenderTarget.h>
#include <DeviceUtils/DescriptorHeap/DescriptorHeap.h>
#include <NoStd.h>

extern std::shared_ptr<Renderer> renderer;

namespace DeviceUtils
{
	std::shared_ptr<SwapChainPass> CreateRenderPass(const std::string name, std::shared_ptr<DeviceUtils::DescriptorHeap>& descriptorHeap, DXGI_FORMAT depthStencilFormat)
	{
		auto& d3dDevice = renderer->d3dDevice;

		unsigned int bufferCount = renderer->numFrames;
		std::shared_ptr<SwapChainPass> swapChainPass = std::make_shared<SwapChainPass>();
		swapChainPass->name = name;
		swapChainPass->rtvDescriptorHeap = descriptorHeap;
		nostd::VecN_push_back(bufferCount, swapChainPass->renderTargets);
		UpdateRenderTargetViews(d3dDevice, renderer->swapChain, descriptorHeap->descriptorHeap, swapChainPass->renderTargets.data(), bufferCount);
		for (unsigned int i = 0U; i < bufferCount; i++)
		{
			std::string passName = name + "[" + std::to_string(i) + "]";
			CCNAME_D3D12_OBJECT_N(swapChainPass->renderTargets[i], passName);
			LogCComPtrAddress(passName, swapChainPass->renderTargets[i]);
		}
		swapChainPass->screenViewport = renderer->screenViewport;
		swapChainPass->scissorRect = renderer->scissorRect;
		swapChainPass->width = static_cast<unsigned int>(abs(renderer->scissorRect.right - renderer->scissorRect.left));
		swapChainPass->height = static_cast<unsigned int>(abs(renderer->scissorRect.bottom - renderer->scissorRect.top));

		swapChainPass->depthStencilFormat = depthStencilFormat;
		if (depthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			swapChainPass->depthStencilViewDescriptorHeap = CreateDescriptorHeap(d3dDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			CCNAME_D3D12_OBJECT(swapChainPass->depthStencilViewDescriptorHeap);

			UpdateDepthStencilView(d3dDevice, swapChainPass->depthStencilViewDescriptorHeap, swapChainPass->depthStencilTexture, depthStencilFormat, swapChainPass->width, swapChainPass->height);
			CCNAME_D3D12_OBJECT_N(swapChainPass->depthStencilTexture, name);
		}

		return swapChainPass;
	}

	void SwapChainPass::Pass(std::function<void()> renderCallback, bool clearRTV, XMVECTORF32 clearColor)
	{
		BeginRenderPass(depthStencilViewDescriptorHeap, clearRTV, clearColor);
		renderCallback();
		EndRenderPass();
	}

	void SwapChainPass::BeginRenderPass(CComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap, bool clearRTV, XMVECTORF32 clearColor)
	{
		unsigned int backBufferIndex = renderer->backBufferIndex;
		auto backBuffer = renderTargets[backBufferIndex];
		auto commandList = renderer->commandList;

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, name.c_str());
#endif

		//transition the back buffer from present to render target so it's allowed to draw
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier);

		commandList->RSSetViewports(1, &screenViewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(rtvDescriptorHeap->descriptorHeap->GetCPUDescriptorHandleForHeapStart(), backBufferIndex, rtvDescriptorHeap->descriptorSize);
		if (clearRTV)
		{
			commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
		}

		if (dsvDescriptorHeap)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
			commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
		}
		else
		{
			commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
		}
	}

	void SwapChainPass::CopyFromRenderToTexture(const std::shared_ptr<RenderToTexture>& renderToTexture)
	{
		unsigned int backbufferIndex = renderer->backBufferIndex;
		auto commandList = renderer->commandList;
		auto backbuffer = renderTargets[backbufferIndex];
		auto rtt = renderToTexture->renderToTexture;

		std::vector<CD3DX12_RESOURCE_BARRIER> hold = {
			CD3DX12_RESOURCE_BARRIER::Transition(backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST),
			CD3DX12_RESOURCE_BARRIER::Transition(rtt, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE),
		};
		commandList->ResourceBarrier((unsigned int)hold.size(), hold.data());

		D3D12_TEXTURE_COPY_LOCATION src = { .pResource = renderTargets[backbufferIndex], .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, .SubresourceIndex = 0U };
		D3D12_TEXTURE_COPY_LOCATION dst = { .pResource = renderToTexture->renderToTexture, .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, .SubresourceIndex = 0U };
		commandList->CopyTextureRegion(&src, 0, 0, 0, &dst, nullptr);

		std::vector<CD3DX12_RESOURCE_BARRIER> release = {
			CD3DX12_RESOURCE_BARRIER::Transition(backbuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET),
			CD3DX12_RESOURCE_BARRIER::Transition(rtt, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		};
		commandList->ResourceBarrier((unsigned int)release.size(), release.data());
	}

	void SwapChainPass::EndRenderPass()
	{
		unsigned int backBufferIndex = renderer->backBufferIndex;
		auto backBuffer = renderTargets[backBufferIndex];
		auto commandList = renderer->commandList;
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		commandList->ResourceBarrier(1, &barrier);

#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
	}

	void SwapChainPass::ReleaseResources()
	{
		renderTargets.clear();
	}

	void SwapChainPass::Resize(unsigned int width, unsigned int height)
	{
		this->width = width;
		this->height = height;
		unsigned int bufferCount = renderer->numFrames;
		nostd::VecN_push_back(bufferCount, renderTargets);
		UpdateRenderTargetViews(renderer->d3dDevice, renderer->swapChain, rtvDescriptorHeap->descriptorHeap, renderTargets.data(), bufferCount);
		for (unsigned int i = 0U; i < bufferCount; i++)
		{
			std::string passName = name + "[" + std::to_string(i) + "]";
			CCNAME_D3D12_OBJECT_N(renderTargets[i], passName);
		}
		screenViewport = renderer->screenViewport;
		screenViewport.Width = std::min(static_cast<float>(width), screenViewport.Width);
		screenViewport.Height = std::min(static_cast<float>(height), screenViewport.Height);
		scissorRect = renderer->scissorRect;
		scissorRect.right = std::min(static_cast<LONG>(width), scissorRect.right);
		scissorRect.bottom = std::min(static_cast<LONG>(height), scissorRect.bottom);
	}
}