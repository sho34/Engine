#pragma once

#include <set>
#include <map>
#include <atlbase.h>
#include "../../DeviceUtils/D3D12Device/Builder.h"

using namespace Microsoft::WRL;

namespace DeviceUtils
{

	struct DescriptorHeap {

		unsigned int size;
		unsigned int descriptorSize;
		CComPtr<ID3D12DescriptorHeap> descriptorHeap;

		std::set<unsigned int> usedCpuDescriptorHandle;
		std::map<unsigned int, CD3DX12_CPU_DESCRIPTOR_HANDLE> cpuDescriptorHandleMap;

		std::set<unsigned int> usedGpuDescriptorHandle;
		std::map<unsigned int, CD3DX12_GPU_DESCRIPTOR_HANDLE> gpuDescriptorHandleMap;

		void CreateDescriptorHeap(CComPtr<ID3D12Device2>& device, unsigned int sz, D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
		{
			size = sz;
			descriptorHeap = DeviceUtils::CreateDescriptorHeap(device, size, heapType, flags);
			descriptorSize = device->GetDescriptorHandleIncrementSize(heapType);
			std::string heapName = "descriptorHeap-" + heapTypeToString.at(heapType);
			descriptorHeap->SetName(nostd::StringToWString(heapName).c_str());
		}

		void DestroyDescriptorHeap()
		{
			descriptorHeap.Release();
			descriptorHeap = nullptr;
		}

		unsigned int GetDescriptorSize()
		{
			return descriptorSize;
		}

		void AllocDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
		{
			AllocCPUDescriptor(cpuHandle);
			AllocGPUDescriptor(gpuHandle);
		}

		void AllocCPUDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
		{
			UINT slotIndex = 0U;
			for (; slotIndex < size; slotIndex++) {
				if (!usedCpuDescriptorHandle.contains(slotIndex)) break;
			}
			assert(slotIndex != size);

			usedCpuDescriptorHandle.insert(slotIndex);

			cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			cpuHandle.Offset(slotIndex, GetDescriptorSize());
		}

		void AllocGPUDescriptor(CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
		{
			UINT slotIndex = 0U;
			for (; slotIndex < size; slotIndex++) {
				if (!usedGpuDescriptorHandle.contains(slotIndex)) break;
			}
			assert(slotIndex != size);

			usedGpuDescriptorHandle.insert(slotIndex);

			gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart());
			gpuHandle.Offset(slotIndex, GetDescriptorSize());
		}

		void FreeDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
		{
			FreeCPUDescriptor(cpuHandle);
			FreeGPUDescriptor(gpuHandle);
		}

		void FreeCPUDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle)
		{
			if (!descriptorHeap) return;

			CD3DX12_CPU_DESCRIPTOR_HANDLE startHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

			UINT slotIndex = static_cast<UINT>(cpuHandle.ptr - startHandle.ptr) / GetDescriptorSize();
			usedCpuDescriptorHandle.erase(slotIndex);

			cpuHandle.ptr = 0;
		}

		void FreeGPUDescriptor(CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
		{
			if (!descriptorHeap) return;

			CD3DX12_GPU_DESCRIPTOR_HANDLE startHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart());

			UINT slotIndex = static_cast<UINT>(gpuHandle.ptr - startHandle.ptr) / GetDescriptorSize();
			usedGpuDescriptorHandle.erase(slotIndex);

			gpuHandle.ptr = 0;
		}
	};
};

