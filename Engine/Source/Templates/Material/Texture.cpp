#include "pch.h"
#include "Texture.h"
#include "../../Common/DirectXHelper.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/DeviceUtils/Resources/Resources.h"
#include "../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include <NoStd.h>
#include <wrl.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <atlbase.h>
#include <nlohmann/json.hpp>

extern std::shared_ptr<Renderer> renderer;

std::unordered_map<std::string, CComPtr<ID3D12Resource>> texturesCache;
std::unordered_map<std::string, unsigned int> texturesCacheMipMaps;
std::unordered_map<std::string, unsigned int> texturesCacheRefCount;

nostd::RefTracker<MaterialTexture, std::shared_ptr<MaterialTextureInstance>> refTracker;

void TransformJsonToMaterialTextures(std::map<TextureType, MaterialTexture>& textures, nlohmann::json object, const std::string& key) {

	if (!object.contains(key)) return;

	nlohmann::json jtextures = object[key];

	for (nlohmann::json::iterator it = jtextures.begin(); it != jtextures.end(); it++)
	{
		textures.insert_or_assign(strToTextureType.at(it.key()), MaterialTexture(
			{
				.path = it.value()["path"],
				.format = stringToDxgiFormat[it.value()["format"]],
				.numFrames = it.value()["numFrames"]
			}
		));
	}
}

std::map<TextureType, std::shared_ptr<MaterialTextureInstance>> GetTextures(const std::map<TextureType, MaterialTexture>& textures)
{
	std::map<TextureType, std::shared_ptr<MaterialTextureInstance>> texInstances;

	std::transform(textures.begin(), textures.end(), std::inserter(texInstances, texInstances.end()), [](auto pair)
		{
			MaterialTexture& texture = pair.second;
			std::shared_ptr<MaterialTextureInstance> instance = refTracker.AddRef(texture, [&texture]()
				{
					std::shared_ptr<MaterialTextureInstance> instance = std::make_shared<MaterialTextureInstance>();
					instance->Load(texture);
					return instance;
				}
			);
			return std::pair<TextureType, std::shared_ptr<MaterialTextureInstance>>(pair.first, instance);
		}
	);

	return texInstances;
}

std::shared_ptr<MaterialTextureInstance> GetTextureFromGPUHandle(const MaterialTexture& texture)
{
	return refTracker.AddRef(texture, [texture]()
		{
			std::shared_ptr<MaterialTextureInstance> instance = std::make_shared<MaterialTextureInstance>();
			instance->textureName = texture.path;
			instance->gpuHandle = texture.gpuHandle;
			return instance;
		}
	);
}

void DestroyMaterialTextureInstance(std::shared_ptr<MaterialTextureInstance>& texture)
{
	auto key = refTracker.FindKey(texture);
	refTracker.RemoveRef(key, texture);
}

void MaterialTextureInstance::Load(MaterialTexture& texture)
{
	if (texture.numFrames == 0U)
	{
		CreateTextureResource(texture);
	}
	else
	{
		CreateTextureArrayResource(texture);
	}
}

