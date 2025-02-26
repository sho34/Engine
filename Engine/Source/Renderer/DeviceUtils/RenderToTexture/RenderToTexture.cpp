#include "pch.h"
#include "RenderToTexture.h"
#include "../D3D12Device/Builder.h"
#include "../../Renderer.h"
#include "../../../Common/DirectXHelper.h"

extern std::shared_ptr<Renderer> renderer;

namespace DeviceUtils {

	std::shared_ptr<DeviceUtils::DescriptorHeap> renderToTextureDescriptorHeap;

	void CreateRenderToTextureDescriptorHeap()
	{
		auto& d3dDevice = renderer->d3dDevice;
		renderToTextureDescriptorHeap = std::make_shared<DeviceUtils::DescriptorHeap>();
		renderToTextureDescriptorHeap->CreateDescriptorHeap(d3dDevice, maxRenderToTexturesRenderTargets);
	}

	void DestroyRenderToTextureDescriptorHeap()
	{
		renderToTextureDescriptorHeap->DestroyDescriptorHeap();
		renderToTextureDescriptorHeap = nullptr;
	}

	void RenderToTexture::Create()
	{
		auto& d3dDevice = renderer->d3dDevice;

		const CD3DX12_HEAP_PROPERTIES renderToTextureHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		const CD3DX12_RESOURCE_DESC renderToTextureRenderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			format, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
		);

		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&renderToTextureHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&renderToTextureRenderTargetDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(&renderToTexture)
		));

		CCNAME_D3D12_OBJECT_N(renderToTexture, name);

		D3D12_RENDER_TARGET_VIEW_DESC rttdesc;
		rttdesc.Format = format;
		rttdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rttdesc.Texture2D.MipSlice = 0;
		rttdesc.Texture2D.PlaneSlice = 0;

		renderToTextureDescriptorHeap->AllocCPUDescriptor(cpuRenderTargetViewHandle);

		d3dDevice->CreateRenderTargetView(renderToTexture, &rttdesc, cpuRenderTargetViewHandle);
	}

	void RenderToTexture::ReleaseResources()
	{
		renderToTexture = nullptr;
		Destroy();
	}

	void RenderToTexture::Resize(unsigned int width, unsigned int height)
	{
		this->width = width;
		this->height = height;
		Create();
	}

	void RenderToTexture::Destroy()
	{
		if (!renderToTextureDescriptorHeap) return;

		renderToTextureDescriptorHeap->FreeCPUDescriptor(cpuRenderTargetViewHandle);
	}
}