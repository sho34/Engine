#include "pch.h"
#include "DiffuseIrradianceMap.h"

#include "../../../Renderer/Renderer.h"
#include "../../../Renderer/DeviceUtils/Resources/Resources.h"
#include "../../../Common/DirectXHelper.h"
#include "../../../Templates/Textures/Texture.h"
#include <DirectXTex.h>

extern std::shared_ptr<Renderer> renderer;

using namespace DeviceUtils;

D3D12_STATIC_SAMPLER_DESC IBLDiffuseSampler = {
	.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
	.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	.MipLODBias = 0,
	.MaxAnisotropy = 0,
	.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
	.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
	.MinLOD = 0.0f,
	.MaxLOD = D3D12_FLOAT32_MAX,
	.ShaderRegister = 0,
	.RegisterSpace = 0,
	.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL,
};

namespace ComputeShader
{
	DiffuseIrradianceMap::DiffuseIrradianceMap(std::string envMapUUID, std::filesystem::path iblDiffuseFile) :
		ComputeInterface("IBLDiffuseIrradianceMap_cs", { IBLDiffuseSampler })
	{
		using namespace Templates;

		outputFile = iblDiffuseFile;

		//create the uav resource for the calculation results (U0)
		D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		resourceDesc = CD3DX12_RESOURCE_DESC::Tex3D(dataFormat, faceWidth, faceHeight, numFaces, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		renderer->d3dDevice->CreateCommittedResource(
			&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource)
		);
		CCNAME_D3D12_OBJECT_N(resource, std::string("DiffuseIrradianceMap"));
		LogCComPtrAddress("DiffuseIrradianceMap", resource);
		DeviceUtils::AllocCSUDescriptor(resultCpuHandle, resultGpuHandle);

		//create a uav desc/view for writing the diffuse ibl cube texture
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc =
		{
			.Format = dataFormat, .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D,
			.Texture3D = {.MipSlice = 0, .FirstWSlice = 0, .WSize = numFaces }
		};
		renderer->d3dDevice->CreateUnorderedAccessView(resource, nullptr, &uavDesc, resultCpuHandle);

		//create a srv desc/view for reading the envmap but as a cube texture
		envMap = std::make_shared<TextureInstance>();
		envMap->Load(envMapUUID);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDescRead = {
			.Format = envMap->viewDesc.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.TextureCube = {
				.MostDetailedMip = envMap->viewDesc.Texture2DArray.MostDetailedMip,
				.MipLevels = envMap->viewDesc.Texture2DArray.MipLevels,
				.ResourceMinLODClamp = envMap->viewDesc.Texture2DArray.ResourceMinLODClamp
			}
		};
		DeviceUtils::AllocCSUDescriptor(envMapCubeCpuHandle, envMapCubeGpuHandle);
		renderer->d3dDevice->CreateShaderResourceView(envMap->texture, &srvDescRead, envMapCubeCpuHandle);
	}

	DiffuseIrradianceMap::~DiffuseIrradianceMap()
	{
		DeviceUtils::FreeCSUDescriptor(envMapCubeCpuHandle, envMapCubeGpuHandle);
		DeviceUtils::FreeCSUDescriptor(resultCpuHandle, resultGpuHandle);
		resource = nullptr;
		readBackResource = nullptr;
	}

	void DiffuseIrradianceMap::Compute()
	{
		CComPtr<ID3D12GraphicsCommandList2>& commandList = renderer->commandList;

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, L"DiffuseIrradianceMap Compute");
#endif

		shader.SetComputeState();

		commandList->SetComputeRootDescriptorTable(0, resultGpuHandle);
		commandList->SetComputeRootDescriptorTable(1, envMapCubeGpuHandle);
		commandList->Dispatch(8, 8, 6);

#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
	}

	void DiffuseIrradianceMap::Solution()
	{
		DeviceUtils::CaptureTexture3D(
			renderer->d3dDevice,
			renderer->commandQueue,
			resource,
			faceWidth * pixelSize,
			resourceDesc,
			readBackResource,
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COMMON
		);

		XMFLOAT4* mem{};
		D3D12_RANGE range{};
		range.Begin = 0;
		range.End = dataSize;

		readBackResource->Map(0, &range, reinterpret_cast<void**>(&mem));

		WriteFile(mem);

		D3D12_RANGE emptyRange{ 0, 0 };
		readBackResource->Unmap(0, &emptyRange);
		readBackResource = nullptr;
	}

	void DiffuseIrradianceMap::WriteFile(XMFLOAT4* data) const
	{
		using namespace DirectX;

		std::vector<Image> imgs;
		for (unsigned int i = 0; i < numFaces; i++)
		{
			imgs.push_back(
				{
					.width = faceWidth,
					.height = faceHeight,
					.format = dataFormat,
					.rowPitch = static_cast<size_t>(faceWidth * pixelSize),
					.slicePitch = static_cast<size_t>(faceWidth * faceHeight * pixelSize),
					.pixels = reinterpret_cast<uint8_t*>(data) + i * faceWidth * faceHeight * pixelSize
				}
			);
		}

		TexMetadata meta =
		{
			.width = faceWidth,
			.height = faceHeight,
			.depth = 1,
			.arraySize = numFaces,
			.mipLevels = 1,
			.miscFlags = TEX_MISC_TEXTURECUBE,
			.miscFlags2 = 0,
			.format = dataFormat,
			.dimension = TEX_DIMENSION_TEXTURE2D
		};

		DX::ThrowIfFailed(SaveToDDSFile(imgs.data(), imgs.size(), meta, DDS_FLAGS_NONE, outputFile.wstring().c_str()));
	}
};
