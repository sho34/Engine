#include "pch.h"
#include "ConstantsBuffer.h"
#include "../D3D12Device/Builder.h"
#include "../../../Common/DirectXHelper.h"

namespace DeviceUtils::ConstantsBuffer
{
	static ConstantsBufferCpuHandleMap constantsBufferCpuMap;
	static ConstantsBufferGpuHandleMap constantsBufferGpuMap;

	static unsigned int csuDescriptorSize;
	static CComPtr<ID3D12DescriptorHeap> csuDescriptorHeap; //CBV_SRV_UAV

	static std::set<UINT> csuUsedDescriptorHandle;
	static std::map<UINT,CD3DX12_CPU_DESCRIPTOR_HANDLE> csuCpuDescriptorHandleMap;
	static std::map<UINT,CD3DX12_GPU_DESCRIPTOR_HANDLE> csuGpuDescriptorHandleMap;

	void CreateCSUDescriptorHeap(CComPtr<ID3D12Device2>& d3dDevice, UINT numFrames) {
		csuDescriptorHeap = D3D12Device::CreateDescriptorHeap(d3dDevice, numFrames * csuNumDescriptorsInFrame, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		csuDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CCNAME_D3D12_OBJECT(csuDescriptorHeap);
	}

	void DestroyCSUDescriptorHeap()
	{
		csuDescriptorHeap.Release();
		csuDescriptorHeap = nullptr;
	}

	UINT GetCSUDescriptorSize()
	{
		return csuDescriptorSize;
	}

	CComPtr<ID3D12DescriptorHeap>& GetCSUDescriptorHeap() 
	{
		return csuDescriptorHeap;
	}

	void AllocCSUDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		UINT slotIndex = 0U;
		for (; slotIndex < csuNumDescriptorsInFrame; slotIndex++) {
			if (!csuUsedDescriptorHandle.contains(slotIndex)) break;
		}
		assert(slotIndex != csuNumDescriptorsInFrame);

		csuUsedDescriptorHandle.insert(slotIndex);

		cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(csuDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(csuDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(slotIndex,GetCSUDescriptorSize());
		gpuHandle.Offset(slotIndex,GetCSUDescriptorSize());
	}

	void FreeCSUDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle) {

		CD3DX12_CPU_DESCRIPTOR_HANDLE startHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(csuDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		
		UINT slotIndex = static_cast<UINT>(cpuHandle.ptr - startHandle.ptr) / GetCSUDescriptorSize();
		csuUsedDescriptorHandle.erase(slotIndex);

		cpuHandle.ptr = 0;
		gpuHandle.ptr = 0;
	}

	static std::mutex constantsBufferMutex;
	std::shared_ptr<ConstantsBufferViewData> CreateConstantsBufferViewData(const std::shared_ptr<Renderer>& renderer, size_t bufferSize, std::string cbName)
	{
		std::lock_guard<std::mutex> lock(constantsBufferMutex);
		ConstantsBufferViewDataPtr cbvData = std::make_shared<ConstantsBufferViewData>(bufferSize);

		//create the d3d12 cbuffer
		CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(renderer->numFrames * cbvData->alignedConstantBufferSize);
		CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		DX::ThrowIfFailed(renderer->d3dDevice->CreateCommittedResource(
			&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, 
			&constantBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&cbvData->constantBuffer)
		));

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

			constantsBufferCpuMap[cbvData].push_back(cpu_xhandle);
			constantsBufferGpuMap[cbvData].push_back(gpu_xhandle);
#if defined(_DEBUG)
			if (n == 0) {
				std::wstring cbufferName = (L"ConstantsBuffer[" + StringToWString(cbName) + L":" + std::to_wstring(cpu_xhandle.ptr) + L"]");
				cbvData->constantBuffer->SetName(cbufferName.c_str());
			}
#endif
		}

		//map the CPU memory to the GPU and then mapped memory
		//mapea la memoria de la PCU con la GPU y luego vacia la memoria mapeada
		CD3DX12_RANGE readRange(0, 0);
		DX::ThrowIfFailed(cbvData->constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&cbvData->mappedConstantBuffer)));
		ZeroMemory(cbvData->mappedConstantBuffer, renderer->numFrames * cbvData->alignedConstantBufferSize);

		return cbvData;
	}

	void DestroyConstantsBufferViewData()
	{
		for (auto& [cbvData,cpuHandlers] : constantsBufferCpuMap)
		{
			for (int i = 0; i < constantsBufferCpuMap[cbvData].size(); i++) {
				FreeCSUDescriptor(constantsBufferCpuMap[cbvData][i], constantsBufferGpuMap[cbvData][i]);
			}
			cbvData->constantBuffer.Release();
			cbvData->constantBuffer = nullptr;
		}
		constantsBufferCpuMap.clear();
		constantsBufferGpuMap.clear();
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(std::shared_ptr<ConstantsBufferViewData> cbvData, UINT index)
	{
		return constantsBufferCpuMap[cbvData][index];
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(std::shared_ptr<ConstantsBufferViewData> cbvData, UINT index)
	{
		return constantsBufferGpuMap[cbvData][index];
	}

	void WriteMaterialVariablesToConstantsBufferSpace(MaterialPtr& material, std::shared_ptr<ConstantsBufferViewData>& cbvData, UINT cbvFrameIndex)
	{
		for (auto& [varName, varMapping] : material->variablesMapping) {

			UINT8* source = *material->variablesBuffer[varMapping.mapping.bufferIndex] + varMapping.mapping.offset;
			UINT8* destination = cbvData->mappedConstantBuffer + (cbvFrameIndex * cbvData->alignedConstantBufferSize) + varMapping.mapping.offset;
			memcpy(destination,  source, varMapping.mapping.size);
			int i = 0;
		}
	}

}