#pragma once
#include "ComputeInterface.h"
#include "../../Renderer/DeviceUtils/RenderToTexture/RenderToTexture.h"

namespace ComputeShader
{
	using namespace DeviceUtils;

	struct IBLDiffuseIrradianceMap : public ComputeInterface
	{
		unsigned int faceWidth = 64U;
		unsigned int faceHeight = 64U;
		unsigned int numFaces = 6U;
		unsigned int pixelSize = 4U * sizeof(float);
		unsigned int faceSize = faceWidth * faceHeight * pixelSize;
		unsigned int dataSize = static_cast<unsigned int>(faceSize * numFaces);
		DXGI_FORMAT dataFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

		std::filesystem::path outputFile;

		std::shared_ptr<TextureInstance> envMap;
		::CD3DX12_CPU_DESCRIPTOR_HANDLE envMapCubeCpuHandle;	//SRV, (T0) 
		::CD3DX12_GPU_DESCRIPTOR_HANDLE envMapCubeGpuHandle; //SRV, (T0)		

		D3D12_RESOURCE_DESC resourceDesc;
		CComPtr<ID3D12Resource> resource;
		::CD3DX12_CPU_DESCRIPTOR_HANDLE resultCpuHandle;	//UAV, (U0) 
		::CD3DX12_GPU_DESCRIPTOR_HANDLE resultGpuHandle; //UAV, (U0)
		CComPtr<ID3D12Resource> readBackResource;

		IBLDiffuseIrradianceMap(std::string envMapUUID, std::filesystem::path iblDiffuseFile);
		~IBLDiffuseIrradianceMap();

		virtual void Compute();
		virtual void Solution();
		void WriteFile(XMFLOAT4* data);
	};
};

