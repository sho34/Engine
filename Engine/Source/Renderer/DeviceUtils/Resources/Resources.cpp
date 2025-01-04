#include "pch.h"
#include "Resources.h"
#include "../../../Common/DirectXHelper.h"

namespace DeviceUtils::Resources {

	static std::mutex loadBufferResourceMutex;
	void UpdateBufferResource(CComPtr<ID3D12Device2>& device, CComPtr<ID3D12GraphicsCommandList2>& commandList,
		CComPtr<ID3D12Resource>& pDestinationResource, CComPtr<ID3D12Resource>& pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData) {
		std::lock_guard<std::mutex> lock(loadBufferResourceMutex);

		size_t bufferSize = numElements * elementSize;

		// Create a committed resource for the GPU resource in a default heap.
		const CD3DX12_HEAP_PROPERTIES defaultHeapTypeProperties(D3D12_HEAP_TYPE_DEFAULT);
		const CD3DX12_RESOURCE_DESC defaultHeapBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE);
		DX::ThrowIfFailed(device->CreateCommittedResource(
			&defaultHeapTypeProperties,
			D3D12_HEAP_FLAG_NONE,
			&defaultHeapBufferDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&pDestinationResource)));

		//NAME_D3D12_OBJECT(pDestinationResource);

		// Create a committed resource for the upload.
		if (bufferData)
		{
			const CD3DX12_HEAP_PROPERTIES uploadHeapTypeProperties(D3D12_HEAP_TYPE_UPLOAD);
			const CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
			DX::ThrowIfFailed(device->CreateCommittedResource(
				&uploadHeapTypeProperties,
				D3D12_HEAP_FLAG_NONE,
				&uploadBufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&pIntermediateResource)));

			//NAME_D3D12_OBJECT(pIntermediateResource);

			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = bufferData;
			subresourceData.RowPitch = bufferSize;
			subresourceData.SlicePitch = subresourceData.RowPitch;

			UpdateSubresources(commandList,
				pDestinationResource, pIntermediateResource,
				0, 0, 1, &subresourceData);
		}
	}

	static std::unordered_map<std::string, CComPtr<ID3D12Resource>> texturesCache;
	static std::unordered_map<std::string, UINT> texturesCacheMipMaps;

	void CreateTextureResource(CComPtr<ID3D12Device2>& d3dDevice, CComPtr<ID3D12GraphicsCommandList2>& commandList,
		std::string path, CComPtr<ID3D12Resource>& texture, CComPtr<ID3D12Resource>& textureUpload, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, DXGI_FORMAT textureFormat) {

		UINT nMipMaps = 0U;
		if (texturesCache.find(path) != texturesCache.end()) {
			//if the texture is in the cache, take a new reference from it
			//si la textura esta en el cache, toma una nueva referencia de el
			texture = texturesCache[path];
			nMipMaps = texturesCacheMipMaps[path];
		}
		else {
			//to simplify things use dds image format and load trough DirectXTK12
			//para simplificar las cosas usamos el formato de imagenes dds y lo cargamos a travez de DirecXTK12
			std::unique_ptr<uint8_t[]> ddsData;
			std::vector<D3D12_SUBRESOURCE_DATA> subresources;
			DX::ThrowIfFailed(LoadDDSTextureFromFile(d3dDevice, StringToWString(path).c_str(), &texture, ddsData, subresources));

			//obtain the size of the buffer which is SUM(1->n):mipmap(i)->width*height*4
			//obtener el porte del buffer el cual es SUM(1->n):mipmap(i)->width*height*4
			auto uploadBufferSize = GetRequiredIntermediateSize(texture, 0, static_cast<UINT>(subresources.size()));

			//create the upload texture
			//creamos la textura de subida
			CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
			DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(&heapTypeUpload, D3D12_HEAP_FLAG_NONE, &uploadBufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUpload)));

			//copy the data for the texture and it's mipmaps
			//copiamos los datos de la textura y sus mipmaps
			UpdateSubresources(commandList, texture, textureUpload, 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

			nMipMaps = static_cast<UINT>(subresources.size());

			//store a reference to the texture in it's mipmaps in the cache
			//guardamos una referencia a la textura y sus mipmaps en el cache
			texturesCache[path] = texture;
			texturesCacheMipMaps[path] = nMipMaps;

			//now put a barrier for this copy to avoid keeping processing things untils this is done
			//ahora ponemos una barrera para esta copia para esperar a que el procesamiento complete
			auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			commandList->ResourceBarrier(1, &transition);
		}

		//create the descriptor for this texture
		//crear el descriptor de la textura
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureFormat;
		srvDesc.Texture2D.MipLevels = nMipMaps;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	}

	void CreateTextureArrayResource(CComPtr<ID3D12Device2>& d3dDevice, CComPtr<ID3D12GraphicsCommandList2>& commandList,
		std::string path, CComPtr<ID3D12Resource>& texture, CComPtr<ID3D12Resource>& textureUpload, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, UINT numFrames, DXGI_FORMAT textureFormat) {

		UINT nMipMaps = 0U;
		if (texturesCache.find(path) != texturesCache.end()) {
			//if the texture is in the cache, take a new reference from it
			//si la textura esta en el cache, toma una nueva referencia de el
			texture = texturesCache[path];
			nMipMaps = texturesCacheMipMaps[path];
		}
		else {
			//to simplify things use dds image format and load through DirectXTK12
			//para simplificar las cosas usamos el formato de imagenes dds y lo cargamos a travez de DirecXTK12
			std::unique_ptr<uint8_t[]> ddsData;
			std::vector<D3D12_SUBRESOURCE_DATA> subresources;
			DX::ThrowIfFailed(LoadDDSTextureFromFile(d3dDevice, StringToWString(path).c_str(), &texture.p, ddsData, subresources));

			//obtain the size of the buffer which is numFrames*SUM(1->n):mipmap(i)->width*height*4
			//obtener el porte del buffer el cual es numFrames*SUM(1->n):mipmap(i)->width*height*4
			auto uploadBufferSize = GetRequiredIntermediateSize(texture.p, 0, static_cast<UINT>(subresources.size()));

			//create the upload texture
			//creamos la textura de subida
			CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
			DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(&heapTypeUpload, D3D12_HEAP_FLAG_NONE, &uploadBufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUpload)));

			//copy the data for the texture and it's mipmaps
			//copiamos los datos de la textura y sus mipmaps
			UpdateSubresources(commandList, texture, textureUpload, 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

			nMipMaps = static_cast<UINT>(subresources.size()) / numFrames;

			//store a reference to the texture in it's mipmaps in the cache
			//guardamos una referencia a la textura y sus mipmaps en el cache
			texturesCache[path] = texture;
			texturesCacheMipMaps[path] = nMipMaps;

			//now put a barrier for this copy to avoid keeping processing things untils this is done
			//ahora ponemos una barrera para esta copia para esperar a que el procesamiento complete
			auto transition = CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			commandList->ResourceBarrier(1, &transition);
		}

		//create the descriptor for this texture
		//crear el descriptor de la textura
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureFormat;
		srvDesc.Texture2DArray.MipLevels = nMipMaps;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		srvDesc.Texture2DArray.ArraySize = numFrames;
	}

	void DestroyTextureResources()
	{

		for (auto& [name, tex] : texturesCache) {
			tex.Release();
			tex = nullptr;
		}
		texturesCache.clear();

	}

}