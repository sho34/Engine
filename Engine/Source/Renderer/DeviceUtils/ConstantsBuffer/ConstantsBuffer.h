#pragma once
#include <map>
#include <atlbase.h>
#include "../../../d3dx12.h"
//#include "../../Renderer.h"
//#include "../../../Templates/Material/Material.h"

namespace DeviceUtils
{
	static constexpr unsigned int csuNumDescriptorsInFrame = 1000;

	void CreateCSUDescriptorHeap(unsigned int numFrames);
	void DestroyCSUDescriptorHeap();
	unsigned int GetCSUDescriptorSize();
	CComPtr<ID3D12DescriptorHeap>& GetCSUDescriptorHeap();
	void AllocCSUDescriptor(::CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, ::CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
	void FreeCSUDescriptor(::CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, ::CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);

	struct ConstantsBuffer {
		ConstantsBuffer(size_t size, std::string name) : alignedConstantBufferSize((size + 255) & ~255), name(name) { }
		~ConstantsBuffer() { Destroy(); }
		std::string name;
		unsigned int alignedConstantBufferSize;

		CComPtr<ID3D12Resource> constantBuffer;
		byte* mappedConstantBuffer = nullptr;
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> cpu_xhandle;
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> gpu_xhandle;

		template<typename T> size_t push(T& data, unsigned int index, size_t offset = 0ULL) {
			memcpy(mappedConstantBuffer + alignedConstantBufferSize * index + offset, &data, sizeof(T));
			return offset + sizeof(T);
		}

		void SetRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot, unsigned int backBufferIndex);
		void Destroy();
	};

	std::shared_ptr<ConstantsBuffer> CreateConstantsBuffer(size_t bufferSize, std::string cbName = "");
	void DestroyConstantsBuffer(std::shared_ptr<ConstantsBuffer>& cbuffer);
	void DestroyConstantsBuffer();
	::CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const std::shared_ptr<ConstantsBuffer>& cbvData, unsigned int index);
	::CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const std::shared_ptr<ConstantsBuffer>& cbvData, unsigned int index);
	//void WriteMaterialVariablesToConstantsBufferSpace(MaterialPtr& material, std::shared_ptr<ConstantsBufferViewData>& cbvData, UINT cbvFrameIndex);
};

typedef std::map<std::shared_ptr<DeviceUtils::ConstantsBuffer>, std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE>> ConstantsBufferCpuHandleMap;
typedef std::map<std::shared_ptr<DeviceUtils::ConstantsBuffer>, std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE>> ConstantsBufferGpuHandleMap;