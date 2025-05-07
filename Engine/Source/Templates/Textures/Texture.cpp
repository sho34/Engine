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

	std::string GetUtilsPathX()
	{
		char* utilsPath;
		size_t utilsPathLen;
		_dupenv_s(&utilsPath, &utilsPathLen, "CULPEO_UTILS");

		return std::string(utilsPath);
	}

	void GetImageAttributes(std::filesystem::path src, DXGI_FORMAT& format, unsigned int& width, unsigned int& height, unsigned int& mipLevels, unsigned int& numFrames)
	{
		using namespace raymii;

		std::string cmdInfo = GetUtilsPathX() + "texdiag.exe info " + src.string();
		CommandResult resultInfo = Command::exec(cmdInfo);

		std::string text = resultInfo.output;
		std::regex pattern("width = ([\\d]+)[\\w\\d\\n\\W]+height = ([\\d]+)[\\w\\d\\n\\W]+mipLevels = ([\\d]+)[\\w\\d\\n\\W]+arraySize = ([\\d]+)[\\w\\d\\n\\W]+format = ([\\w]+)");
		std::smatch matches;

		if (std::regex_search(text, matches, pattern))
		{
			width = std::stoi(matches[1]);
			height = std::stoi(matches[2]);
			mipLevels = std::stoi(matches[3]);
			numFrames = std::stoi(matches[4]);
			format = stringToDxgiFormat.at(matches[5]);
		}
		else
		{
			assert(!!!"no info found from image file");
		}
	}

	void ConvertToDDS(std::filesystem::path image, std::filesystem::path dds, DXGI_FORMAT desiredFormat)
	{
		//DXGI_FORMAT format;
		//unsigned int width;
		//unsigned int height;
		//unsigned int mipLevels;
		//unsigned int numFrames;
		//GetImageAttributes(image, format, width, height, mipLevels, numFrames);

		/*
		std::map<DXGI_FORMAT, DXGI_FORMAT> formatLUT = {
			{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM }
		};

		if (formatLUT.contains(format))
		{
			format = formatLUT.at(format);
		}
		*/

		using namespace raymii;

		//get the format of the file
		std::filesystem::path parentPath = image.parent_path();
		std::string cmdConv = GetUtilsPathX() + "texconv.exe " + image.string() + " -f " + dxgiFormatsToString.at(desiredFormat) + " -y";
		CommandResult result = Command::exec(cmdConv);

		std::filesystem::path ddsUpperCase = dds;
		ddsUpperCase.replace_extension(".DDS");

		std::filesystem::rename(ddsUpperCase, dds);
	}

	void CreateDDSFile(std::string uuid, std::filesystem::path path, DXGI_FORMAT format)
	{
		std::filesystem::path ddsPath = path;
		ddsPath.replace_extension(".dds");

		unsigned int width;
		unsigned int height;
		unsigned int mipLevels;
		unsigned int numFrames;

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
	void TextureInstance::Load(std::string& texture)
	{
		materialTexture = texture;

		TextureTemplate& t = GetTextureTemplates().at(texture);
		nlohmann::json& json = std::get<1>(t);

		std::filesystem::path path = std::get<0>(t);
		std::filesystem::path ddsPath = path; ddsPath.replace_extension(".dds");
		CreateTextureResource(ddsPath, stringToDxgiFormat.at(json.at("format")), json.at("numFrames"));
	}

	void TextureInstance::CreateTextureResource(std::filesystem::path path, DXGI_FORMAT format, unsigned int numFrames)
	{
		using namespace DeviceUtils;

		auto& d3dDevice = renderer->d3dDevice;
		auto& commandList = renderer->commandList;

		//Load the dds file to a buffer using LoadDDSTextureFromFile
		std::unique_ptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		DX::ThrowIfFailed(LoadDDSTextureFromFile(d3dDevice, nostd::StringToWString(path.string()).c_str(), &texture, ddsData, subresources));

		//get the ammount of memory required for the upload
		auto uploadBufferSize = GetRequiredIntermediateSize(texture, 0, static_cast<unsigned int>(subresources.size()));

		//create the upload texture
		CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(&heapTypeUpload, D3D12_HEAP_FLAG_NONE, &uploadBufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload)));

		//use to command list to copy the texture data from cpu to gpu space
		UpdateSubresources(commandList, texture, upload, 0, 0, static_cast<unsigned int>(subresources.size()), subresources.data());

		//calculate the mipmaps being aware of the array textures passed as numFrames
		unsigned int nMipMaps = static_cast<unsigned int>(subresources.size()) / max(numFrames, 1U);

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

	void TextureInstance::Destroy()
	{
		FreeCSUDescriptor(cpuHandle, gpuHandle);
	}
}