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
#include "../../Editor/Editor.h"
#include "../../Shaders/Compute/IBL/DiffuseIrradianceMap.h"
#include "../../Shaders/Compute/IBL/PrefilteredEnvironmentMap.h"
#include "../../Shaders/Compute/IBL/BRDFLUT.h"

extern std::shared_ptr<Renderer> renderer;

namespace Editor {
	extern std::string selTemp;
};

namespace Templates
{
#if defined(_EDITOR)
	std::shared_ptr<TextureInstance> texturePreview = nullptr;
	std::vector<std::string> cubeTextureAxesNames = { "X+" , "X-" , "Y+" , "Y-" , "Z+" , "Z-" , };
	std::map<TextureType, std::string> textureTypeFilesTitle = {
		{ TextureType_2D, "Texture files. (*.jpg, *.jpeg, *.png, *.bmp)"},
		{ TextureType_Array, "Texture files. (*.gif)"},
		{ TextureType_Cube, "Texture files. (*.jpg, *.jpeg, *.png, *.bmp)"},
	};
	std::map<TextureType, std::string> textureTypeFilesFilter = {
		{ TextureType_2D, "*.jpg;*.jpeg;*.png;*.bmp" },
		{ TextureType_Array,"*.gif" },
		{ TextureType_Cube,"*.jpg;*.jpeg;*.png;*.bmp" },
	};
#endif

	namespace Texture
	{
#if defined(_EDITOR)
		nlohmann::json creationJson;
		unsigned int popupModalId = 0U;
		std::filesystem::path filePickingDirectory = defaultAssetsFolder;
		int imageFrame = 1U;
		bool createCubeFromSkybox = false;
		bool createDiffuseCubeIBL = false;
		bool createSpecularCubeIBL = false;
		bool createBRDFCubeIBL = false;
		std::string createCubeUUID = "";
#endif
	};

	TEMPDEF_FULL(Texture);
	TEMPDEF_REFTRACKER(Texture);

	void Create2DDDSFile(nlohmann::json json, bool overwrite)
	{
		using namespace Utils;

		std::filesystem::path ddsPath = json.at("name");
		ddsPath.replace_extension(".dds");

		unsigned int width, height, mipLevels, numFrames;
		DXGI_FORMAT format;

		if (!std::filesystem::exists(ddsPath))
			overwrite = true;

		if (!overwrite)
		{
			GetImageAttributes(ddsPath, format, width, height, mipLevels, numFrames);
		}
		else
		{
			std::filesystem::path image = json.at("images").at(0);
			GetImageAttributes(image, format, width, height, mipLevels, numFrames);
			if (!IsPowerOfTwo(width)) { width = PrevPowerOfTwo(width); }
			if (!IsPowerOfTwo(height)) { height = PrevPowerOfTwo(height); }
			ConvertToDDS(image, ddsPath, format, width, height, mipLevels);
		}

		TextureTemplate& t = GetTextureTemplates().at(json.at("uuid"));

		nlohmann::json& tj = std::get<1>(t);
		tj.at("numFrames") = numFrames;
		tj.at("format") = dxgiFormatsToString.at(format);
		tj.at("width") = width;
		tj.at("height") = height;
		tj.at("mipLevels") = mipLevels;
	}

	void CreateArrayDDSFile(nlohmann::json json, bool overwrite)
	{
		using namespace Utils;

		std::filesystem::path ddsPath = json.at("name");
		ddsPath.replace_extension(".dds");

		unsigned int width, height, mipLevels, numFrames;
		DXGI_FORMAT format;

		if (!std::filesystem::exists(ddsPath))
			overwrite = true;

		if (!overwrite)
		{
			GetImageAttributes(ddsPath, format, width, height, mipLevels, numFrames);
		}
		else
		{
			std::filesystem::path image = json.at("images").at(0);
			AssembleArrayDDSFromGif(ddsPath, image);
			GetImageAttributes(image, format, width, height, mipLevels, numFrames);
			if (!IsPowerOfTwo(width)) { width = PrevPowerOfTwo(width); }
			if (!IsPowerOfTwo(height)) { height = PrevPowerOfTwo(height); }
			ConvertToDDS(image, ddsPath, stringToDxgiFormat.at(json.at("format")), width, height, mipLevels);
			GetImageAttributes(ddsPath, format, width, height, mipLevels, numFrames);
		}

		TextureTemplate& t = GetTextureTemplates().at(json.at("uuid"));

		nlohmann::json& tj = std::get<1>(t);
		tj.at("numFrames") = numFrames;
		tj.at("format") = dxgiFormatsToString.at(format);
		tj.at("width") = width;
		tj.at("height") = height;
		tj.at("mipLevels") = mipLevels;
	}

