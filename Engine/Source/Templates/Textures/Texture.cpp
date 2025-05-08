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
#include <Editor.h>

extern std::shared_ptr<Renderer> renderer;

namespace Templates
{
	TEMPDEF_FULL(Texture);

	void CreateDDSFile(std::string uuid, std::filesystem::path path, bool overwrite, DXGI_FORMAT format, unsigned int width, unsigned int height, unsigned int mipLevels)
	{
		std::filesystem::path ddsPath = path;
		ddsPath.replace_extension(".dds");

		using namespace Utils;

		if (!std::filesystem::exists(ddsPath) || overwrite)
		{
			ConvertToDDS(path, ddsPath, format, width, height, mipLevels);
		}
		unsigned int numFrames = 0;
		GetImageAttributes(ddsPath, format, width, height, mipLevels, numFrames);

		TextureTemplate& t = GetTextureTemplates().at(uuid);

		nlohmann::json& json = std::get<1>(t);
		json.at("numFrames") = numFrames;
		json.at("format") = dxgiFormatsToString.at(format);
		json.at("width") = width;
		json.at("height") = height;
		json.at("mipLevels") = mipLevels;
	}

	void RebuildTexture(std::string uuid)
	{
		TextureTemplate& t = GetTextureTemplates().at(uuid);
		std::string& path = std::get<0>(t);
		nlohmann::json& json = std::get<1>(t);

		DXGI_FORMAT format = stringToDxgiFormat.at(json.at("format"));
		unsigned int width = json.at("width");
		unsigned int height = json.at("height");
		unsigned int mipLevels = json.at("mipLevels");
		CreateDDSFile(uuid, path, true, format, width, height, mipLevels);
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
			CreateDDSFile(texj.at("uuid"), name, false, stringToDxgiFormat.at(texture.at("format")));
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
		CreateDDSFile(texj.at("uuid"), name, false, format);
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

	bool Texture::DrawEditorInformationAttributes(std::string uuid)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "shader-information-atts";

		TextureTemplate& t = GetTextureTemplates().at(uuid);

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
		return false;
	}

