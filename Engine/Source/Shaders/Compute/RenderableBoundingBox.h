#pragma once
#include <DirectXCollision.h>
#include <vector>
#include <atlbase.h>
#include <wrl/client.h>
#include "ComputeInterface.h"
#include "../../Common/d3dx12.h"
#include "../../Common/DirectXHelper.h"
#include "../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"

namespace Scene { struct Renderable; };

namespace ComputeShader
{
	using namespace DeviceUtils;

	struct RenderableBoundingBox : public ComputeInterface
	{
		std::shared_ptr<ConstantsBuffer> bonesCbv;
		BoundingBox boundingBox;

		//Animable/Compute Shader stuff
		std::vector<CComPtr<ID3D12Resource>> resources;
		std::vector<CComPtr<ID3D12Resource>> readBackResources;
		std::vector<std::shared_ptr<ConstantsBuffer>> constantsBuffers; //CBV, 0
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> verticesCpuHandles; //SRV, 3
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> verticesGpuHandles;	//SRV, 3
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> resultCpuHandle;	//UAV, 2
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> resultGpuHandle; //UAV, 2

		RenderableBoundingBox(std::shared_ptr<Renderable> r);
		~RenderableBoundingBox();

		virtual void Compute();
		virtual void Solution();
	};
}

