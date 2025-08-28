#pragma once
#include "../ComputeInterface.h"
#include <wrl.h>
#include <wrl/client.h>
#include <atlbase.h>
#include <d3dx12.h>

namespace DeviceUtils {
	struct ConstantsBuffer; struct RenderToTexture; struct DescriptorHeap;
};

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
		CD3DX12_CPU_DESCRIPTOR_HANDLE resultCpuHandle;	//UAV, (U0) 
		CD3DX12_GPU_DESCRIPTOR_HANDLE resultGpuHandle; //UAV, (U0)

		//UAV Clearing
		std::shared_ptr<DeviceUtils::DescriptorHeap> resultClearHeap; //UAV (U0)
		CD3DX12_CPU_DESCRIPTOR_HANDLE resultClearCpuHandle; //UAV (U0)

		LuminanceHistogram(std::shared_ptr<RenderToTexture> renderToTexture);
		~LuminanceHistogram();

		void UpdateLuminanceHistogramParams(unsigned int width, unsigned int height, float minLogLuminance, float maxLogLuminance) const;

		virtual void Compute();
		virtual void Solution() {};
	};
}

