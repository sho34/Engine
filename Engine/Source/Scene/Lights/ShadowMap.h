#pragma once
#include <DirectXMath.h>
#include "../../Common/d3dx12.h"

namespace Scene { struct Light; }
namespace DeviceUtils { struct ConstantsBuffer; }
enum VertexClass;

namespace Scene {

	using namespace DirectX;
	using namespace DeviceUtils;

	inline static const D3D12_CLEAR_VALUE shadowMapOptimizedClearValue = {
		.Format = DXGI_FORMAT_D32_FLOAT, .DepthStencil = { 1.0f , 0 },
	};
	inline static const D3D12_DEPTH_STENCIL_VIEW_DESC shadowMapDepthStencilViewDesc = {
		.Format = DXGI_FORMAT_D32_FLOAT, .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
		.Flags = D3D12_DSV_FLAG_NONE, .Texture2D = {.MipSlice = 0 },
	};
	inline static const D3D12_SHADER_RESOURCE_VIEW_DESC shadowMapSrvDesc = {
		.Format = DXGI_FORMAT_R32_FLOAT,
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
	};

#if defined(_EDITOR)
	enum ShadowMapUpdateFlags {
		ShadowMapUpdateFlags_CreateShadowMap = 0x1,
		ShadowMapUpdateFlags_DestroyShadowMap = 0x2,
		ShadowMapUpdateFlags_RebuildShadowMap = ShadowMapUpdateFlags_CreateShadowMap | ShadowMapUpdateFlags_DestroyShadowMap,

		ShadowMapUpdateFlags_CreateShadowMapMinMaxChain = 0x4,
		ShadowMapUpdateFlags_DestroyShadowMapMinMaxChain = 0x8,
		ShadowMapUpdateFlags_RebuildShadowMapMinMaxChain = ShadowMapUpdateFlags_CreateShadowMapMinMaxChain | ShadowMapUpdateFlags_DestroyShadowMapMinMaxChain,

		ShadowMapUpdateFlags_CreateBoth = ShadowMapUpdateFlags_CreateShadowMap | ShadowMapUpdateFlags_CreateShadowMapMinMaxChain,
		ShadowMapUpdateFlags_DestroyBoth = ShadowMapUpdateFlags_DestroyShadowMap | ShadowMapUpdateFlags_DestroyShadowMapMinMaxChain,
		ShadowMapUpdateFlags_RebuildBoth = ShadowMapUpdateFlags_CreateBoth | ShadowMapUpdateFlags_DestroyBoth
	};
#endif

	struct ShadowMapAttributes {
		XMMATRIX atts0;
		XMMATRIX atts1;
		XMMATRIX atts2;
		XMMATRIX atts3;
		XMMATRIX atts4;
		XMMATRIX atts5;
		XMFLOAT4 atts6;
	};


	//CREATE
	void CreateShadowMapResources();

	//READ&GET
	bool SceneHasShadowMaps();
	unsigned int GetNextAvailableShadowMapSlot();

	std::shared_ptr<ConstantsBuffer> GetShadowMapConstantsBuffer();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandleStart();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandle(unsigned int index);

	//UPDATE
	void AllocShadowMapSlot(unsigned int slot);
	void FreeShadowMapSlot(unsigned int slot);
	void UpdateConstantsBufferShadowMapAttributes(const std::shared_ptr<Light>& light, unsigned int backbufferIndex, unsigned int shadowMapIndex);

	//RENDER

	//DELETE
	void DestroyShadowMaps();
	void DestroyShadowMapResources();
};

