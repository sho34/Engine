#include "pch.h"
#include "RenderPass.h"
#include "../../Common/DirectXHelper.h"
#include "../DeviceUtils/RenderTarget/RenderTarget.h"
#include "../DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include "../../Common/d3dx12.h"
#include <map>

using namespace DeviceUtils;

extern std::shared_ptr<Renderer> renderer;

template <>
struct std::hash<RenderPassRenderTargetDesc>
{
	std::size_t operator()(const RenderPassRenderTargetDesc& r) const
	{
		using std::hash;
		const std::vector<DXGI_FORMAT>& rtFormats = std::get<0>(r);
		DXGI_FORMAT depthFormat = std::get<1>(r);
		size_t h = 0ULL;
		nostd::hash_combine(h, rtFormats, depthFormat);
		return h;
	}
};


namespace RenderPass {

	static std::map<size_t, RenderPassRenderTargetDesc> hashesRenderPassesRenderTargets;

	std::map<DXGI_FORMAT, DXGI_FORMAT> depthFormatConversion = {
		{ DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT }
	};

	RenderPassRenderTargetDesc& GetRenderPassRenderTargetDesc(size_t passHash)
	{
		return hashesRenderPassesRenderTargets.at(passHash);
	}

	std::shared_ptr<SwapChainPass> CreateRenderPass(const std::string name, std::shared_ptr<DeviceUtils::DescriptorHeap>& descriptorHeap)
	{
		unsigned int bufferCount = renderer->numFrames;
		std::shared_ptr<SwapChainPass> swapChainPass = std::make_shared<SwapChainPass>();
		swapChainPass->name = name;
		swapChainPass->rtvDescriptorHeap = descriptorHeap;
		nostd::VecN_push_back(bufferCount, swapChainPass->renderTargets);
		UpdateRenderTargetViews(renderer->d3dDevice, renderer->swapChain, descriptorHeap->descriptorHeap, swapChainPass->renderTargets.data(), bufferCount);
		for (unsigned int i = 0U; i < bufferCount; i++)
		{
			std::string passName = name + "[" + std::to_string(i) + "]";
			CCNAME_D3D12_OBJECT_N(swapChainPass->renderTargets[i], passName);
		}
		swapChainPass->screenViewport = renderer->screenViewport;
		swapChainPass->scissorRect = renderer->scissorRect;
		return swapChainPass;
	}

	void SwapChainPass::Pass(std::function<void(size_t)> renderCallback, std::shared_ptr<DeviceUtils::DescriptorHeap> dsvDescriptorHeap, bool clearRTV, XMVECTORF32 clearColor)
	{
		if (passHash == -1)
		{
			std::vector<DXGI_FORMAT> rtFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };
			DXGI_FORMAT dsFormat = (dsvDescriptorHeap ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_UNKNOWN);
			RenderPassRenderTargetDesc passRTDesc = std::tie(rtFormats, dsFormat);

			passHash = std::hash<RenderPassRenderTargetDesc>()(passRTDesc);
			hashesRenderPassesRenderTargets.insert_or_assign(passHash, passRTDesc);
		}

