#include "pch.h"
#include "Texture.h"
#include <UUID.h>
#include <DXTypes.h>
#include <NoStd.h>
#include <DDSTextureLoader.h>
#include "../Templates.h"
#include "../TemplateDef.h"
#include "../../Renderer/Renderer.h"
#include "../../Common/DirectXHelper.h"
#include "../Utils/ImageConvert.h"

extern std::shared_ptr<Renderer> renderer;

namespace Templates
{
	//uuid to TextureTemplates
	std::map<std::string, TextureTemplate> textures;
	TemplatesContainer<TextureTemplate>& GetTextureTemplates()
	{
		return textures;
	}

	TEMPDEF_FULL(Texture);

	void CreateDDSFile(std::string uuid, std::filesystem::path path, DXGI_FORMAT format)
	{
		std::filesystem::path ddsPath = path;
		ddsPath.replace_extension(".dds");

		unsigned int width;
		unsigned int height;
		unsigned int mipLevels;
		unsigned int numFrames;

		using namespace Utils;

		if (!std::filesystem::exists(ddsPath))
		{
			ConvertToDDS(path, ddsPath, format);
		}
		GetImageAttributes(ddsPath, format, width, height, mipLevels, numFrames);

		TextureTemplate& t = GetTextureTemplates().at(uuid);

		nlohmann::json& json = std::get<1>(t);
		json.at("numFrames") = numFrames;
		json.at("format") = dxgiFormatsToString.at(format);
		json.at("width") = width;
		json.at("height") = height;
		json.at("mipLevels") = mipLevels;
	}

	void CreateTexturesTemplatesFromMaterial(nlohmann::json json)
	{
		if (!json.contains("textures")) return;
		for (auto& [textureType, texture] : json.at("textures").items())
		{
			if (texture.type() != nlohmann::json::value_t::object) continue;

			std::string name = nostd::normalize_path(texture.at("path"));
			nlohmann::json texj = nlohmann::json(
				{
					{ "uuid", getUUID() },
					{ "name", name },
					{ "numFrames", texture.at("numFrames") },
					{ "format", texture.at("format") },
					{ "width", 128 },
					{ "height", 128 },
					{ "mipLevels", 1 }
				}
			);

			CreateTexture(texj);
			CreateDDSFile(texj.at("uuid"), name, stringToDxgiFormat.at(texture.at("format")));
		}
	}

	std::string CreateTextureTemplate(std::string name, DXGI_FORMAT format)
	{
		nlohmann::json texj = nlohmann::json(
			{
				{ "uuid", getUUID() },
				{ "name", name},
				{ "numFrames", 0 },
				{ "format", dxgiFormatsToString.at(format) },
				{ "width", 128 },
				{ "height", 128 },
				{ "mipLevels", 1 }
			}
		);
		CreateTexture(texj);
		CreateDDSFile(texj.at("uuid"), name, format);
		return texj.at("uuid");
	}

	void DestroyTexture(std::string uuid)
	{
	}

	void ReleaseTexturesTemplates()
	{
	}

#if defined(_EDITOR)
	void DrawTexturePanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop)
	{
		std::string tableName = "texture-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			Texture::DrawEditorInformationAttributes(uuid);
			Texture::DrawEditorAssetAttributes(uuid);
			Texture::DrawEditorTexturePreview(uuid);
			ImGui::EndTable();
		}
	}

	void Texture::DrawEditorInformationAttributes(std::string uuid)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "shader-information-atts";

		TextureTemplate& t = textures.at(uuid);

		std::string fileName = std::get<0>(t);
		nlohmann::json& json = std::get<1>(t);
		ImDrawFileSelector(
			"##",
			fileName,
			[&json](std::filesystem::path texPath)
			{
				//std::filesystem::path hlslFilePath = shaderPath;
				//hlslFilePath.replace_extension(".hlsl");
				//json.at("path") = hlslFilePath.stem().string();
			},
			defaultAssetsFolder,
			"Texture files. (*.jpg, *.jpeg, *.png, *.gif)",
			"*.jpg;*.jpeg;*.png;*.gif",
			true
			);
	}

	void Texture::DrawEditorAssetAttributes(std::string uuid)
	{
	}

	void Texture::DrawEditorTexturePreview(std::string uuid)
	{
	}

	void CreateNewTexture()
	{
	}

	void DeleteTexture(std::string uuid)
	{
	}

	void DrawTexturesPopups()
	{
	}

#endif

	TEMPDEF_REFTRACKER(Texture);

	void TextureInstance::Load(std::string& texture, DXGI_FORMAT format, unsigned int numFrames, unsigned int nMipMaps)
	{
		using namespace Templates;
		std::filesystem::path path = GetTextureName(texture);
		path.replace_extension(".dds");
		std::string pathS = path.string();
		materialTexture = texture;
		CreateTextureResource(pathS, format, numFrames, nMipMaps);
	}

	void TextureInstance::CreateTextureResource(std::string& path, DXGI_FORMAT format, unsigned int numFrames, unsigned int nMipMaps)
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

	std::map<TextureType, std::shared_ptr<TextureInstance>> GetTextures(const std::map<TextureType, std::string>& textures)
	{
		std::map<TextureType, std::shared_ptr<TextureInstance>> texInstances;

		std::transform(textures.begin(), textures.end(), std::inserter(texInstances, texInstances.end()), [](auto pair)
			{
				std::string& texture = pair.second;
				using namespace Textures;
				std::shared_ptr<TextureInstance> instance = refTracker.AddRef(texture, [&texture]()
					{
						using namespace Templates;
						std::shared_ptr<TextureInstance> instance = std::make_shared<TextureInstance>();
						nlohmann::json& json = GetTextureTemplate(texture);
						instance->Load(texture, stringToDxgiFormat.at(json.at("format")), json.at("numFrames"), json.at("mipLevels"));
						return instance;
					}
				);
				return std::pair<TextureType, std::shared_ptr<TextureInstance>>(pair.first, instance);
			}
		);

		return texInstances;
	}

	std::shared_ptr<TextureInstance> GetTextureFromGPUHandle(const std::string& texture, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle)
	{
		using namespace Textures;
		return refTracker.AddRef(texture, [texture, gpuHandle]()
			{
				std::shared_ptr<TextureInstance> instance = std::make_shared<TextureInstance>();
				instance->textureName = texture;
				instance->gpuHandle = gpuHandle;
				return instance;
			}
		);
	}

	void DestroyTextureInstance(std::shared_ptr<TextureInstance>& texture)
	{
		using namespace Textures;
		auto key = refTracker.FindKey(texture);
		refTracker.RemoveRef(key, texture);
	}

}