	void CreateCubeDDSFile(nlohmann::json json, bool overwrite)
	{
		using namespace Utils;

		std::filesystem::path ddsPath = json.at("name");
		ddsPath.replace_extension(".dds");

		unsigned int width, height, mipLevels, numFrames;
		DXGI_FORMAT format;

		if (!std::filesystem::exists(ddsPath))
			overwrite = true;

		if (!overwrite)
		{
			GetImageAttributes(ddsPath, format, width, height, mipLevels, numFrames);
		}
		else
		{
			if (!Texture::createCubeFromSkybox)
			{
				unsigned int widthO, widthN;
				unsigned int heightO, heightN;
				unsigned int mipLevelsO, mipLevelsN;
				unsigned int numFramesO, numFramesN;
				DXGI_FORMAT formatO;

				for (unsigned int i = 0U; i < json.at("images").size(); i++)
				{
					std::filesystem::path imagePath = json.at("images").at(i);
					GetImageAttributes(imagePath, formatO, widthO, heightO, mipLevelsO, numFramesO);
					widthN = std::min(i == 0 ? widthO : widthN, widthO);
					heightN = std::min(i == 0 ? heightO : heightN, heightO);
					mipLevelsN = std::min(i == 0 ? mipLevelsO : mipLevelsN, mipLevelsO);
					numFramesN = std::min(i == 0 ? numFramesO : numFramesN, numFramesO);
				}

				width = widthN;
				height = heightN;
				format = stringToDxgiFormat.at(json.at("format"));

				if (!IsPowerOfTwo(width)) { width = PrevPowerOfTwo(width); }
				if (!IsPowerOfTwo(height)) { height = PrevPowerOfTwo(height); }

				AssembleCubeDDS(ddsPath, json.at("images"), width, height);
				ConvertToDDS(ddsPath, ddsPath, format, 0U, 0U, 0U);
				GetImageAttributes(ddsPath, format, width, height, mipLevels, numFrames);
			}
			else
			{
				AssembleCubeDDSFromSkybox(ddsPath, json.at("images").at(0));
				GetImageAttributes(ddsPath, format, width, height, mipLevels, numFrames);
				if (!IsPowerOfTwo(width)) { width = PrevPowerOfTwo(width); }
				if (!IsPowerOfTwo(height)) { height = PrevPowerOfTwo(height); }
				ConvertToDDS(ddsPath, ddsPath, stringToDxgiFormat.at(json.at("format")), width, height, 0U);
				GetImageAttributes(ddsPath, format, width, height, mipLevels, numFrames);
			}
		}

		TextureTemplate& t = GetTextureTemplates().at(json.at("uuid"));

		nlohmann::json& tj = std::get<1>(t);
		tj.at("numFrames") = numFrames;
		tj.at("format") = dxgiFormatsToString.at(format);
		tj.at("width") = width;
		tj.at("height") = height;
		tj.at("mipLevels") = mipLevels;
	}

	void CreateDDSFile(nlohmann::json& json, bool overwrite)
	{
		std::filesystem::path dir = json.at("name");
		dir.remove_filename();
		std::filesystem::create_directory(dir);

		switch (stringToTextureType.at(json.at("type")))
		{
		case TextureType_2D:
		{
			Create2DDDSFile(json, overwrite);
		}
		break;
		case TextureType_Array:
		{
			CreateArrayDDSFile(json, overwrite);
		}
		break;
		case TextureType_Cube:
		{
			CreateCubeDDSFile(json, overwrite);
		}
		break;
		}
	}