		BeginRenderPass(dsvDescriptorHeap, clearRTV, clearColor);
		renderCallback(passHash);
		EndRenderPass();
	}

	void SwapChainPass::BeginRenderPass(std::shared_ptr<DeviceUtils::DescriptorHeap> dsvDescriptorHeap, bool clearRTV, XMVECTORF32 clearColor)
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
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(dsvDescriptorHeap->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
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
		unsigned int bufferCount = renderer->numFrames;
		nostd::VecN_push_back(bufferCount, renderTargets);
		UpdateRenderTargetViews(renderer->d3dDevice, renderer->swapChain, rtvDescriptorHeap->descriptorHeap, renderTargets.data(), bufferCount);
		for (unsigned int i = 0U; i < bufferCount; i++)
		{
			std::string passName = name + "[" + std::to_string(i) + "]";
			CCNAME_D3D12_OBJECT_N(renderTargets[i], passName);
		}
		screenViewport = renderer->screenViewport;
		scissorRect = renderer->scissorRect;
	}

	std::shared_ptr<RenderToTexturePass> CreateRenderPass(const std::string name, std::vector<DXGI_FORMAT> renderTargetsFormats, DXGI_FORMAT depthStencilFormat, unsigned int width, unsigned int height)
	{
		auto& d3dDevice = renderer->d3dDevice;
		std::shared_ptr<RenderToTexturePass> renderPass = std::make_shared<RenderToTexturePass>();

		renderPass->name = name;
		renderPass->screenViewport = {
			.TopLeftX = 0.0f, .TopLeftY = 0.0f,
			.Width = static_cast<float>(width), .Height = static_cast<float>(height),
			.MinDepth = 0.0f, .MaxDepth = 1.0f
		};
		renderPass->scissorRect = {
			.left = 0L, .top = 0L,
			.right = static_cast<long>(width),
			.bottom = static_cast<long>(height)
		};

		for (auto format : renderTargetsFormats)
		{
			std::shared_ptr<RenderToTexture> rtt = std::make_shared<RenderToTexture>();
			rtt->name = name + "[" + std::to_string(renderPass->renderToTexture.size()) + "]";
			rtt->format = format;
			rtt->width = width;
			rtt->height = height;

			renderPass->renderToTexture.push_back(rtt);
			rtt->Create();
			AllocCSUDescriptor(rtt->cpuTextureHandle, rtt->gpuTextureHandle);

			D3D12_SHADER_RESOURCE_VIEW_DESC rttSRVDesc = {
				.Format = format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
			};

			d3dDevice->CreateShaderResourceView(rtt->renderToTexture, &rttSRVDesc, rtt->cpuTextureHandle);
		}

		renderPass->depthStencilFormat = depthStencilFormat;
		if (depthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			renderPass->depthStencilViewDescriptorHeap = CreateDescriptorHeap(d3dDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			CCNAME_D3D12_OBJECT(renderPass->depthStencilViewDescriptorHeap);

			UpdateDepthStencilView(d3dDevice, renderPass->depthStencilViewDescriptorHeap, renderPass->depthStencilTexture, depthStencilFormat, width, height);
			CCNAME_D3D12_OBJECT_N(renderPass->depthStencilTexture, name);

			AllocCSUDescriptor(renderPass->cpuDepthStencilTextureHandle, renderPass->gpuDepthStencilTextureHandle);

			D3D12_SHADER_RESOURCE_VIEW_DESC depthStencilSRVDesc = {
				.Format = depthFormatConversion.contains(depthStencilFormat) ? depthFormatConversion.at(depthStencilFormat) : depthStencilFormat,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
			};

			d3dDevice->CreateShaderResourceView(renderPass->depthStencilTexture, &depthStencilSRVDesc, renderPass->cpuDepthStencilTextureHandle);
		}

		if (renderPass->passHash == -1)
		{
			RenderPassRenderTargetDesc passRTDesc = std::tie(renderTargetsFormats, depthStencilFormat);
			renderPass->passHash = std::hash<RenderPassRenderTargetDesc>()(passRTDesc);
			hashesRenderPassesRenderTargets.insert_or_assign(renderPass->passHash, passRTDesc);
		}

		return renderPass;
	}

	void RenderToTexturePass::Pass(std::function<void(size_t)> renderCallback, XMVECTORF32 clearColor)
	{
		BeginRenderPass(clearColor);
		renderCallback(passHash);
		EndRenderPass();
	}

	void RenderToTexturePass::BeginRenderPass(XMVECTORF32 clearColor)
	{
		auto commandList = renderer->commandList;
#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, name.c_str());
#endif

		//transition the texture resources from pixel shader resource to render target
		std::vector<CD3DX12_RESOURCE_BARRIER> barriers;
		for (auto& rtt : renderToTexture)
		{
			barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(rtt->renderToTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
		}
		if (barriers.size())
		{
			commandList->ResourceBarrier(static_cast<unsigned int>(barriers.size()), barriers.data());
		}

		commandList->RSSetViewports(1, &screenViewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
		for (auto& rtt : renderToTexture)
		{
			commandList->ClearRenderTargetView(rtt->cpuRenderTargetViewHandle, clearColor, 0, nullptr);
			rtvHandles.push_back(rtt->cpuRenderTargetViewHandle);
		}

		if (depthStencilViewDescriptorHeap)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(depthStencilViewDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
			commandList->OMSetRenderTargets(static_cast<unsigned int>(rtvHandles.size()), rtvHandles.data(), false, &dsv);
		}
		else
		{
			commandList->OMSetRenderTargets(static_cast<unsigned int>(rtvHandles.size()), rtvHandles.data(), false, nullptr);
		}
	}

	void RenderToTexturePass::EndRenderPass()
	{
		auto commandList = renderer->commandList;

		//transition the texture resources from render target to pixel shader resource
		std::vector<CD3DX12_RESOURCE_BARRIER> barriers;
		for (auto& rtt : renderToTexture)
		{
			barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(rtt->renderToTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		}
		if (barriers.size())
		{
			commandList->ResourceBarrier(static_cast<unsigned int>(barriers.size()), barriers.data());
		}
#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
	}

	void RenderToTexturePass::Destroy()
	{
		renderToTexture.clear();
		if (depthStencilTexture != nullptr)
		{
			FreeCSUDescriptor(cpuDepthStencilTextureHandle, gpuDepthStencilTextureHandle);
		}
	}

	void RenderToTexturePass::ReleaseResources()
	{
		for (auto& rtt : renderToTexture)
		{
			rtt->ReleaseResources();
		}

		if (depthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			depthStencilTexture = nullptr;
			FreeCSUDescriptor(cpuDepthStencilTextureHandle, gpuDepthStencilTextureHandle);
		}
	}

	void RenderToTexturePass::Resize(unsigned int width, unsigned int height)
	{
		auto& d3dDevice = renderer->d3dDevice;

		screenViewport = {
			.TopLeftX = 0.0f, .TopLeftY = 0.0f,
			.Width = static_cast<float>(width), .Height = static_cast<float>(height),
			.MinDepth = 0.0f, .MaxDepth = 1.0f
		};
		scissorRect = {
			.left = 0L, .top = 0L,
			.right = static_cast<long>(width),
			.bottom = static_cast<long>(height)
		};

		for (auto& rtt : renderToTexture)
		{
			rtt->Resize(width, height);
		}

		if (depthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			UpdateDepthStencilView(d3dDevice, depthStencilViewDescriptorHeap, depthStencilTexture, depthStencilFormat, width, height);
			CCNAME_D3D12_OBJECT_N(depthStencilTexture, name);

			AllocCSUDescriptor(cpuDepthStencilTextureHandle, gpuDepthStencilTextureHandle);

			D3D12_SHADER_RESOURCE_VIEW_DESC depthStencilSRVDesc = {
				.Format = depthFormatConversion.contains(depthStencilFormat) ? depthFormatConversion.at(depthStencilFormat) : depthStencilFormat,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
			};

			d3dDevice->CreateShaderResourceView(depthStencilTexture, &depthStencilSRVDesc, cpuDepthStencilTextureHandle);
		}
	}
}