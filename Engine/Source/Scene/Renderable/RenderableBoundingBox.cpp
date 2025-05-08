#include "pch.h"
#include "RenderableBoundingBox.h"
#include "Renderable.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/DeviceUtils/Resources/Resources.h"

extern std::shared_ptr<Renderer> renderer;

namespace Scene
{
	void RenderableBoundingBox::Init(std::shared_ptr<Renderable>& r)
	{
		renderable = r;
		if (!r->animable) return;

		for (auto& mesh : r->meshes)
		{
			//Create the bounding box compute resource
			CD3DX12_CPU_DESCRIPTOR_HANDLE uavCpuHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE uavGpuHandle;
			CComPtr<ID3D12Resource> bbox;
			CComPtr<ID3D12Resource> bboxrb;
			CreateComputeResource(bbox, bboxrb, uavCpuHandle, uavGpuHandle);
			resources.push_back(bbox);
			readBackResources.push_back(bboxrb);
			resultCpuHandle.push_back(uavCpuHandle);
			resultGpuHandle.push_back(uavGpuHandle);

			//Create the SRV for the vertex buffers of each mesh
			CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpuHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle;
			CreateMeshVerticesShaderResourceView(mesh, srvCpuHandle, srvGpuHandle);
			verticesCpuHandles.push_back(srvCpuHandle);
			verticesGpuHandles.push_back(srvGpuHandle);
		}

		//Get an instance of the BoundingBox Compute shader
		shader = GetShaderInstance({ .shaderType = COMPUTE_SHADER, .shaderUUID = FindShaderUUIDByName("BoundingBox_cs") });

		//Create the Constants Buffer for each mesh
		std::transform(r->meshes.begin(), r->meshes.end(), std::back_inserter(constantsBuffers), [this](auto& mesh)
			{
				auto cbv = CreateConstantsBuffer(shader->cbufferSize[0], "bboxCS." + mesh->uuid);
				unsigned int numVertices = mesh->vbvData.vertexBufferView.SizeInBytes / mesh->vbvData.vertexBufferView.StrideInBytes;
				cbv->push<unsigned int>(numVertices, 0);
				return cbv;
			}
		);

		//Build the shader's root signature
		auto& vsCBparams = shader->constantsBuffersParameters;
		auto& psCBparams = shader->constantsBuffersParameters;
		auto& uavParams = shader->uavParameters;
		auto& psSRVCSparams = shader->srvCSParameters;
		auto& psSRVTexparams = shader->srvTexParameters;
		auto& psSamplersParams = shader->samplersParameters;
		std::vector<MaterialSamplerDesc> samplers;

		RootSignatureDesc rootSignatureDesc = std::tie(vsCBparams, psCBparams, uavParams, psSRVCSparams, psSRVTexparams, psSamplersParams, samplers);
		rootSignature = CreateRootSignature(rootSignatureDesc);

		size_t rootSignatureHash = std::get<0>(rootSignature);
		ComputePipelineStateDesc pipelineStateDesc = std::tie(shader->byteCode, rootSignatureHash);
		pipelineState = CreateComputePipelineState(pipelineStateDesc);
	}

	void RenderableBoundingBox::CreateComputeResource(CComPtr<ID3D12Resource>& resource, CComPtr<ID3D12Resource>& readBackResource, ::CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, ::CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		size_t dataSize = sizeof(XMFLOAT4) * 2ULL;
		D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		// Create UAV
		renderer->d3dDevice->CreateCommittedResource(
			&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc,
			D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource)
		);

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer = {
			.FirstElement = 0ULL, .NumElements = 1U, .StructureByteStride = sizeof(XMFLOAT4) * 2,
			.CounterOffsetInBytes = 0ULL, .Flags = D3D12_BUFFER_UAV_FLAG_NONE
		};

		DeviceUtils::AllocCSUDescriptor(cpuHandle, gpuHandle);

		renderer->d3dDevice->CreateUnorderedAccessView(resource, nullptr, &uavDesc, cpuHandle);

		//Create ReadBack
		D3D12_HEAP_PROPERTIES readBackHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
		D3D12_RESOURCE_DESC bufferDescReadBack = CD3DX12_RESOURCE_DESC::Buffer(dataSize);