	void RebuildTexture(std::string uuid)
	{
		TextureTemplate& t = GetTextureTemplates().at(uuid);
		std::string& path = std::get<0>(t);
		nlohmann::json& json = std::get<1>(t);
		nlohmann::json buildJson = json;
		buildJson["name"] = path;
		buildJson["uuid"] = uuid;
		CreateDDSFile(buildJson, true);
	}

	nlohmann::json CreateBaseTextureJson(std::string name, unsigned int numFrames, DXGI_FORMAT format)
	{
		return nlohmann::json(
			{
				{ "uuid", getUUID() },
				{ "name", name },
				{ "images", { name } },
				{ "numFrames", numFrames },
				{ "format", dxgiFormatsToString.at(format) },
				{ "type", textureTypeToString.at(TextureType_2D) },
				{ "width", 128 },
				{ "height", 128 },
				{ "mipLevels", 1 }
			}
		);
	}

	void CreateTexturesTemplatesFromMaterial(nlohmann::json json)
	{
		if (!json.contains("textures")) return;
		for (auto& [textureType, texture] : json.at("textures").items())
		{
			if (texture.type() != nlohmann::json::value_t::object) continue;

			std::string name = nostd::normalize_path(texture.at("path"));
			nlohmann::json texj = CreateBaseTextureJson(name, texture.at("numFrames"), stringToDxgiFormat.at(texture.at("format")));
			CreateTexture(texj);
			CreateDDSFile(texj, false);
		}
	}

