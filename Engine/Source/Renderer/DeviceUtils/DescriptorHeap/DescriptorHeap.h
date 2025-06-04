#pragma once

#include <set>
#include <map>
#include <atlbase.h>
#include "../../DeviceUtils/D3D12Device/Builder.h"
#include <d3d12.h>
#include "../../../d3dx12.h"

using namespace Microsoft::WRL;

namespace DeviceUtils
{
	using namespace DirectX;

	struct DescriptorHeap {

		unsigned int size;
		unsigned int descriptorSize;
		CComPtr<ID3D12DescriptorHeap> descriptorHeap;

		std::set<unsigned int> usedCpuDescriptorHandle;
		std::map<unsigned int, CD3DX12_CPU_DESCRIPTOR_HANDLE> cpuDescriptorHandleMap;

		std::set<unsigned int> usedGpuDescriptorHandle;
		std::map<unsigned int, CD3DX12_GPU_DESCRIPTOR_HANDLE> gpuDescriptorHandleMap;

		void CreateDescriptorHeap(CComPtr<ID3D12Device2>& device, unsigned int sz, D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		void DestroyDescriptorHeap();

		unsigned int GetDescriptorSize();

		void AllocDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);

		void AllocCPUDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle);

		void AllocGPUDescriptor(CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);

		void FreeDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);

		void FreeCPUDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle);

		void FreeGPUDescriptor(CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
	};
};

