#pragma once
#include "../ComputeInterface.h"
#include "../../../Renderer/DeviceUtils/RenderToTexture/RenderToTexture.h"

namespace ComputeShader
{
	using namespace DeviceUtils;

	struct DiffuseIrradianceMap : public ComputeInterface
	{
		const unsigned int faceWidth = 64U;
		const unsigned int faceHeight = 64U;
		const unsigned int numFaces = 6U;
		const unsigned int pixelSize = 4U * sizeof(float);
		const unsigned int faceSize = faceWidth * faceHeight * pixelSize;
		const unsigned int dataSize = static_cast<unsigned int>(faceSize * numFaces);
		const DXGI_FORMAT dataFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

		std::filesystem::path outputFile;

		std::shared_ptr<TextureInstance> envMap;
		::CD3DX12_CPU_DESCRIPTOR_HANDLE envMapCubeCpuHandle; //SRV, (T0)
		::CD3DX12_GPU_DESCRIPTOR_HANDLE envMapCubeGpuHandle; //SRV, (T0)

		D3D12_RESOURCE_DESC resourceDesc;
		CComPtr<ID3D12Resource> resource;
		::CD3DX12_CPU_DESCRIPTOR_HANDLE resultCpuHandle; //UAV, (U0) 
		::CD3DX12_GPU_DESCRIPTOR_HANDLE resultGpuHandle; //UAV, (U0)

		CComPtr<ID3D12Resource> readBackResource;

		DiffuseIrradianceMap(std::string envMapUUID, std::filesystem::path iblDiffuseFile);
		~DiffuseIrradianceMap();

		virtual void Compute();
		virtual void Solution();
		void WriteFile(XMFLOAT4* data) const;
	};
};