void MaterialTextureInstance::CreateTextureResource(MaterialTexture& tex)
{
	using namespace DirectX;
	using namespace DeviceUtils;

	textureName = tex.path;

	auto& d3dDevice = renderer->d3dDevice;
	auto& commandList = renderer->commandList;

	unsigned int nMipMaps = 0U;
	if (texturesCache.contains(tex.path))
	{
		//if the texture is in the cache, take a new reference from it
		//si la textura esta en el cache, toma una nueva referencia de el
		texture = texturesCache[tex.path];
		nMipMaps = texturesCacheMipMaps[tex.path];
		texturesCacheRefCount[tex.path]++;
	}
	else
	{
		//to simplify things use dds image format and load trough DirectXTK12
		//para simplificar las cosas usamos el formato de imagenes dds y lo cargamos a travez de DirecXTK12
		std::unique_ptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		DX::ThrowIfFailed(LoadDDSTextureFromFile(d3dDevice, nostd::StringToWString(tex.path).c_str(), &texture, ddsData, subresources));

		//obtain the size of the buffer which is SUM(1->n):mipmap(i)->width*height*4
		//obtener el porte del buffer el cual es SUM(1->n):mipmap(i)->width*height*4
		auto uploadBufferSize = GetRequiredIntermediateSize(texture, 0, static_cast<unsigned int>(subresources.size()));

		//create the upload texture
		//creamos la textura de subida
		CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(&heapTypeUpload, D3D12_HEAP_FLAG_NONE, &uploadBufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload)));

		//copy the data for the texture and it's mipmaps
		//copiamos los datos de la textura y sus mipmaps
		UpdateSubresources(commandList, texture, upload, 0, 0, static_cast<unsigned int>(subresources.size()), subresources.data());

		nMipMaps = static_cast<unsigned int>(subresources.size());

		//store a reference to the texture in it's mipmaps in the cache
		//guardamos una referencia a la textura y sus mipmaps en el cache
		texturesCache[tex.path] = texture;
		texturesCacheMipMaps[tex.path] = nMipMaps;
		texturesCacheRefCount[tex.path] = 1U;

		//now put a barrier for this copy to avoid keeping processing things untils this is done
		//ahora ponemos una barrera para esta copia para esperar a que el procesamiento complete
		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &transition);
	}

	//create the descriptor for this texture
	//crear el descriptor de la textura
	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.Format = tex.format;
	viewDesc.Texture2D.MipLevels = nMipMaps;
	viewDesc.Texture2D.MostDetailedMip = 0;
	viewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	//create the handles and SRV descriptor
	AllocCSUDescriptor(cpuHandle, gpuHandle);
	renderer->d3dDevice->CreateShaderResourceView(texture, &viewDesc, cpuHandle);
}

void MaterialTextureInstance::CreateTextureArrayResource(MaterialTexture& tex)
{
	using namespace DirectX;
	using namespace DeviceUtils;

	auto& d3dDevice = renderer->d3dDevice;
	auto& commandList = renderer->commandList;

	unsigned int nMipMaps = 0U;
	if (texturesCache.contains(tex.path))
	{
		//if the texture is in the cache, take a new reference from it
		//si la textura esta en el cache, toma una nueva referencia de el
		texture = texturesCache[tex.path];
		nMipMaps = texturesCacheMipMaps[tex.path];
	}
	else
	{
		//to simplify things use dds image format and load through DirectXTK12
		//para simplificar las cosas usamos el formato de imagenes dds y lo cargamos a travez de DirecXTK12
		std::unique_ptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		DX::ThrowIfFailed(LoadDDSTextureFromFile(d3dDevice, nostd::StringToWString(tex.path).c_str(), &texture.p, ddsData, subresources));

		//obtain the size of the buffer which is numFrames*SUM(1->n):mipmap(i)->width*height*4
		//obtener el porte del buffer el cual es numFrames*SUM(1->n):mipmap(i)->width*height*4
		auto uploadBufferSize = GetRequiredIntermediateSize(texture.p, 0, static_cast<unsigned int>(subresources.size()));

		//create the upload texture
		//creamos la textura de subida
		CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(&heapTypeUpload, D3D12_HEAP_FLAG_NONE, &uploadBufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload)));

		//copy the data for the texture and it's mipmaps
		//copiamos los datos de la textura y sus mipmaps
		UpdateSubresources(commandList, texture, upload, 0, 0, static_cast<unsigned int>(subresources.size()), subresources.data());

		nMipMaps = static_cast<unsigned int>(subresources.size()) / tex.numFrames;

		//store a reference to the texture in it's mipmaps in the cache
		//guardamos una referencia a la textura y sus mipmaps en el cache
		texturesCache[tex.path] = texture;
		texturesCacheMipMaps[tex.path] = nMipMaps;

		//now put a barrier for this copy to avoid keeping processing things untils this is done
		//ahora ponemos una barrera para esta copia para esperar a que el procesamiento complete
		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &transition);
	}

	//create the descriptor for this texture
	//crear el descriptor de la textura
	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.Format = tex.format;
	viewDesc.Texture2DArray.MipLevels = nMipMaps;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
	viewDesc.Texture2DArray.ArraySize = tex.numFrames;

	//create the handles and SRV descriptor
	AllocCSUDescriptor(cpuHandle, gpuHandle);
	renderer->d3dDevice->CreateShaderResourceView(texture, &viewDesc, cpuHandle);
}

void MaterialTextureInstance::Destroy()
{
	texturesCacheRefCount[textureName]--;
	if (texturesCacheRefCount[textureName] == 0U)
	{
		texturesCache.erase(textureName);
		texturesCacheMipMaps.erase(textureName);
		texturesCacheRefCount.erase(textureName);
	}
}