		renderer->d3dDevice->CreateCommittedResource(
			&readBackHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDescReadBack,
			D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&readBackResource)
		);
	}

	void RenderableBoundingBox::CreateMeshVerticesShaderResourceView(std::shared_ptr<MeshInstance>& mesh, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		AllocCSUDescriptor(cpuHandle, gpuHandle);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = mesh->vbvData.vertexBufferView.SizeInBytes / mesh->vbvData.vertexBufferView.StrideInBytes;
		srvDesc.Buffer.StructureByteStride = mesh->vbvData.vertexBufferView.StrideInBytes;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		renderer->d3dDevice->CreateShaderResourceView(mesh->vbvData.vertexBuffer, &srvDesc, cpuHandle);
	}

	void RenderableBoundingBox::CreateDXBoundingBox()
	{
		if (renderable->meshes.size() == 0ULL)
		{
			boundingBox = BoundingBox({ 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f, 0.5f });
			return;
		}
		auto it = renderable->meshes.begin();
		boundingBox = (*it)->boundingBox;
		it++;
		while (it != renderable->meshes.end())
		{
			BoundingBox out;
			BoundingBox::CreateMerged(out, boundingBox, (*it)->boundingBox);
			boundingBox = out;
			it++;
		}
	}

	void RenderableBoundingBox::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox)
	{
		BoundingBox bb = boundingBox;
		XMVECTOR pv = { bb.Center.x, bb.Center.y, bb.Center.z };
		XMFLOAT3 rotV = renderable->rotation();
		XMVECTOR rot = XMQuaternionRotationRollPitchYaw(rotV.x, rotV.y, rotV.z);
		pv = XMVector3Rotate(pv, rot);
		XMFLOAT3 boxP = { pv.m128_f32[0],pv.m128_f32[1],pv.m128_f32[2] };
		bbox->position(boxP * renderable->scale() + renderable->position());
		bbox->scale(bb.Extents * renderable->scale());
		bbox->rotation(rotV);
	}

	void RenderableBoundingBox::ComputeBoundingBox()
	{
		if (!renderable->animable) return;

		CComPtr<ID3D12GraphicsCommandList2>& commandList = renderer->commandList;
		CComPtr<ID3D12RootSignature>& rs = std::get<1>(rootSignature);
		CComPtr<ID3D12PipelineState>& ps = std::get<1>(pipelineState);

		using namespace Animation;
		auto bonesCbv = GetAnimatedConstantsBuffer(renderable);

		commandList->SetComputeRootSignature(rs);
		commandList->SetPipelineState(ps);

		unsigned int backBufferIndex = renderer->backBufferIndex;
		commandList->SetComputeRootDescriptorTable(1, bonesCbv->gpu_xhandle[backBufferIndex]);

		for (unsigned int i = 0; i < verticesCpuHandles.size(); i++)
		{
			auto& cbv = constantsBuffers[i];
			commandList->SetComputeRootDescriptorTable(0, cbv->gpu_xhandle[0]);
			commandList->SetComputeRootDescriptorTable(2, resultGpuHandle[i]);
			commandList->SetComputeRootDescriptorTable(3, verticesGpuHandles[i]);
			commandList->Dispatch(1, 1, 1);
		}
	}

	void RenderableBoundingBox::SolveDXBoundingBox()
	{
		if (!renderable->animable) return;

		XMFLOAT4* mem{};
		D3D12_RANGE range{};
		range.Begin = 0;
		range.End = sizeof(XMFLOAT4) * 2ULL;

		auto& commandList = renderer->commandList;

		for (unsigned int i = 0; i < verticesCpuHandles.size(); i++)
		{
			DeviceUtils::TransitionResource(commandList, resources[i], D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
			DeviceUtils::TransitionResource(commandList, readBackResources[i], D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
			commandList->CopyResource(readBackResources[i], resources[i]);

			readBackResources[i]->Map(0, &range, reinterpret_cast<void**>(&mem));

			XMFLOAT3 center = { mem[0].x, mem[0].y , mem[0].z };
			XMFLOAT3 extents = { mem[1].x, mem[1].y , mem[1].z };

			BoundingBox gpuBBox = BoundingBox(center, extents);

			if (i == 0)
			{
				boundingBox = gpuBBox;
			}
			else
			{
				BoundingBox out;
				BoundingBox::CreateMerged(out, boundingBox, gpuBBox);
				boundingBox = out;
			}

			D3D12_RANGE emptyRange{ 0, 0 };
			readBackResources[i]->Unmap(0, &emptyRange);

			DeviceUtils::TransitionResource(commandList, resources[i], D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
			DeviceUtils::TransitionResource(commandList, readBackResources[i], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
		}
	}

	void RenderableBoundingBox::Destroy()
	{
		if (!renderable || !renderable->animable) {
			renderable = nullptr;
			return;
		}

		resources.clear();
		readBackResources.clear();
		constantsBuffers.clear();
		for (unsigned int i = 0; i < verticesCpuHandles.size(); i++)
		{
			DeviceUtils::FreeCSUDescriptor(verticesCpuHandles[i], verticesGpuHandles[i]);
		}
		for (unsigned int i = 0; i < resultCpuHandle.size(); i++)
		{
			DeviceUtils::FreeCSUDescriptor(resultCpuHandle[i], resultGpuHandle[i]);
		}
		DestroyShaderBinary(shader);
		renderable = nullptr;
	}
}
