#include "pch.h"
#include "MaterialTexture.h"
#include "../Textures/Texture.h"
#include "../../Common/DirectXHelper.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/DeviceUtils/Resources/Resources.h"
#include "../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include <wrl.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <atlbase.h>
#include <nlohmann/json.hpp>
#include <NoStd.h>
#include <ShaderMaterials.h>
#include <DDSTextureLoader.h>
#include "../../Common/d3dx12.h"

extern std::shared_ptr<Renderer> renderer;

//std::unordered_map<std::string, CComPtr<ID3D12Resource>> texturesCache;
//std::unordered_map<std::string, unsigned int> texturesCacheMipMaps;
//std::unordered_map<std::string, unsigned int> texturesCacheRefCount;

namespace Textures
{
	static nostd::RefTracker<std::string, std::shared_ptr<MaterialTextureInstance>> refTracker;
};

void TransformJsonToMaterialTextures(std::map<TextureType, std::string>& textures, nlohmann::json object, const std::string& key) {

	if (!object.contains(key)) return;

	nlohmann::json jtextures = object[key];

	for (nlohmann::json::iterator it = jtextures.begin(); it != jtextures.end(); it++)
	{
		textures.insert_or_assign(strToTextureType.at(it.key()), it.value());
	}
}

std::map<TextureType, std::shared_ptr<MaterialTextureInstance>> GetTextures(const std::map<TextureType, std::string>& textures)
{
	std::map<TextureType, std::shared_ptr<MaterialTextureInstance>> texInstances;

	std::transform(textures.begin(), textures.end(), std::inserter(texInstances, texInstances.end()), [](auto pair)
		{
			std::string& texture = pair.second;
			using namespace Textures;
			std::shared_ptr<MaterialTextureInstance> instance = refTracker.AddRef(texture, [&texture]()
				{
					using namespace Templates;
					std::shared_ptr<MaterialTextureInstance> instance = std::make_shared<MaterialTextureInstance>();
					nlohmann::json& json = GetTextureTemplate(texture);
					instance->Load(texture, stringToDxgiFormat.at(json.at("format")), json.at("numFrames"), json.at("mipLevels"));
					return instance;
				}
			);
			return std::pair<TextureType, std::shared_ptr<MaterialTextureInstance>>(pair.first, instance);
		}
	);

	return texInstances;
}

std::shared_ptr<MaterialTextureInstance> GetTextureFromGPUHandle(const std::string& texture, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
	using namespace Textures;
	return refTracker.AddRef(texture, [texture, gpuHandle]()
		{
			std::shared_ptr<MaterialTextureInstance> instance = std::make_shared<MaterialTextureInstance>();
			instance->textureName = texture;
			instance->gpuHandle = gpuHandle;
			return instance;
		}
	);
}

void DestroyMaterialTextureInstance(std::shared_ptr<MaterialTextureInstance>& texture)
{
	using namespace Textures;
	auto key = refTracker.FindKey(texture);
	refTracker.RemoveRef(key, texture);
}

void MaterialTextureInstance::Load(std::string& texture, DXGI_FORMAT format, unsigned int numFrames, unsigned int nMipMaps)
{
	using namespace Templates;
	std::filesystem::path path = GetTextureName(texture);
	path.replace_extension(".dds");
	std::string pathS = path.string();
	materialTexture = texture;
	CreateTextureResource(pathS, format, numFrames, nMipMaps);
}

void MaterialTextureInstance::CreateTextureResource(std::string& path, DXGI_FORMAT format, unsigned int numFrames, unsigned int nMipMaps)
{
	using namespace DeviceUtils;

	auto& d3dDevice = renderer->d3dDevice;
	auto& commandList = renderer->commandList;

	//Load the dds file to a buffer using LoadDDSTextureFromFile
	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DX::ThrowIfFailed(LoadDDSTextureFromFile(d3dDevice, nostd::StringToWString(path).c_str(), &texture, ddsData, subresources));

	//get the ammount of memory required for the upload
	auto uploadBufferSize = GetRequiredIntermediateSize(texture, 0, static_cast<unsigned int>(subresources.size()));

	//create the upload texture
	CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(&heapTypeUpload, D3D12_HEAP_FLAG_NONE, &uploadBufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload)));

	//use to command list to copy the texture data from cpu to gpu space
	UpdateSubresources(commandList, texture, upload, 0, 0, static_cast<unsigned int>(subresources.size()), subresources.data());

	//put a barrier on the texture from a copy destination to a pixel shader resource
	auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &transition);

	//these are common attributes for now
	viewDesc.Format = format;
	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (numFrames <= 1U)
	{
		//simple static textures
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipLevels = nMipMaps;
		viewDesc.Texture2D.MostDetailedMip = 0;
		viewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	}
	else
	{
		//array textures(animated gifs)
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewDesc.Texture2DArray.MipLevels = nMipMaps;
		viewDesc.Texture2DArray.MostDetailedMip = 0;
		viewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		viewDesc.Texture2DArray.ArraySize = numFrames;
	}

	//allocate descriptors handles for the SRV and kick the resource creation
	AllocCSUDescriptor(cpuHandle, gpuHandle);
	renderer->d3dDevice->CreateShaderResourceView(texture, &viewDesc, cpuHandle);
}

void MaterialTextureInstance::Destroy()
{
	/*
	texturesCacheRefCount[textureName]--;
	if (texturesCacheRefCount[textureName] == 0U)
	{
		texturesCache.erase(textureName);
		texturesCacheMipMaps.erase(textureName);
		texturesCacheRefCount.erase(textureName);
	}
	*/
}
