#pragma once

#include "../DescriptorHeap/DescriptorHeap.h"
#include <dxgiformat.h>

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils
{
	static constexpr unsigned int maxRenderToTexturesRenderTargets = 100U;

	struct RenderToTexture {
		std::string name;
		DXGI_FORMAT format;
		unsigned int width;
		unsigned int height;
		D3D12_RESOURCE_DESC resourceDesc;
		CComPtr<ID3D12Resource> renderToTexture;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuRenderTargetViewHandle;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuTextureHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuTextureHandle;

		~RenderToTexture() { Destroy(); }
		void Create();
		void ReleaseResources();
		void Resize(unsigned int width, unsigned int height);
		void Destroy();
	};

	//CREATE
	void CreateRenderToTextureDescriptorHeap();

	//DESTROY
	void DestroyRenderToTextureDescriptorHeap();

};