	std::string CreateTextureTemplate(std::string name, DXGI_FORMAT format)
	{
		nlohmann::json texj = CreateBaseTextureJson(name, 0, format);
		CreateTexture(texj);
		CreateDDSFile(texj, false);
		return texj.at("uuid");
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
		std::string parentFolder = Texture::filePickingDirectory.relative_path().string();

		bool rebuildInstance = false;

		for (unsigned int i = 0; i < json.at("images").size(); i++)
		{
			nlohmann::json jsonF = { {"file",json.at("images").at(i)} };
			ImGui::PushID((std::string("image-") + std::to_string(i + 1)).c_str());
			{
				ImDrawJsonFilePicker(jsonF,
					"file",
					parentFolder,
					textureTypeFilesTitle.at(stringToTextureType.at(json.at("type"))),
					textureTypeFilesFilter.at(stringToTextureType.at(json.at("type"))),
					[&jsonF, &json, i, &t, &rebuildInstance]
					{
						std::filesystem::path filePath = nostd::normalize_path(jsonF.at("file"));
						Texture::filePickingDirectory = filePath.relative_path().remove_filename();
						json.at("images")[i] = filePath.string();
						if (stringToTextureType.at(json.at("type")) != TextureType_Cube)
						{
							std::string& name = std::get<0>(t);
							name = filePath.string();
						}
						rebuildInstance = true;
					}
				, "##");
			}
			ImGui::PopID();
		}

		if (rebuildInstance)
		{
			RebuildInstance(uuid);
		}

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

		ImGui::PushID("texture-file-type");
		{
			ImGui::InputText("Type", json.at("type").get_ptr<std::string*>(), ImGuiInputTextFlags_ReadOnly);
		}
		ImGui::PopID();

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
					rebuildInstance = true;
				}, "MipLevels"
			);
		}
		ImGui::PopID();

		if (rebuildInstance)
		{
			RebuildInstance(uuid);
		}

		return false;
	}

	void Texture::RebuildInstance(std::string& uuid)
	{
		RebuildTexture(uuid);
		std::shared_ptr<TextureInstance> instance = Templates::GetTextureInstance(uuid);
		if (instance)
		{
			instance->updateFlag |= TextureUpdateFlags_Reload;
		}
		texturePreview->updateFlag |= TextureUpdateFlags_Reload;
	}

	void Texture::DrawEditorTexturePreview(std::string uuid)
	{
		nlohmann::json& json = GetTextureTemplate(uuid);

		unsigned int numFrames = json.at("numFrames");
		if (numFrames > 1U)
		{
			if (ImGui::SliderInt("Frame", &Texture::imageFrame, 1, numFrames))
			{
				texturePreview->updateFlag |= TextureUpdateFlags_Reload;
			}
		}

		ImDrawTextureImage((ImTextureID)texturePreview->gpuHandle.ptr,
			static_cast<unsigned int>(json.at("width")),
			static_cast<unsigned int>(json.at("height"))
		);
	}

	void CreateNewTexture()
	{
		ResetTexturePopupParameters();
		Texture::popupModalId = TexturePopupModal_CreateNew;
		Texture::creationJson = CreateBaseTextureJson("", 1U, DXGI_FORMAT_R8G8B8A8_UNORM);
		/*
		Texture::creationJson["type"] = "Cube";
		Texture::creationJson["name"] = "skybox";
		Texture::creationJson["images"].clear();
		Texture::creationJson["images"].push_back("Assets/ibl/family/skybox.png");
		Texture::creationJson["images"].push_back("Assets/ibl/family/skybox.png");
		Texture::creationJson["images"].push_back("Assets/ibl/family/skybox.png");
		Texture::creationJson["images"].push_back("Assets/ibl/family/skybox.png");
		Texture::creationJson["images"].push_back("Assets/ibl/family/skybox.png");
		Texture::creationJson["images"].push_back("Assets/ibl/family/skybox.png");
		Texture::createCubeFromSkybox = true;
		Texture::createDiffuseCubeIBL = true;
		Texture::createSpecularCubeIBL = true;
		Texture::createBRDFCubeIBL = true;
		*/
	}

	void ResetTexturePopupParameters()
	{
		Texture::popupModalId = 0;
		Texture::createCubeFromSkybox = false;
		Texture::imageFrame = 1U;
		Texture::filePickingDirectory = defaultAssetsFolder;
	}

	void ResetTexturePopupIBLParameters()
	{
		Texture::createDiffuseCubeIBL = false;
		Texture::createSpecularCubeIBL = false;
		Texture::createBRDFCubeIBL = false;
		Texture::createCubeUUID = "";
	}

	void DeleteTexture(std::string uuid)
	{
		nlohmann::json json = GetTextureTemplate(uuid);
		if (json.contains("systemCreated") && json.at("systemCreated") == true)
		{
			Texture::popupModalId = TexturePopupModal_CannotDelete;
			return;
		}

		GetTextureTemplates().erase(uuid);
	}

	void DrawTexturesPopups()
	{
		Editor::DrawOkPopup(Texture::popupModalId, TexturePopupModal_CannotDelete, "Cannot delete texture", []
			{
				ImGui::Text("Cannot delete a system created texture");
			}
		);

		Editor::DrawCreateWindow(Texture::popupModalId, TexturePopupModal_CreateNew, "Create new texture", [](auto OnCancel)
			{
				nlohmann::json& json = Texture::creationJson;

				std::string parentFolder = Texture::filePickingDirectory.relative_path().string();

				TextureType textureType = stringToTextureType.at(json.at("type"));

				ImGui::PushID("texture-type");
				{
					DrawComboSelection(json.at("type").get<std::string>(), nostd::GetKeysFromMap(stringToTextureType), [&json](std::string type)
						{
							json.at("name") = "";
							json.at("type") = type;
							if (stringToTextureType.at(json.at("type")) != TextureType_Cube)
							{
								json["images"] = nlohmann::json::array({ "" });
							}
							else
							{
								json["images"] = nlohmann::json::array({ "","","","","","" });
							}
						}, "Type"
					);
				}
				ImGui::PopID();

				switch (stringToTextureType.at(json.at("type")))
				{
				case TextureType_2D:
				case TextureType_Array:
				{
					ImGui::PushID("texture-name");
					{
						ImDrawJsonFilePicker(json, "name", parentFolder,
							textureTypeFilesTitle.at(stringToTextureType.at(json.at("type"))),
							textureTypeFilesFilter.at(stringToTextureType.at(json.at("type"))),
							[&json]
							{
								json.at("images")[0] = json.at("name") = nostd::normalize_path(json.at("name"));
							}
						);
					}
					ImGui::PopID();
				}
				break;
				case TextureType_Cube:
				{
					ImGui::PushID("texture-name");
					{
						ImGui::InputText("name", json.at("name").get_ptr<std::string*>());
					}
					ImGui::PopID();

					ImGui::PushID("texture-is-skybox");
					{
						ImGui::Checkbox("Skybox", &Texture::createCubeFromSkybox);
					}
					ImGui::PopID();

					ImGui::PushID("texture-ibl-diffuse");
					{
						ImGui::Checkbox("Diffuse IBL", &Texture::createDiffuseCubeIBL);
						ImGui::Checkbox("Specular IBL", &Texture::createSpecularCubeIBL);
					}
					ImGui::PopID();

					ImGui::PushID("texture-ibl-specular");
					{
						ImGui::Checkbox("Specular IBL", &Texture::createSpecularCubeIBL);
					}
					ImGui::PopID();

					ImGui::PushID("texture-brdf-lut");
					{
						ImGui::Checkbox("BRDF LUT IBL", &Texture::createBRDFCubeIBL);
					}
					ImGui::PopID();

					if (!Texture::createCubeFromSkybox)
					{
						for (unsigned int i = 0; i < cubeTextureAxesNames.size(); i++)
						{
							std::string ID = std::string("texture-image-") + cubeTextureAxesNames.at(i);
							ImGui::PushID(ID.c_str());
							{
								nlohmann::json jsonF = { {"file",json.at("images").at(i)} };
								ImDrawJsonFilePicker(jsonF,
									"file",
									parentFolder,
									textureTypeFilesTitle.at(stringToTextureType.at(json.at("type"))),
									textureTypeFilesFilter.at(stringToTextureType.at(json.at("type"))),
									[&jsonF, &json, i]
									{
										std::filesystem::path filePath = nostd::normalize_path(jsonF.at("file"));
										Texture::filePickingDirectory = filePath.relative_path().remove_filename();
										json.at("images")[i] = filePath.string();
									}
								, cubeTextureAxesNames.at(i));
							}
							ImGui::PopID();
						}
					}
					else
					{
						std::string ID = std::string("texture-image-skybox");
						ImGui::PushID(ID.c_str());
						{
							nlohmann::json jsonF = { {"file",json.at("images").at(0)} };
							ImDrawJsonFilePicker(jsonF,
								"file",
								parentFolder,
								textureTypeFilesTitle.at(stringToTextureType.at(json.at("type"))),
								textureTypeFilesFilter.at(stringToTextureType.at(json.at("type"))),
								[&jsonF, &json]
								{
									std::filesystem::path filePath = nostd::normalize_path(jsonF.at("file"));
									Texture::filePickingDirectory = filePath.relative_path().remove_filename();
									json.at("images")[0] = filePath.string();
									json.at("images")[1] = filePath.string();
									json.at("images")[2] = filePath.string();
									json.at("images")[3] = filePath.string();
									json.at("images")[4] = filePath.string();
									json.at("images")[5] = filePath.string();
								}
							, "Skybox");
						}
						ImGui::PopID();
					}
				}
				break;
				}

				ImGui::PushID("texture-format");
				{
					ImGui::Text("Format");
					drawFromCombo(json, "format", stringToDxgiFormat);
				}
				ImGui::PopID();

				if (ImGui::Button("Cancel"))
				{
					ResetTexturePopupParameters();
					ResetTexturePopupIBLParameters();
					OnCancel();
				}

				ImGui::SameLine();

				bool disabledCreate = false;
				if (json.at("name") == "") disabledCreate = true;
				if (textureType == TextureType_Cube)
				{
					for (unsigned int i = 0; i < 6U; i++)
					{
						if (json.at("images")[i] == "")
						{
							disabledCreate = true;
							break;
						}
					}
				}

				if (disabledCreate)
				{
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}

				if (ImGui::Button("Create"))
				{
					if (!json.at("name").empty())
					{
						using namespace Utils;

						if (textureType == TextureType_Cube)
						{
							std::filesystem::path newPath = json.at("images").at(0);
							newPath = newPath.parent_path();
							json.at("name") = nostd::normalize_path(newPath.string() + "/" + std::string(json.at("name")));
							Texture::createCubeUUID = json.at("uuid");
						}

						CreateTexture(json);
						CreateDDSFile(json, true);
						ResetTexturePopupParameters();
					}
				}

				if (disabledCreate)
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}
			}
		);
	}

	bool TexturesPopupIsOpen()
	{
		return !!Texture::popupModalId;
	}

