#pragma once
#include "../DeviceUtils/RenderToTexture/RenderToTexture.h"
#include "../DeviceUtils/DescriptorHeap/DescriptorHeap.h"

using namespace DeviceUtils;
using namespace DirectX;
struct Renderer;

namespace RenderPass
{
	struct SwapChainPass
	{
		std::string name;
		D3D12_VIEWPORT screenViewport;
		D3D12_RECT scissorRect;
		std::shared_ptr<DeviceUtils::DescriptorHeap> rtvDescriptorHeap;
		std::vector<CComPtr<ID3D12Resource>> renderTargets;

		void BeginRenderPass(std::shared_ptr<DeviceUtils::DescriptorHeap> dsvDescriptorHeap = nullptr, bool clearRTV = true, XMVECTORF32 clearColor = DirectX::Colors::Black);
		void CopyFromRenderToTexture(const std::shared_ptr<RenderToTexture>& renderToTexture);
		void EndRenderPass();
		void ReleaseResources();
		void Resize(unsigned int width, unsigned int height);
	};

	struct RenderToTexturePass
	{
		std::string name;
		D3D12_VIEWPORT screenViewport;
		D3D12_RECT scissorRect;
		std::vector<std::shared_ptr<RenderToTexture>> renderToTexture;
		DXGI_FORMAT depthStencilFormat;
		CComPtr<ID3D12DescriptorHeap> depthStencilViewDescriptorHeap;
		CComPtr<ID3D12Resource> depthStencilTexture;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDepthStencilTextureHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDepthStencilTextureHandle;

		~RenderToTexturePass() { Destroy(); }
		void BeginRenderPass(XMVECTORF32 clearColor = DirectX::Colors::Black);
		void EndRenderPass();
		void DebugX();
		void Destroy();
		void ReleaseResources();
		void Resize(unsigned int width, unsigned int height);
	};

	//struct RenderPass {

		/*
		std::vector<RenderToTexturePtr> renderToTexture;

		CComPtr<ID3D12DescriptorHeap> depthStencilViewDescriptorHeap;
		CComPtr<ID3D12Resource> depthStencilTexture;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDepthStencilTextureHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDepthStencilTextureHandle;
		*/
		//};

	std::shared_ptr<SwapChainPass> CreateRenderPass(const std::string name, std::shared_ptr<DeviceUtils::DescriptorHeap>& descriptorHeap);
	std::shared_ptr<RenderToTexturePass> CreateRenderPass(const std::string name, std::vector<DXGI_FORMAT> renderTargetsFormats, DXGI_FORMAT depthStencilFormat, unsigned int width, unsigned int height);

	/*
	template<bool useSwapChain, size_t size, D3D12_DESCRIPTOR_HEAP_TYPE heapType, D3D12_DESCRIPTOR_HEAP_FLAGS flags>
	std::shared_ptr<RenderPass> CreateRenderPass(
		std::shared_ptr<Renderer>& renderer, DeviceUtils::DescriptorHeap::DescriptorHeap<size, heapType, flags>& descriptorHeap
		DXGI_FORMAT depthStencilFormat,
		unsigned int width, unsigned int height) { }

	//use swapchain render target
	template<true, size_t size, D3D12_DESCRIPTOR_HEAP_TYPE heapType, D3D12_DESCRIPTOR_HEAP_FLAGS flags>
	std::shared_ptr<RenderPass> CreateRenderPass(
		std::shared_ptr<Renderer>& renderer, DeviceUtils::DescriptorHeap::DescriptorHeap<size, heapType, flags>& descriptorHeap
		std::vector<DXGI_FORMAT> renderTargetsFormats, DXGI_FORMAT depthStencilFormat,
		unsigned int width, unsigned int height) { }

	//use render to texture
	template<false, size_t size, D3D12_DESCRIPTOR_HEAP_TYPE heapType, D3D12_DESCRIPTOR_HEAP_FLAGS flags>
	std::shared_ptr<RenderPass> CreateRenderPass(
		std::shared_ptr<Renderer>& renderer, DeviceUtils::DescriptorHeap::DescriptorHeap<size, heapType, flags>& descriptorHeap
		std::vector<DXGI_FORMAT> renderTargetsFormats, DXGI_FORMAT depthStencilFormat,
		unsigned int width, unsigned int height) { }
		*/
		/*
		std::shared_ptr<RenderPass> CreateRenderPass(
			CComPtr<ID3D12Device2>& device,
			std::vector<DXGI_FORMAT> renderTargetsFormats,
			DXGI_FORMAT depthStencilFormat,
			unsigned int width,
			unsigned int height
		);

		void DestroyRenderPass(std::shared_ptr<RenderPass> renderPass);
		*/

};

