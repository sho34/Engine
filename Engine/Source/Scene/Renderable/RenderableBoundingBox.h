#pragma once
#include <DirectXCollision.h>
#include <vector>
#include <atlbase.h>
#include <wrl/client.h>
#include "../../Common/d3dx12.h"
#include "../../Common/DirectXHelper.h"
#include "../../Renderer/DeviceUtils/RootSignature/RootSignature.h"
#include "../../Renderer/DeviceUtils/PipelineState/PipelineState.h"
#include "../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"

namespace Scene
{
	using namespace DeviceUtils;

	struct Renderable;

	struct RenderableBoundingBox
	{
		std::shared_ptr<Renderable> renderable;
		BoundingBox boundingBox;

		//Animable/Compute Shader stuff
		std::vector<CComPtr<ID3D12Resource>> resources;
		std::vector<CComPtr<ID3D12Resource>> readBackResources;
		std::vector<std::shared_ptr<ConstantsBuffer>> constantsBuffers; //CBV
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> verticesCpuHandles; //SRV
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> verticesGpuHandles;	//SRV
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> resultCpuHandle;	//UAV
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> resultGpuHandle; //UAV
		std::shared_ptr<ShaderInstance> shader;
		HashedRootSignature rootSignature;
		HashedPipelineState pipelineState;

		void Init(std::shared_ptr<Renderable>& r);
		void CreateComputeResource(CComPtr<ID3D12Resource>& resource, CComPtr<ID3D12Resource>& readBackResource, ::CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, ::CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
		void CreateMeshVerticesShaderResourceView(std::shared_ptr<MeshInstance>& mesh, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
		void CreateDXBoundingBox();
		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);
		void ComputeBoundingBox();
		void SolveDXBoundingBox();
		void Destroy();
	};
}

