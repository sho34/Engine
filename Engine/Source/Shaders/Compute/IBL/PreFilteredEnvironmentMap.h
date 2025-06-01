#pragma once
#include "../ComputeInterface.h"

namespace ComputeShader
{
	using namespace DeviceUtils;

	struct PreFilteredEnvironmentMap : public ComputeInterface
	{
		unsigned int faceWidth = 0U;
		unsigned int faceHeight = 0U;
		unsigned int numMipMaps = 0U;
		unsigned int numFaces = 6U;
		const unsigned int pixelSize = 4U * sizeof(float);
		const DXGI_FORMAT dataFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

		std::filesystem::path outputFile;

		std::shared_ptr<TextureInstance> envMap;
		::CD3DX12_CPU_DESCRIPTOR_HANDLE envMapCubeCpuHandle; //SRV, (T0)
		::CD3DX12_GPU_DESCRIPTOR_HANDLE envMapCubeGpuHandle; //SRV, (T0)

		std::vector<D3D12_RESOURCE_DESC> resourcesDesc;
		std::vector<CComPtr<ID3D12Resource>> resources;
		std::vector<::CD3DX12_CPU_DESCRIPTOR_HANDLE> mipsResultsCpuHandle; //UAV, (U0) 
		std::vector<::CD3DX12_GPU_DESCRIPTOR_HANDLE> mipsResultsGpuHandle; //UAV, (U0)
		std::vector<std::shared_ptr<ConstantsBuffer>> mipsResultsCB; //CBV, (C0)

		std::vector<size_t> readBackSizes;
		std::vector<CComPtr<ID3D12Resource>> readBackResources;

		PreFilteredEnvironmentMap(std::string envMapUUID, std::filesystem::path iblPreFilteredEnvironmentMapFile);
		~PreFilteredEnvironmentMap();

		virtual void Compute();
		virtual void Solution();
		void WriteFile(std::vector<Image>& imgs) const;
	};
};

