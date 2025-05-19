#include "pch.h"
#include "LuminanceHistogramAverage.h"

#include "../../Renderer/Renderer.h"
#include "../../Renderer/DeviceUtils/Resources/Resources.h"

extern std::shared_ptr<Renderer> renderer;

using namespace DeviceUtils;

namespace ComputeShader
{
	LuminanceHistogramAverage::LuminanceHistogramAverage(
		::CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		::CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle
	) : ComputeInterface("LuminanceHistogramAverage_cs")
	{
		//cpu/gpu handles of the previously computed histogram, this is an UAV of 256 uints
		histogramCpuHandle = cpuHandle;
		histogramGpuHandle = gpuHandle;

		//create the luminicance histogram buffer containing the calculation parameters (C0)
		constantsBuffers = CreateConstantsBuffer(sizeof(LuminanceHistogramAverageBuffer), "LuminanceHistogramAverageBuffer");

		//create the uav resource for the calculation results, this is table of 256 unsigned ints (U0)
		unsigned int dataSize = static_cast<unsigned int>(sizeof(float));
		D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
			.Format = DXGI_FORMAT_UNKNOWN, .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
			.Buffer = {
				.FirstElement = 0ULL, .NumElements = 1U, .StructureByteStride = sizeof(float),
				.CounterOffsetInBytes = 0ULL, .Flags = D3D12_BUFFER_UAV_FLAG_NONE
			}
		};

		renderer->d3dDevice->CreateCommittedResource(
			&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&average)
		);

		DeviceUtils::AllocCSUDescriptor(averageCpuHandle, averageGpuHandle);
		renderer->d3dDevice->CreateUnorderedAccessView(average, nullptr, &uavDesc, averageCpuHandle);

		//this is the rtt of a single pixel that represents the scene luminance calculated by the CS shader(this is the output)
		/*
		rtt = std::make_shared<RenderToTexture>();
		rtt->name = "";
		rtt->format = DXGI_FORMAT_R32_FLOAT;
		rtt->width = 1U;
		rtt->height = 1U;
		rtt->Create();
		AllocCSUDescriptor(rtt->cpuTextureHandle, rtt->gpuTextureHandle);

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {
			.Format = DXGI_FORMAT_R32_FLOAT, .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
			.Buffer = {
				.FirstElement = 0ULL, .NumElements = 1, .StructureByteStride = 4,
				.CounterOffsetInBytes = 0ULL, .Flags = D3D12_BUFFER_UAV_FLAG_NONE
			}
		};
		renderer->d3dDevice->CreateUnorderedAccessView(rtt->renderToTexture, nullptr, &uavDesc, rtt->cpuTextureHandle);
		*/
		/*
		D3D12_SHADER_RESOURCE_VIEW_DESC rttSRVDesc = {
			.Format = DXGI_FORMAT_R32_FLOAT,
			.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
		};

		renderer->d3dDevice->CreateShaderResourceView(rtt->renderToTexture, &rttSRVDesc, rtt->cpuTextureHandle);
		*/
	}

	LuminanceHistogramAverage::~LuminanceHistogramAverage()
	{
		DestroyConstantsBuffer(constantsBuffers);
		FreeCSUDescriptor(averageCpuHandle, averageGpuHandle);
		average = nullptr;
		//FreeCSUDescriptor(rtt->cpuTextureHandle, rtt->gpuTextureHandle);
		//rtt->renderToTexture = nullptr;
		//rtt->Destroy();
		//rtt = nullptr;
	}

	void LuminanceHistogramAverage::UpdateLuminanceHistogramAverageParams(unsigned int pixelCount, float minLogLuminance, float maxLogLuminance, float timeDelta, float tau) const
	{
		LuminanceHistogramAverageBuffer params
		{
			.pixelCount = pixelCount,
			.minLogLuminance = minLogLuminance,
			.logLuminanceRange = (maxLogLuminance - minLogLuminance),
			.timeDelta = timeDelta,
			.tau = tau
		};
		constantsBuffers->push(params, 0);
	}

	void LuminanceHistogramAverage::Compute()
	{
		CComPtr<ID3D12GraphicsCommandList2>& commandList = renderer->commandList;

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, L"LuminanceHistogramAverage Compute");
#endif

		//after clearing the uav we can compute
		shader.SetComputeState();

		commandList->SetComputeRootDescriptorTable(0, constantsBuffers->gpu_xhandle[0]);
		commandList->SetComputeRootDescriptorTable(1, histogramGpuHandle);
		commandList->SetComputeRootDescriptorTable(2, averageGpuHandle);
		commandList->Dispatch(1, 1, 1);

#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
	}
}