	bool Texture::DrawEditorAssetAttributes(std::string uuid)
	{
		nlohmann::json& json = GetTextureTemplate(uuid);

		static std::vector<unsigned int> dimensions = {
			16384U, 8192U, 4096U, 2048U, 1024U, 512U, 256U, 128U, 64U, 32U, 16U, 8U, 4U, 2U, 1U,
		};

		std::vector<std::string> dimensionsS;
		std::transform(dimensions.begin(), dimensions.end(), std::back_inserter(dimensionsS), [](unsigned int i) { return std::to_string(i); });

		std::map<unsigned int, unsigned int> numMipMaps;
		std::transform(dimensions.begin(), dimensions.end(), std::inserter(numMipMaps, numMipMaps.end()), [&numMipMaps](unsigned int d)
			{
				return std::pair<unsigned int, unsigned int>(d, static_cast<unsigned int>(dimensions.size() - numMipMaps.size()));
			}
		);

		std::vector<std::string> numMipMapsS;
		std::transform(numMipMaps.begin(), numMipMaps.end(), std::back_inserter(numMipMapsS), [](std::pair<unsigned int, unsigned int> pair)
			{
				return std::to_string(pair.second);
			}
		);

		auto RecalculateMipMaps = [&numMipMaps](nlohmann::json& json)
			{
				unsigned int width = static_cast<unsigned int>(json.at("width"));
				unsigned int height = static_cast<unsigned int>(json.at("height"));
				unsigned int mipLevels = numMipMaps.at(max(width, height));
				unsigned int currentLevels = json.at("mipLevels");
				json.at("mipLevels") = std::min(currentLevels, mipLevels);
			};

		bool rebuildInstance = false;

		ImGui::PushID("texture-file-numFrames");
		{
			int numFrames = json.at("numFrames");
			ImGui::InputInt("Num Frames", &numFrames, 1, 100, ImGuiInputTextFlags_ReadOnly);
		}
		ImGui::PopID();

		ImGui::PushID("texture-file-format");
		{
			std::vector<std::string> textureFormats = nostd::GetKeysFromMap(stringToDxgiFormat);
			std::string format = json.at("format");
			DrawComboSelection(format, textureFormats, [&json, uuid, &rebuildInstance](std::string newFormat)
				{
					json.at("format") = newFormat;
					RebuildTexture(uuid);
					rebuildInstance = true;
				}, "Format"
			);
		}
		ImGui::PopID();

		ImGui::PushID("texture-file-width");
		{
			unsigned int width = json.at("width");
			std::string widthS = std::to_string(width);
			DrawComboSelection(widthS, dimensionsS, [&json, RecalculateMipMaps, uuid, &rebuildInstance](std::string newWidth)
				{
					json.at("width") = std::stoi(newWidth);
					RecalculateMipMaps(json);
					RebuildTexture(uuid);
					rebuildInstance = true;
				}, "Width"
			);
		}
		ImGui::PopID();

		ImGui::PushID("texture-file-height");
		{
			unsigned int height = json.at("height");
			std::string heightS = std::to_string(height);
			DrawComboSelection(heightS, dimensionsS, [&json, RecalculateMipMaps, uuid, &rebuildInstance](std::string newHeight)
				{
					json.at("height") = std::stoi(newHeight);
					RecalculateMipMaps(json);
					RebuildTexture(uuid);
					rebuildInstance = true;
				}, "Height"
			);
		}
		ImGui::PopID();

		ImGui::PushID("texture-file-mipLevels");
		{
			int mipLevels = json.at("mipLevels");
			std::string mipLevelsS = std::to_string(mipLevels);
			DrawComboSelection(mipLevelsS, numMipMapsS, [&json, RecalculateMipMaps, uuid, &rebuildInstance](std::string newMipLevels)
				{
					json.at("mipLevels") = std::stoi(newMipLevels);
					RecalculateMipMaps(json);
					RebuildTexture(uuid);
					rebuildInstance = true;
				}, "MipLevels"
			);
		}
		ImGui::PopID();

		if (rebuildInstance)
		{
			std::shared_ptr<TextureInstance> instance = Templates::GetTextureInstance(uuid);
			if (instance)
			{
				instance->rebuildTexture = true;
			}
		}

		return false;
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
#if defined(_DEVELOPMENT)
		if (!std::filesystem::exists(path))
		{
			RebuildTexture(texture);
		}
#endif
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

	void TextureInstance::ReleaseResources()
	{
		FreeCSUDescriptor(cpuHandle, gpuHandle);
		texture = nullptr;
		upload = nullptr;
	}

	std::map<TextureType, std::shared_ptr<TextureInstance>> GetTextures(const std::map<TextureType, std::string>& textures)
	{
		std::map<TextureType, std::shared_ptr<TextureInstance>> texInstances;

		std::transform(textures.begin(), textures.end(), std::inserter(texInstances, texInstances.end()), [](auto pair)
			{
				std::string& texture = pair.second;
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
		auto key = refTracker.FindKey(texture);
		refTracker.RemoveRef(key, texture);
	}

	void ReloadTextureInstances()
	{
		std::vector<std::shared_ptr<TextureInstance>> texturesToReload;
		refTracker.ForEach([&texturesToReload](std::shared_ptr<TextureInstance> texture)
			{
				if (texture->rebuildTexture)
				{
					texturesToReload.push_back(texture);
				}
			}
		);

		if (texturesToReload.size() == 0ULL) return;

		renderer->Flush();
		renderer->RenderCriticalFrame([&texturesToReload]
			{
				std::for_each(texturesToReload.begin(), texturesToReload.end(), [](std::shared_ptr<TextureInstance>& texture)
					{
						texture->ReleaseResources();
					}
				);
				std::for_each(texturesToReload.begin(), texturesToReload.end(), [](std::shared_ptr<TextureInstance>& texture)
					{
						nlohmann::json& json = GetTextureTemplate(texture->materialTexture);
						texture->Load(texture->materialTexture, stringToDxgiFormat.at(json.at("format")), json.at("numFrames"), json.at("mipLevels"));
					}
				);
			}
		);
	}

}