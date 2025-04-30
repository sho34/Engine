#include "pch.h"
#include "ConstantsBuffer.h"
#include "../D3D12Device/Builder.h"
#include "../DescriptorHeap/DescriptorHeap.h"
#include "../../Renderer.h"
#include "../../../Common/DirectXHelper.h"

extern std::shared_ptr<Renderer> renderer;

namespace DeviceUtils
{
	std::shared_ptr<DescriptorHeap> csuDescriptorHeap;
	std::vector<std::shared_ptr<ConstantsBuffer>> constantsBuffers;

	void ConstantsBuffer::SetRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot, unsigned int backBufferIndex)
	{
		commandList->SetGraphicsRootDescriptorTable(cbvSlot, gpu_xhandle[backBufferIndex]);
		cbvSlot++;
	}

	void ConstantsBuffer::Destroy()
	{
		constantBuffer = nullptr;
		for (unsigned int i = 0; i < cpu_xhandle.size(); i++)
		{
			FreeCSUDescriptor(cpu_xhandle[i], gpu_xhandle[i]);
		}
		mappedConstantBuffer = nullptr;
		cpu_xhandle.clear();
		gpu_xhandle.clear();
	}

	void CreateCSUDescriptorHeap(unsigned int numFrames) {
		csuDescriptorHeap = std::make_shared<DescriptorHeap>();
		csuDescriptorHeap->CreateDescriptorHeap(renderer->d3dDevice, numFrames * csuNumDescriptorsInFrame, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	}

	void DestroyCSUDescriptorHeap()
	{
		csuDescriptorHeap->DestroyDescriptorHeap();
	}

	unsigned int GetCSUDescriptorSize()
	{
		return csuDescriptorHeap->descriptorSize;
	}

	CComPtr<ID3D12DescriptorHeap>& GetCSUDescriptorHeap()
	{
		return csuDescriptorHeap->descriptorHeap;
	}

	void AllocCSUDescriptor(::CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, ::CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		csuDescriptorHeap->AllocDescriptor(cpuHandle, gpuHandle);
	}

	void FreeCSUDescriptor(::CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, ::CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		csuDescriptorHeap->FreeDescriptor(cpuHandle, gpuHandle);
	}

	//static std::mutex constantsBufferMutex;
	std::shared_ptr<ConstantsBuffer> CreateConstantsBuffer(size_t bufferSize, std::string cbName)
	{
		//std::lock_guard<std::mutex> lock(constantsBufferMutex);
		std::shared_ptr<ConstantsBuffer> cbvData = std::make_shared<ConstantsBuffer>(bufferSize, cbName);

		//create the d3d12 cbuffer
		CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(renderer->numFrames * cbvData->alignedConstantBufferSize);
		CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		DX::ThrowIfFailed(renderer->d3dDevice->CreateCommittedResource(
			&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
			&constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&cbvData->constantBuffer)
		));

		CCNAME_D3D12_OBJECT_N(cbvData->constantBuffer, cbName);
		LogCComPtrAddress(cbName, cbvData->constantBuffer);

		//create the views for the constants buffer and get the handles to the views
		D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = cbvData->constantBuffer->GetGPUVirtualAddress();
		for (UINT n = 0; n < renderer->numFrames; n++) {
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
			desc.BufferLocation = cbvGpuAddress;
			desc.SizeInBytes = cbvData->alignedConstantBufferSize;

			CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_xhandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_xhandle;
			AllocCSUDescriptor(cpu_xhandle, gpu_xhandle);

			renderer->d3dDevice->CreateConstantBufferView(&desc, cpu_xhandle);
			cbvGpuAddress += desc.SizeInBytes;

			cbvData->cpu_xhandle.push_back(cpu_xhandle);
			cbvData->gpu_xhandle.push_back(gpu_xhandle);
		}

		//map the CPU memory to the GPU and then mapped memory
		//mapea la memoria de la PCU con la GPU y luego vacia la memoria mapeada
		CD3DX12_RANGE readRange(0, 0);
		DX::ThrowIfFailed(cbvData->constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&cbvData->mappedConstantBuffer)));
		ZeroMemory(cbvData->mappedConstantBuffer, renderer->numFrames * cbvData->alignedConstantBufferSize);

		constantsBuffers.push_back(cbvData);

		return cbvData;
	}

	void DestroyConstantsBuffer(std::shared_ptr<ConstantsBuffer>& cbuffer)
	{
		DEBUG_PTR_COUNT(cbuffer);
		nostd::vector_erase(constantsBuffers, cbuffer);
		cbuffer = nullptr;
	}

	void DestroyConstantsBuffer()
	{
		constantsBuffers.clear();
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const std::shared_ptr<ConstantsBuffer>& cbvData, unsigned int index)
	{
		return cbvData->cpu_xhandle[index];
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const std::shared_ptr<ConstantsBuffer>& cbvData, unsigned int index)
	{
		return cbvData->gpu_xhandle[index];
	}
}