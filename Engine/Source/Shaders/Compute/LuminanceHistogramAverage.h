#pragma once
#include "ComputeShader.h"
#include "ComputeInterface.h"
#include "../Renderer/DeviceUtils/RenderToTexture/RenderToTexture.h"

namespace ComputeShader
{
	using namespace DeviceUtils;

	struct LuminanceHistogramAverageBuffer
	{
		unsigned int  pixelCount;
		float minLogLuminance;
		float logLuminanceRange;
		float timeDelta;
		float tau;
	};

	struct LuminanceHistogramAverage : public ComputeInterface
	{
		//histogram
		::CD3DX12_CPU_DESCRIPTOR_HANDLE histogramCpuHandle;
		::CD3DX12_GPU_DESCRIPTOR_HANDLE histogramGpuHandle;

		std::shared_ptr<ConstantsBuffer> constantsBuffers; //LuminanceHistogramAverageBuffer CBV (C0)

		//luminance
		CComPtr<ID3D12Resource> average;
		::CD3DX12_CPU_DESCRIPTOR_HANDLE averageCpuHandle;
		::CD3DX12_GPU_DESCRIPTOR_HANDLE averageGpuHandle;
		::CD3DX12_CPU_DESCRIPTOR_HANDLE averageReadCpuHandle;
		::CD3DX12_GPU_DESCRIPTOR_HANDLE averageReadGpuHandle;

		LuminanceHistogramAverage(
			::CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle,
			::CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle
		);
		~LuminanceHistogramAverage();

		void UpdateLuminanceHistogramAverageParams(unsigned int  pixelCount, float minLogLuminance, float maxLogLuminance, float timeDelta, float tau) const;

		virtual void Compute();
		virtual void Solution() {};
	};
}