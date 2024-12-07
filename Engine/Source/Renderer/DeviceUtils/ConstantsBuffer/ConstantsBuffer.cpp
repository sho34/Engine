#include "pch.h"
#include "ConstantsBuffer.h"
#include "../D3D12Device/Builder.h"
#include "../../../Common/DirectXHelper.h"

namespace DeviceUtils::ConstantsBuffer
{
	static ConstantsBufferCpuHandleMap constantsBufferCpuMap;
	static ConstantsBufferGpuHandleMap constantsBufferGpuMap;

	static UINT csuDescriptorSize;
	static ComPtr<ID3D12DescriptorHeap> csuDescriptorHeap; //CBV_SRV_UAV
	static CD3DX12_CPU_DESCRIPTOR_HANDLE csuCpuDescriptorHandleStart;
	static CD3DX12_CPU_DESCRIPTOR_HANDLE csuCpuDescriptorHandleCurrent;
	static CD3DX12_GPU_DESCRIPTOR_HANDLE csuGpuDescriptorHandleStart;
	static CD3DX12_GPU_DESCRIPTOR_HANDLE csuGpuDescriptorHandleCurrent;

	void CreateCSUDescriptorHeap(ComPtr<ID3D12Device2>& d3dDevice, UINT numFrames) {
		csuDescriptorHeap = D3D12Device::CreateDescriptorHeap(d3dDevice, numFrames * csuNumDescriptorsInFrame, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		csuDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		csuCpuDescriptorHandleStart = csuCpuDescriptorHandleCurrent = CD3DX12_CPU_DESCRIPTOR_HANDLE(csuDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		csuGpuDescriptorHandleStart = csuGpuDescriptorHandleCurrent = CD3DX12_GPU_DESCRIPTOR_HANDLE(csuDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		NAME_D3D12_OBJECT(csuDescriptorHeap);
	}

	UINT GetCSUDescriptorSize()
	{
		return csuDescriptorSize;
	}

	ComPtr<ID3D12DescriptorHeap>& GetCSUDescriptorHeap() 
	{
		return csuDescriptorHeap;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE&	GetCpuDescriptorHandleStart()
	{
		return csuCpuDescriptorHandleStart;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE&	GetCpuDescriptorHandleCurrent()
	{
		return csuCpuDescriptorHandleCurrent;
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE&	GetGpuDescriptorHandleStart()
	{
		return csuGpuDescriptorHandleStart;
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE&	GetGpuDescriptorHandleCurrent()
	{
		return csuGpuDescriptorHandleCurrent;
	}

	static std::mutex constantsBufferMutex;
	std::shared_ptr<ConstantsBufferViewData> CreateConstantsBufferViewData(const std::shared_ptr<Renderer>& renderer, size_t bufferSize, std::wstring cbName)
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
#if defined(_DEBUG)
		std::wstring cbufferName = (L"ConstantsBuffer[" + cbName + L":" + std::to_wstring(GetCpuDescriptorHandleCurrent().ptr) + L"]");
		cbvData->constantBuffer->SetName(cbufferName.c_str());
#endif

		//create the views for the constants buffer and get the handles to the views
		D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = cbvData->constantBuffer->GetGPUVirtualAddress();
		for (UINT n = 0; n < renderer->numFrames; n++) {
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
			desc.BufferLocation = cbvGpuAddress;
			desc.SizeInBytes = cbvData->alignedConstantBufferSize;
			renderer->d3dDevice->CreateConstantBufferView(&desc, GetCpuDescriptorHandleCurrent());
			constantsBufferCpuMap[cbvData].push_back(CD3DX12_CPU_DESCRIPTOR_HANDLE(GetCpuDescriptorHandleCurrent()));
			constantsBufferGpuMap[cbvData].push_back(CD3DX12_GPU_DESCRIPTOR_HANDLE(GetGpuDescriptorHandleCurrent()));
			cbvGpuAddress += desc.SizeInBytes;
			GetCpuDescriptorHandleCurrent().Offset(GetCSUDescriptorSize());
			GetGpuDescriptorHandleCurrent().Offset(GetCSUDescriptorSize());
		}

		//map the CPU memory to the GPU and then mapped memory
		//mapea la memoria de la PCU con la GPU y luego vacia la memoria mapeada
		CD3DX12_RANGE readRange(0, 0);
		DX::ThrowIfFailed(cbvData->constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&cbvData->mappedConstantBuffer)));
		ZeroMemory(cbvData->mappedConstantBuffer, renderer->numFrames * cbvData->alignedConstantBufferSize);

		return cbvData;
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