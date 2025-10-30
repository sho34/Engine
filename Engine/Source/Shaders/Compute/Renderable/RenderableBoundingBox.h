#pragma once
#include <DirectXCollision.h>
#include <vector>
#include <atlbase.h>
#include <wrl/client.h>
#include "../ComputeInterface.h"
#include "../../../Common/d3dx12.h"
#include "../../../Common/DirectXHelper.h"

namespace ComputeShader
{

	struct RenderableBoundingBox : public ComputeInterface
	{
		ConstantsBufferUUID bonesCbv;
		BoundingBox boundingBox;

		//Animable/Compute Shader stuff
		std::vector<CComPtr<ID3D12Resource>> resources;
		std::vector<CComPtr<ID3D12Resource>> readBackResources;
		std::vector<ConstantsBufferUUID> constantsBuffers; //CBV, 0
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> verticesCpuHandles; //SRV, 3
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> verticesGpuHandles;	//SRV, 3
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> resultCpuHandle;	//UAV, 2
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> resultGpuHandle; //UAV, 2

		RenderableBoundingBox(JUUID renderableUUID);
		~RenderableBoundingBox();

		virtual void Compute();
		virtual void Solution();
	};

	JUUID CreateRenderableBoundingBox(RenderableUUID renderable);
	std::unique_ptr<RenderableBoundingBox>& GetRenderableBoundingBox(JUUID compUUID);
	void DeleteRenderableBoundingBox(JUUID compUUID);
}

