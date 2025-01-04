#pragma once
#include <wrl.h>
#include <memory>
#include <map>
#include "../../../d3dx12.h"
#include "../../Renderer.h"
#include "../../../Templates/Material/MaterialImpl.h"

namespace DeviceUtils::ConstantsBuffer
{
	static constexpr UINT	csuNumDescriptorsInFrame = 1000;

	void CreateCSUDescriptorHeap(CComPtr<ID3D12Device2>& d3dDevice, UINT numFrames);
	void DestroyCSUDescriptorHeap();
	UINT GetCSUDescriptorSize();
	CComPtr<ID3D12DescriptorHeap>&	GetCSUDescriptorHeap();
	void AllocCSUDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
	void FreeCSUDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);

	struct ConstantsBufferViewData {
		ConstantsBufferViewData(size_t size) : alignedConstantBufferSize((size + 255) & ~255) { }
		~ConstantsBufferViewData() { constantBuffer.Release(); constantBuffer = nullptr; }
		UINT alignedConstantBufferSize;

		CComPtr<ID3D12Resource> constantBuffer;
		UINT8* mappedConstantBuffer = nullptr;

		template<typename T> size_t push(T& data, UINT index, size_t offset = 0ULL) {
			memcpy(mappedConstantBuffer + alignedConstantBufferSize * index + offset, &data, sizeof(T));
			return offset + sizeof(T);
		}
	};

	std::shared_ptr<ConstantsBufferViewData> CreateConstantsBufferViewData(const std::shared_ptr<Renderer>& renderer, size_t bufferSize, std::string cbName="");
	void DestroyConstantsBufferViewData();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(std::shared_ptr<ConstantsBufferViewData> cbvData, UINT index);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(std::shared_ptr<ConstantsBufferViewData> cbvData, UINT index);
	void WriteMaterialVariablesToConstantsBufferSpace(MaterialPtr& material, std::shared_ptr<ConstantsBufferViewData>& cbvData, UINT cbvFrameIndex);
};

typedef DeviceUtils::ConstantsBuffer::ConstantsBufferViewData ConstantsBufferViewDataT;
typedef std::shared_ptr<ConstantsBufferViewDataT> ConstantsBufferViewDataPtr;
typedef std::map<ConstantsBufferViewDataPtr, std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE>> ConstantsBufferCpuHandleMap;
typedef std::map< ConstantsBufferViewDataPtr, std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE>> ConstantsBufferGpuHandleMap;