#include "pch.h"
#include "RenderableBoundingBox.h"
#include "../../Scene/Renderable/Renderable.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/DeviceUtils/Resources/Resources.h"

extern std::shared_ptr<Renderer> renderer;

namespace ComputeShader
{
	RenderableBoundingBox::RenderableBoundingBox(std::shared_ptr<Renderable> r) : ComputeInterface("BoundingBox_cs")
	{
		using namespace Animation;
		bonesCbv = GetAnimatedConstantsBuffer(r);

		auto createComputeResource = [this](size_t numResources)
			{
				size_t dataSize = sizeof(XMFLOAT4) * 2ULL;
				D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
					.Format = DXGI_FORMAT_UNKNOWN, .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
					.Buffer = {
						.FirstElement = 0ULL, .NumElements = 1U, .StructureByteStride = sizeof(XMFLOAT4) * 2,
						.CounterOffsetInBytes = 0ULL, .Flags = D3D12_BUFFER_UAV_FLAG_NONE
					}
				};

				D3D12_HEAP_PROPERTIES readBackHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
				D3D12_RESOURCE_DESC bufferDescReadBack = CD3DX12_RESOURCE_DESC::Buffer(dataSize);

				for (unsigned int i = 0; i < numResources; i++)
				{
					// Create UAV for the result
					resources.push_back(CComPtr<ID3D12Resource>());
					auto& resource = resources.back();
					renderer->d3dDevice->CreateCommittedResource(
						&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource)
					);

					resultCpuHandle.push_back(::CD3DX12_CPU_DESCRIPTOR_HANDLE());
					resultGpuHandle.push_back(::CD3DX12_GPU_DESCRIPTOR_HANDLE());
					::CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle = resultCpuHandle.back();
					::CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle = resultGpuHandle.back();
					DeviceUtils::AllocCSUDescriptor(cpuHandle, gpuHandle);
					renderer->d3dDevice->CreateUnorderedAccessView(resource, nullptr, &uavDesc, cpuHandle);

					//Create ReadBack
					readBackResources.push_back(CComPtr<ID3D12Resource>());
					auto& readBackResource = readBackResources.back();
					renderer->d3dDevice->CreateCommittedResource(
						&readBackHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDescReadBack,
						D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&readBackResource)
					);
				}
			};

		auto createMeshVerticesShaderResourceView = [this](std::shared_ptr<MeshInstance>& mesh)
			{
				verticesCpuHandles.push_back(CD3DX12_CPU_DESCRIPTOR_HANDLE());
				verticesGpuHandles.push_back(CD3DX12_GPU_DESCRIPTOR_HANDLE());
				mesh->CreateVerticesShaderResourceView(verticesCpuHandles.back(), verticesGpuHandles.back());
			};

		auto createNumVerticesConstantsBuffer = [this](std::shared_ptr<MeshInstance>& mesh)
			{
				auto cbv = CreateConstantsBuffer(shader.shader->cbufferSize[0], "bboxCS." + mesh->uuid);
				unsigned int numVertices = mesh->vbvData.vertexBufferView.SizeInBytes / mesh->vbvData.vertexBufferView.StrideInBytes;
				cbv->push<unsigned int>(numVertices, 0);
				constantsBuffers.push_back(cbv);
			};

		//Create the bounding box compute resource
		createComputeResource(r->meshes.size());

		bool extend = false;
		for (auto& mesh : r->meshes)
		{
			//Create the SRV for the vertex buffers of each mesh
			createMeshVerticesShaderResourceView(mesh);

			createNumVerticesConstantsBuffer(mesh);

			mesh->ExtendBoundingBox(boundingBox, extend);
			extend = true;
		}
	}

	RenderableBoundingBox::~RenderableBoundingBox()
	{
		bonesCbv = nullptr;
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
	}

	void RenderableBoundingBox::Compute()
	{
		CComPtr<ID3D12GraphicsCommandList2>& commandList = renderer->commandList;

		shader.SetComputeState();

		//0 : the number of vertices
		//1 : UAV for the bones transformation <- as all the meshes shares the same matrices we can just set once
		//2 : bounding box center and extents
		//3 : UAV for the vertices
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

	void RenderableBoundingBox::Solution()
	{
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
}
