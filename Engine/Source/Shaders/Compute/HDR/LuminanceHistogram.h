#pragma once
#include "../ComputeShader.h"
#include "../ComputeInterface.h"
#include "../../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include "../../../Renderer/DeviceUtils/RenderToTexture/RenderToTexture.h"

namespace ComputeShader
{
	using namespace DeviceUtils;

	struct LuminanceHistogramBuffer
	{
		unsigned int inputWidth;
		unsigned int inputHeight;
		float minLogLuminance;
		float oneOverLogLuminanceRange;
	};

	struct LuminanceHistogram : public ComputeInterface
	{
		//histogram
		std::shared_ptr<RenderToTexture> rtt; // HDR BaseTexture, (T0)
		CComPtr<ID3D12Resource> resource; //LuminanceHistogram (U0)
		std::shared_ptr<ConstantsBuffer> constantsBuffers; //LuminanceHistogramBuffer CBV (C0)
		::CD3DX12_CPU_DESCRIPTOR_HANDLE resultCpuHandle;	//UAV, (U0) 
		::CD3DX12_GPU_DESCRIPTOR_HANDLE resultGpuHandle; //UAV, (U0)

		//UAV Clearing
		std::shared_ptr<DeviceUtils::DescriptorHeap> resultClearHeap; //UAV (U0)
		::CD3DX12_CPU_DESCRIPTOR_HANDLE resultClearCpuHandle; //UAV (U0)

		LuminanceHistogram(std::shared_ptr<RenderToTexture> renderToTexture);
		~LuminanceHistogram();

		void UpdateLuminanceHistogramParams(unsigned int width, unsigned int height, float minLogLuminance, float maxLogLuminance) const;

		virtual void Compute();
		virtual void Solution() {};
	};
}

