#include "pch.h"
#include "PreFilteredEnvironmentMap.h"

#include <Renderer.h>
#include <DeviceUtils/Resources/Resources.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <Textures/Texture.h>
#include <DirectXHelper.h>

extern std::shared_ptr<Renderer> renderer;

using namespace DeviceUtils;

D3D12_STATIC_SAMPLER_DESC IBLPreFilteredEnvironmentSampler = {
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
	PreFilteredEnvironmentMap::PreFilteredEnvironmentMap(std::string envMapUUID, std::filesystem::path iblPreFilteredEnvironmentMapFile) :
		ComputeInterface("IBLPrefilteredEnvironmentMap_cs", { IBLPreFilteredEnvironmentSampler })
	{
		//using namespace Templates;
		//
		//outputFile = iblPreFilteredEnvironmentMapFile;
		//
		////create a srv desc/view for reading the envmap but as a cube texture
		//envMap = std::make_shared<TextureInstance>();
		//nlohmann::json& json = GetTextureTemplate(envMapUUID);
		//envMap->Load(envMapUUID);
		//D3D12_SHADER_RESOURCE_VIEW_DESC srvDescRead = {
		//	.Format = envMap->viewDesc.Format, .ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE,
		//	.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		//	.TextureCube = {
		//		.MostDetailedMip = envMap->viewDesc.Texture2DArray.MostDetailedMip,
		//		.MipLevels = envMap->viewDesc.Texture2DArray.MipLevels,
		//		.ResourceMinLODClamp = envMap->viewDesc.Texture2DArray.ResourceMinLODClamp
		//	}
		//};
		//DeviceUtils::AllocCSUDescriptor(envMapCubeCpuHandle, envMapCubeGpuHandle);
		//renderer->d3dDevice->CreateShaderResourceView(envMap->texture, &srvDescRead, envMapCubeCpuHandle);
		//
		//faceWidth = json.at("width");
		//faceHeight = json.at("height");
		//numMipMaps = json.at("mipLevels");
		//unsigned int faceW = faceWidth;
		//unsigned int faceH = faceHeight;
		//
		//faceW = faceWidth;
		//faceH = faceHeight;
		//for (unsigned int i = 0; i < numMipMaps; i++)
		//{
		//	//create the uav resource for the calculation results (U0)
		//	D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		//	resourcesDesc.push_back(CD3DX12_RESOURCE_DESC::Tex3D(dataFormat, faceW, faceH, numFaces, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));
		//	resources.push_back(CComPtr<ID3D12Resource>());
		//	renderer->d3dDevice->CreateCommittedResource(
		//		&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resourcesDesc.back(), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resources.back())
		//	);
		//	CCNAME_D3D12_OBJECT_N(resources.back(), std::string("PrefilteredEnvironmentMap"));
		//	LogCComPtrAddress("PrefilteredEnvironmentMap", resources.back());
		//
		//	//create handlers, descs, views and cbvs for each mipmap
		//	mipsResultsCpuHandle.push_back(::CD3DX12_CPU_DESCRIPTOR_HANDLE());
		//	mipsResultsGpuHandle.push_back(::CD3DX12_GPU_DESCRIPTOR_HANDLE());
		//	auto& resultCpuHandle = mipsResultsCpuHandle.back();
		//	auto& resultGpuHandle = mipsResultsGpuHandle.back();
		//	DeviceUtils::AllocCSUDescriptor(resultCpuHandle, resultGpuHandle);
		//
		//	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc =
		//	{
		//		.Format = dataFormat, .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D,
		//		.Texture3D = {.MipSlice = 0, .FirstWSlice = 0, .WSize = numFaces }
		//	};
		//	renderer->d3dDevice->CreateUnorderedAccessView(resources.back(), nullptr, &uavDesc, resultCpuHandle);
		//
		//	std::shared_ptr<ConstantsBuffer> cbv = CreateConstantsBuffer(sizeof(XMFLOAT4), std::string("PrefilteredEnvironmentMap:CBV[" + std::to_string(i + 1) + "]"));
		//	mipsResultsCB.push_back(cbv);
		//
		//	//Create a ReadBack
		//	readBackSizes.push_back(static_cast<size_t>(faceW) * static_cast<size_t>(faceH) * pixelSize * static_cast<size_t>(numFaces));
		//
		//	faceW = max(faceW >> 1U, 1U);
		//	faceH = max(faceH >> 1U, 1U);
		//}
	}

	PreFilteredEnvironmentMap::~PreFilteredEnvironmentMap()
	{
		//DeviceUtils::FreeCSUDescriptor(envMapCubeCpuHandle, envMapCubeGpuHandle);
		//for (unsigned int i = 0; i < mipsResultsCpuHandle.size(); i++)
		//{
		//	auto& resultCpuHandle = mipsResultsCpuHandle.at(i);
		//	auto& resultGpuHandle = mipsResultsGpuHandle.at(i);
		//	auto& resultCB = mipsResultsCB.at(i);
		//	DeviceUtils::FreeCSUDescriptor(resultCpuHandle, resultGpuHandle);
		//	DeviceUtils::DestroyConstantsBuffer(resultCB);
		//}
		//mipsResultsCpuHandle.clear();
		//mipsResultsGpuHandle.clear();
		//mipsResultsCB.clear();
		//resources.clear();
		//readBackResources.clear();
	}

	void PreFilteredEnvironmentMap::Compute()
	{
		//		CComPtr<ID3D12GraphicsCommandList2>& commandList = renderer->commandList;
		//
		//#if defined(_DEVELOPMENT)
		//		PIXBeginEvent(commandList.p, 0, L"PreFilteredEnvironmentMap Compute");
		//#endif
		//
		//		shader.SetComputeState();
		//
		//		unsigned int faceW = faceWidth;
		//		unsigned int faceH = faceHeight;
		//		for (unsigned int i = 0; i < mipsResultsGpuHandle.size(); i++)
		//		{
		//			unsigned int threadsX = max(faceW / 8, 1U);
		//			unsigned int threadsY = max(faceH / 8, 1U);
		//			float roughness = static_cast<float>(i) / static_cast<float>(numMipMaps);
		//			XMFLOAT4 cb0Params = { roughness, static_cast<float>(i), static_cast<float>(faceWidth), static_cast<float>(faceW) };
		//			mipsResultsCB.at(i)->push<XMFLOAT4>(cb0Params, 0);
		//			commandList->SetComputeRootDescriptorTable(0, mipsResultsCB.at(i)->gpu_xhandle.at(0));
		//			commandList->SetComputeRootDescriptorTable(1, mipsResultsGpuHandle.at(i));
		//			commandList->SetComputeRootDescriptorTable(2, envMapCubeGpuHandle);
		//			commandList->Dispatch(threadsX, threadsY, numFaces);
		//			faceW = max(faceW >> 1, 1U);
		//			faceH = max(faceH >> 1, 1U);
		//		}
		//
		//#if defined(_DEVELOPMENT)
		//		PIXEndEvent(commandList.p);
		//#endif
	}

	void PreFilteredEnvironmentMap::Solution()
	{
		//readBackResources.clear();
		//
		//std::vector<Image> imgs;
		//nostd::VecN_push_back(numMipMaps * numFaces, imgs);
		//
		//unsigned int faceW = faceWidth;
		//unsigned int faceH = faceHeight;
		//std::vector<XMFLOAT4*> mipMems;
		//for (unsigned int i = 0; i < readBackSizes.size(); i++)
		//{
		//	readBackResources.push_back(CComPtr<ID3D12Resource>());
		//	auto& readBackResource = readBackResources.back();
		//	auto& resource = resources[i];
		//
		//	DeviceUtils::CaptureTexture3D(
		//		renderer->d3dDevice,
		//		renderer->commandQueue,
		//		resource,
		//		faceW * pixelSize,
		//		resourcesDesc.at(i),
		//		readBackResource,
		//		D3D12_RESOURCE_STATE_COMMON,
		//		D3D12_RESOURCE_STATE_COMMON
		//	);
		//
		//	D3D12_RANGE range{};
		//	range.Begin = 0;
		//	range.End = readBackSizes[i];
		//
		//	XMFLOAT4* mem{};
		//	readBackResource->Map(0, &range, reinterpret_cast<void**>(&mem));
		//	mipMems.push_back(mem);
		//
		//	for (unsigned int face = 0; face < numFaces; face++)
		//	{
		//		unsigned int imgIndex = i + face * numMipMaps;
		//		auto& img = imgs[imgIndex];
		//		img = {
		//			.width = faceW, .height = faceH, .format = dataFormat,
		//			.rowPitch = static_cast<size_t>(faceW * pixelSize),
		//			.slicePitch = static_cast<size_t>(faceW * faceH * pixelSize),
		//			.pixels = reinterpret_cast<uint8_t*>(mem) + static_cast<size_t>(face * faceW * faceH * pixelSize)
		//		};
		//	}
		//
		//	faceW = max(faceW >> 1, 1U);
		//	faceH = max(faceH >> 1, 1U);
		//}
		//
		//WriteFile(imgs);
		//
		//for (auto& readBackResource : readBackResources)
		//{
		//	D3D12_RANGE emptyRange{ 0, 0 };
		//	readBackResource->Unmap(0, &emptyRange);
		//}
		//readBackResources.clear();
	}

	void PreFilteredEnvironmentMap::WriteFile(std::vector<Image>& imgs) const
	{
		//using namespace DirectX;
		//
		//TexMetadata meta =
		//{
		//	.width = faceWidth,
		//	.height = faceHeight,
		//	.depth = 1,
		//	.arraySize = numFaces,
		//	.mipLevels = numMipMaps,
		//	.miscFlags = TEX_MISC_TEXTURECUBE,
		//	.miscFlags2 = 0,
		//	.format = dataFormat,
		//	.dimension = TEX_DIMENSION_TEXTURE2D
		//};
		//
		//DX::ThrowIfFailed(SaveToDDSFile(imgs.data(), imgs.size(), meta, DDS_FLAGS_NONE, outputFile.wstring().c_str()));
	}
}
