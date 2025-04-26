#pragma once
#include "../DeviceUtils/RenderToTexture/RenderToTexture.h"
#include "../DeviceUtils/DescriptorHeap/DescriptorHeap.h"

using namespace DeviceUtils;
using namespace DirectX;
struct Renderer;

typedef std::tuple<std::vector<DXGI_FORMAT>, DXGI_FORMAT> RenderPassRenderTargetDesc;

namespace RenderPass
{
	struct SwapChainPass
	{
		size_t passHash = -1;
		std::string name;
		D3D12_VIEWPORT screenViewport;
		D3D12_RECT scissorRect;
		std::shared_ptr<DeviceUtils::DescriptorHeap> rtvDescriptorHeap;
		std::vector<CComPtr<ID3D12Resource>> renderTargets;

		void Pass(std::function<void(size_t)> renderCallback, std::shared_ptr<DeviceUtils::DescriptorHeap> dsvDescriptorHeap = nullptr, bool clearRTV = true, XMVECTORF32 clearColor = DirectX::Colors::Black);
		void BeginRenderPass(std::shared_ptr<DeviceUtils::DescriptorHeap> dsvDescriptorHeap = nullptr, bool clearRTV = true, XMVECTORF32 clearColor = DirectX::Colors::Black);
		void CopyFromRenderToTexture(const std::shared_ptr<RenderToTexture>& renderToTexture);
		void EndRenderPass();
		void ReleaseResources();
		void Resize(unsigned int width, unsigned int height);
	};

	struct RenderToTexturePass
	{
		size_t passHash = -1;
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
		void Pass(std::function<void(size_t)> renderCallback, XMVECTORF32 clearColor = DirectX::Colors::Black);
		void BeginRenderPass(XMVECTORF32 clearColor = DirectX::Colors::Black);
		void EndRenderPass();
		void Destroy();
		void ReleaseResources();
		void Resize(unsigned int width, unsigned int height);
	};

	RenderPassRenderTargetDesc& GetRenderPassRenderTargetDesc(size_t passHash);

	std::shared_ptr<SwapChainPass> CreateRenderPass(const std::string name, std::shared_ptr<DeviceUtils::DescriptorHeap>& descriptorHeap);
	std::shared_ptr<RenderToTexturePass> CreateRenderPass(const std::string name, std::vector<DXGI_FORMAT> renderTargetsFormats, DXGI_FORMAT depthStencilFormat, unsigned int width, unsigned int height);
};

