#pragma once
#include <DirectXMath.h>
#include "../../Common/d3dx12.h"

namespace Scene { struct Light; }
namespace DeviceUtils { struct ConstantsBuffer; }
enum VertexClass;

namespace Scene {

	using namespace DirectX;
	using namespace DeviceUtils;

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

	ConstantsBufferUUID GetShadowMapConstantsBuffer();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandleStart();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandle(unsigned int index);

	//UPDATE
	void AllocShadowMapSlot(unsigned int slot);
	void FreeShadowMapSlot(unsigned int slot);
	void WriteConstantsBufferShadowMapAttributes(LightUUID light, unsigned int backbufferIndex, unsigned int shadowMapIndex);
	void WriteShadowMapCamerasConstantsBuffers(LightUUID light, unsigned int backbufferIndex);

	//RENDER

	//DELETE
	void DestroyShadowMaps();
	void DestroyShadowMapResources();
};