#endif

	std::map<TextureShaderUsage, std::shared_ptr<TextureInstance>> GetTextures(const std::map<TextureShaderUsage, std::string>& textures)
	{
		std::map<TextureShaderUsage, std::shared_ptr<TextureInstance>> texInstances;

		std::transform(textures.begin(), textures.end(), std::inserter(texInstances, texInstances.end()), [](auto pair)
			{
				std::string& texture = pair.second;
				std::shared_ptr<TextureInstance> instance = refTracker.AddRef(texture, [&texture]()
					{
						using namespace Templates;
						std::shared_ptr<TextureInstance> instance = std::make_shared<TextureInstance>();
						instance->Load(texture);
						return instance;
					}
				);
				return std::pair<TextureShaderUsage, std::shared_ptr<TextureInstance>>(pair.first, instance);
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
		if (Texture::popupModalId) return;

		std::vector<std::shared_ptr<TextureInstance>> texturesToRelease;
		std::vector<std::shared_ptr<TextureInstance>> texturesToLoad;
		bool destroyPreview = false;
		refTracker.ForEach([&texturesToRelease, &texturesToLoad](std::shared_ptr<TextureInstance> texture)
			{
				if (texture->updateFlag & TextureUpdateFlags_Release) { texturesToRelease.push_back(texture); }
				if (texture->updateFlag & TextureUpdateFlags_Load) { texturesToLoad.push_back(texture); }
			}
		);
		if (texturePreview)
		{
			if (texturePreview->updateFlag & TextureUpdateFlags_Release) { texturesToRelease.push_back(texturePreview); destroyPreview = true; }
			if (texturePreview->updateFlag & TextureUpdateFlags_Load) { texturesToLoad.push_back(texturePreview); destroyPreview = false; }
		}

		if (texturesToRelease.size() == 0ULL && texturesToLoad.size() == 0ULL && !Texture::createDiffuseCubeIBL && !Texture::createSpecularCubeIBL && !Texture::createBRDFCubeIBL) return;

		std::shared_ptr<DiffuseIrradianceMap> iblDiffuseIrradianceMap = nullptr;
		std::shared_ptr<PreFilteredEnvironmentMap> iblPreFilteredEnvironmentMap = nullptr;
		std::shared_ptr<BRDFLUT> iblBRDFLUT = nullptr;

		renderer->Flush();
		renderer->RenderCriticalFrame([texturesToRelease, &texturesToLoad, destroyPreview, &iblDiffuseIrradianceMap, &iblPreFilteredEnvironmentMap, &iblBRDFLUT]
			{
				if (Texture::createDiffuseCubeIBL)
				{
					TextureTemplate& envMap = GetTextureTemplates().at(Texture::createCubeUUID);
					std::string iblDiffuseMap = std::get<0>(envMap) + "_irradiance.dds";
					std::filesystem::path iblDiffuseMapPath = iblDiffuseMap;

					iblDiffuseIrradianceMap = std::make_shared<DiffuseIrradianceMap>(Texture::createCubeUUID, iblDiffuseMapPath);
					iblDiffuseIrradianceMap->Compute();
				}

				if (Texture::createSpecularCubeIBL)
				{
					TextureTemplate& envMap = GetTextureTemplates().at(Texture::createCubeUUID);
					std::string iblPrefilteredEnvMap = std::get<0>(envMap) + "_prefiltered_env.dds";
					std::filesystem::path iblPrefilteredEnvMapPath = iblPrefilteredEnvMap;

					iblPreFilteredEnvironmentMap = std::make_shared<PreFilteredEnvironmentMap>(Texture::createCubeUUID, iblPrefilteredEnvMapPath);
					iblPreFilteredEnvironmentMap->Compute();
				}

				if (Texture::createBRDFCubeIBL)
				{
					TextureTemplate& envMap = GetTextureTemplates().at(Texture::createCubeUUID);
					std::string iblLUT = std::get<0>(envMap) + "_brdf_lut.dds";
					std::filesystem::path iblLUTPath = iblLUT;

					iblBRDFLUT = std::make_shared<BRDFLUT>(iblLUTPath);
					iblBRDFLUT->Compute();
				}

				std::for_each(texturesToRelease.begin(), texturesToRelease.end(), [](std::shared_ptr<TextureInstance> texture)
					{
						texture->ReleaseResources();
						texture->updateFlag &= ~TextureUpdateFlags_Release;
					}
				);
				if (destroyPreview) { texturePreview = nullptr; }
				std::for_each(texturesToLoad.begin(), texturesToLoad.end(), [](std::shared_ptr<TextureInstance> texture)
					{
						nlohmann::json& json = GetTextureTemplate(texture->materialTexture);
						if (texture != texturePreview)
						{
							texture->Load(texture->materialTexture);
						}
						else
						{
							texture->Load(texture->materialTexture, Texture::imageFrame - 1);
						}
						texture->updateFlag &= ~TextureUpdateFlags_Load;
						std::for_each(texture->onChangeCallbacks.begin(), texture->onChangeCallbacks.end(), [](auto& pair)
							{
								pair.second();
							}
						);
					}
				);
			}
		);

		bool resetIBL = false;
		if (iblDiffuseIrradianceMap)
		{
			resetIBL = true;
			iblDiffuseIrradianceMap->Solution();

			nlohmann::json iblDiffuseJson = CreateBaseTextureJson(
				iblDiffuseIrradianceMap->outputFile.string(),
				iblDiffuseIrradianceMap->numFaces,
				iblDiffuseIrradianceMap->dataFormat);
			iblDiffuseJson.at("height") = iblDiffuseIrradianceMap->faceHeight;
			iblDiffuseJson.at("width") = iblDiffuseIrradianceMap->faceWidth;
			iblDiffuseJson.at("images").push_back(iblDiffuseIrradianceMap->envMap->textureName);
			iblDiffuseJson.at("images").push_back(iblDiffuseIrradianceMap->envMap->textureName);
			iblDiffuseJson.at("images").push_back(iblDiffuseIrradianceMap->envMap->textureName);
			iblDiffuseJson.at("images").push_back(iblDiffuseIrradianceMap->envMap->textureName);
			iblDiffuseJson.at("images").push_back(iblDiffuseIrradianceMap->envMap->textureName);
			iblDiffuseJson.at("images").push_back(iblDiffuseIrradianceMap->envMap->textureName);
			iblDiffuseJson.at("mipLevels") = 1;
			iblDiffuseJson.at("type") = textureTypeToString.at(TextureType_Cube);

			CreateTexture(iblDiffuseJson);

			iblDiffuseIrradianceMap = nullptr;
		}

		if (iblPreFilteredEnvironmentMap)
		{
			resetIBL = true;
			iblPreFilteredEnvironmentMap->Solution();

			nlohmann::json iblPreFilteredJson = CreateBaseTextureJson(
				iblPreFilteredEnvironmentMap->outputFile.string(),
				iblPreFilteredEnvironmentMap->numFaces,
				iblPreFilteredEnvironmentMap->dataFormat);
			iblPreFilteredJson.at("height") = iblPreFilteredEnvironmentMap->faceHeight;
			iblPreFilteredJson.at("width") = iblPreFilteredEnvironmentMap->faceWidth;
			iblPreFilteredJson.at("images").push_back(iblPreFilteredEnvironmentMap->envMap->textureName);
			iblPreFilteredJson.at("images").push_back(iblPreFilteredEnvironmentMap->envMap->textureName);
			iblPreFilteredJson.at("images").push_back(iblPreFilteredEnvironmentMap->envMap->textureName);
			iblPreFilteredJson.at("images").push_back(iblPreFilteredEnvironmentMap->envMap->textureName);
			iblPreFilteredJson.at("images").push_back(iblPreFilteredEnvironmentMap->envMap->textureName);
			iblPreFilteredJson.at("images").push_back(iblPreFilteredEnvironmentMap->envMap->textureName);
			iblPreFilteredJson.at("mipLevels") = iblPreFilteredEnvironmentMap->numMipMaps;
			iblPreFilteredJson.at("type") = textureTypeToString.at(TextureType_Cube);

			CreateTexture(iblPreFilteredJson);

			iblPreFilteredEnvironmentMap = nullptr;
		}

		if (iblBRDFLUT)
		{
			resetIBL = true;
			iblBRDFLUT->Solution();

			nlohmann::json iblBRDFLUTJson = CreateBaseTextureJson(iblBRDFLUT->outputFile.string(), 1U, iblBRDFLUT->dataFormat);
			iblBRDFLUTJson["images"] = nlohmann::json::array({ "" });

			CreateTexture(iblBRDFLUTJson);

			iblBRDFLUT = nullptr;
		}

		if (resetIBL)
		{
			ResetTexturePopupParameters();
			ResetTexturePopupIBLParameters();
		}
	}

	void SetSelectedTexture(std::string uuid)
	{
		texturePreview = std::make_shared<TextureInstance>();
		texturePreview->Load(uuid);
	}

	void DeSelectTexture()
	{
		texturePreview->updateFlag |= TextureUpdateFlags_Release;
		Texture::filePickingDirectory = defaultAssetsFolder;
	}

	TextureInstance::~TextureInstance()
	{
		onChangeCallbacks.clear();
	}

	void TextureInstance::Load(std::string uuid, unsigned int startFrame)
	{
		using namespace Templates;
		std::filesystem::path path = GetTextureName(uuid);
		path.replace_extension(".dds");
#if defined(_DEVELOPMENT)
		if (!std::filesystem::exists(path))
		{
			RebuildTexture(uuid);
		}
#endif
		std::string pathS = path.string();
		materialTexture = uuid;
		nlohmann::json& json = GetTextureTemplate(uuid);
		DXGI_FORMAT format = stringToDxgiFormat.at(json.at("format"));
		unsigned int numFrames = json.at("numFrames");
		unsigned int nMipMaps = json.at("mipLevels");
		CreateTextureResource(pathS, format, numFrames, nMipMaps, startFrame);
	}

	void TextureInstance::CreateTextureResource(std::string& path, DXGI_FORMAT format, unsigned int numFrames, unsigned int nMipMaps, unsigned int firstArraySlice)
	{
		using namespace DeviceUtils;

		auto& d3dDevice = renderer->d3dDevice;
		auto& commandList = renderer->commandList;

		//Load the dds file to a buffer using LoadDDSTextureFromFile
		std::unique_ptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;

		DX::ThrowIfFailed(LoadDDSTextureFromFileEx(
			d3dDevice,
			nostd::StringToWString(path).c_str(),
			0,
			D3D12_RESOURCE_FLAG_NONE,
			nonLinearDxgiFormats.contains(format) ? DDS_LOADER_FORCE_SRGB : DDS_LOADER_IGNORE_SRGB,
			&texture,
			ddsData,
			subresources));

		//get the ammount of memory required for the upload
		bufferSize = GetRequiredIntermediateSize(texture, 0, static_cast<unsigned int>(subresources.size()));

		//create the upload texture
		CD3DX12_HEAP_PROPERTIES heapTypeUpload(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(&heapTypeUpload, D3D12_HEAP_FLAG_NONE, &uploadBufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload)));

		CCNAME_D3D12_OBJECT_N(texture, std::string(path + ":texture"));
		LogCComPtrAddress(std::string(path + ":texture"), texture);
		CCNAME_D3D12_OBJECT_N(upload, std::string(path + ":upload"));
		LogCComPtrAddress(std::string(path + ":upload"), upload);

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
			//viewDesc.Texture2DArray.ArraySize = numFrames;
			viewDesc.Texture2DArray.ArraySize = -1;
			viewDesc.Texture2DArray.FirstArraySlice = firstArraySlice;
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

	void TextureInstance::BindChangeCallback(std::string uuid, std::function<void()> cb)
	{
		onChangeCallbacks.insert_or_assign(uuid, cb);
	}

}