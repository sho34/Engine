#include "pch.h"
#include "Texture.h"
#include <UUID.h>
#include <DXTypes.h>
#include <NoStd.h>
#include <DDSTextureLoader.h>
#include <Templates.h>
#include <TemplateDef.h>
#include <Renderer.h>
#include <DirectXHelper.h>
#include "../Utils/ImageConvert.h"
#include <IBL/DiffuseIrradianceMap.h>
#include <IBL/PrefilteredEnvironmentMap.h>
#include <IBL/BRDFLUT.h>
#include <ShaderMaterials.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <algorithm>

extern std::shared_ptr<Renderer> renderer;

namespace Editor {
	void MarkTemplatesPanelAssetsAsDirty();
};

namespace Templates
{
#include <Editor/JDrawersDef.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <TextureAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
	std::shared_ptr<TextureInstance> texturePreview = nullptr;
#endif

	TextureJson::TextureJson(nlohmann::json json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <TextureAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <TextureAtt.h>
#include <JEnd.h>
#include <NoMath.h>
	}

	TEMPDEF_FULL(Texture);
	TEMPDEF_REFTRACKER(Texture);

	void TextureJson::EditorPreview(size_t flags)
	{
		if (flags & (1 << Update_images))
		{
			previewFrame = 0;
			previewIsPlaying = false;
			previewIsLooping = false;
			previewTime = 0.0f;
			previewTimeFactor = 1.0f;
			CreatePreviewTexture();
		}
	}

	void TextureJson::DestroyEditorPreview()
	{
		if (preview)
		{
			auto key = refTracker.FindKey(preview);
			refTracker.RemoveRef(key, preview);
			preview = nullptr;
		}
	}

	void TextureJson::CreatePreviewTexture()
	{
		DestroyEditorPreview();
		preview = GetTextureInstance(uuid(), [this]
			{
				return std::make_shared<TextureInstance>(uuid(), previewFrame);
			}
		);
		reloadPreview = false;
	}

#if defined(_EDITOR)
	void TextureJsonsStep()
	{
		std::set<std::shared_ptr<TextureJson>> texs;
		std::transform(Texturetemplates.begin(), Texturetemplates.end(), std::inserter(texs, texs.begin()), [](auto& temps)
			{
				auto& texJ = std::get<1>(temps.second);
				return texJ;
			}
		);

		std::set<std::shared_ptr<TextureJson>> rebuildImages;
		std::copy_if(texs.begin(), texs.end(), std::inserter(rebuildImages, rebuildImages.begin()), [](auto& tex)
			{
				return tex->dirty(TextureJson::Update_images);
			}
		);
		std::set<std::shared_ptr<TextureJson>> changedAttributes;
		std::copy_if(texs.begin(), texs.end(), std::inserter(changedAttributes, changedAttributes.begin()), [](auto& tex)
			{
				return tex->dirty(TextureJson::Update_format) || tex->dirty(TextureJson::Update_width) ||
					tex->dirty(TextureJson::Update_height) || tex->dirty(TextureJson::Update_mipLevels) ||
					tex->dirty(TextureJson::Update_numFrames);
			}
		);

		bool criticalFrame = rebuildImages.size() > 0ULL || changedAttributes.size() > 0ULL;

		if (criticalFrame)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([&rebuildImages, &changedAttributes]
				{
					std::for_each(rebuildImages.begin(), rebuildImages.end(), [](auto tex)
						{
							CreateDDSFile(tex);
							tex->CreatePreviewTexture();
							tex->clean(TextureJson::Update_images);
						}
					);
					std::for_each(changedAttributes.begin(), changedAttributes.end(), [](auto tex)
						{
							using namespace Utils;
							std::filesystem::path ddsPath = tex->name();
							ddsPath.replace_extension(".dds");
							ImageConverter convert = {
								.src = tex->name(), .dst = ddsPath, .format = tex->format(),
								.width = tex->width(), .height = tex->height(),
								//.mipLevels = tex->mipLevels(), .numFrames = tex->numFrames(),
							};
							if (tex->dirty(TextureJson::Update_mipLevels)) convert.mipLevels = tex->mipLevels();
							if (tex->dirty(TextureJson::Update_numFrames)) convert.numFrames = tex->numFrames();

							ConvertToDDS(convert);
							tex->format(convert.format);
							tex->width(convert.width);
							tex->height(convert.height);
							tex->mipLevels(convert.mipLevels);
							tex->numFrames(convert.numFrames);
							tex->type(convert.type);

							tex->clean(TextureJson::Update_format);
							tex->clean(TextureJson::Update_width);
							tex->clean(TextureJson::Update_height);
							tex->clean(TextureJson::Update_mipLevels);
							tex->clean(TextureJson::Update_numFrames);
							tex->CreatePreviewTexture();
						}
					);
				}
			);
		}
	}

#endif

	void Create2DDDSFile(TextureJson& json)
	{
		using namespace Utils;

		std::filesystem::path ddsPath = json.name();
		ddsPath.replace_extension(".dds");

		std::filesystem::path image = json.images().at(0);
		std::string extension = image.extension().string();
		nostd::ToLower(extension);

		DirectX::TexMetadata info{};
		GetImageAttributes(image, info);

		ImageConverter conv;
		conv.src = image;
		conv.dst = ddsPath;
		conv.format = info.format;
		conv.width = (!IsPowerOfTwo(static_cast<unsigned int>(info.width))) ? PrevPowerOfTwo(static_cast<unsigned int>(info.width)) : static_cast<unsigned int>(info.width);
		conv.height = (!IsPowerOfTwo(static_cast<unsigned int>(info.height))) ? PrevPowerOfTwo(static_cast<unsigned int>(info.height)) : static_cast<unsigned int>(info.height);
		conv.mipLevels = GetMipMaps(static_cast<unsigned int>(conv.width), static_cast<unsigned int>(conv.height));
		ConvertToDDS(conv);

		json.numFrames(conv.numFrames);
		json.format(conv.format);
		json.width(conv.width);
		json.height(conv.height);
		json.mipLevels(conv.mipLevels);
	}

	void CreateArrayDDSFile(TextureJson& json)
	{
		using namespace Utils;

		std::filesystem::path ddsPath = json.name();
		ddsPath.replace_extension(".dds");
		std::filesystem::path image = json.images().at(0);

		//convert the gif to dds
		AssembleArrayDDSFromGif(ddsPath, image);

		//get the dds attributes
		DirectX::TexMetadata info{};
		GetImageAttributes(ddsPath, info);

		//prepare the dds conversion to calculate sizes and mipmaps
		ImageConverter conv;
		conv.src = ddsPath;
		conv.dst = ddsPath;
		conv.format = info.format;
		conv.width = (!IsPowerOfTwo(static_cast<unsigned int>(info.width))) ? PrevPowerOfTwo(static_cast<unsigned int>(info.width)) : static_cast<unsigned int>(info.width);
		conv.height = (!IsPowerOfTwo(static_cast<unsigned int>(info.height))) ? PrevPowerOfTwo(static_cast<unsigned int>(info.height)) : static_cast<unsigned int>(info.height);
		conv.mipLevels = GetMipMaps(static_cast<unsigned int>(conv.width), static_cast<unsigned int>(conv.height));

		//only apply dds to dds conversion if the sizes are not matching
		if (conv.width != static_cast<unsigned int>(info.width) || conv.height != static_cast<unsigned int>(info.height))
			ConvertToDDS(conv);

		json.numFrames(conv.numFrames);
		json.format(conv.format);
		json.width(conv.width);
		json.height(conv.height);
		json.mipLevels(conv.mipLevels);
	}

	void CreateCubeDDSFile(TextureJson& json)
	{
		using namespace Utils;

		std::filesystem::path ddsPath = json.name();
		ddsPath.replace_extension(".dds");

		unsigned int minWidth;
		unsigned int minHeight;
		std::vector<std::string> facesPath = json.images();

		for (unsigned int i = 0; i < facesPath.size(); i++)
		{
			DirectX::TexMetadata info{};
			GetImageAttributes(facesPath[i], info);
			if (i == 0)
			{
				minWidth = static_cast<unsigned int>(info.width);
				minHeight = static_cast<unsigned int>(info.height);
			}
			else
			{
				minWidth = std::min(minWidth, static_cast<unsigned int>(info.width));
				minHeight = std::min(minHeight, static_cast<unsigned int>(info.height));
			}
		}

		AssembleCubeDDS(ddsPath, facesPath, minWidth, minHeight);

		//get the dds attributes
		DirectX::TexMetadata info{};
		GetImageAttributes(ddsPath, info);

		json.numFrames(static_cast<unsigned int>(info.depth));
		json.format(info.format);
		json.width(static_cast<unsigned int>(info.width));
		json.height(static_cast<unsigned int>(info.height));
		json.mipLevels(static_cast<unsigned int>(info.mipLevels));
	}

	void CreateCubeDDSFileFromSkyBox(TextureJson& json)
	{
		using namespace Utils;

		std::filesystem::path ddsPath = json.name();
		ddsPath.replace_extension(".dds");
		std::string image = json.images().at(0);

		AssembleCubeDDSFromSkybox(ddsPath, image);

		//get the dds attributes
		DirectX::TexMetadata info{};
		GetImageAttributes(ddsPath, info);

		if (!IsPowerOfTwo(static_cast<unsigned int>(info.width)) || !IsPowerOfTwo(static_cast<unsigned int>(info.height)))
		{
			//prepare the dds conversion to calculate sizes and mipmaps
			ImageConverter conv;
			conv.src = ddsPath;
			conv.dst = ddsPath;
			conv.format = info.format;
			conv.width = (!IsPowerOfTwo(static_cast<unsigned int>(info.width))) ? PrevPowerOfTwo(static_cast<unsigned int>(info.width)) : static_cast<unsigned int>(info.width);
			conv.height = (!IsPowerOfTwo(static_cast<unsigned int>(info.height))) ? PrevPowerOfTwo(static_cast<unsigned int>(info.height)) : static_cast<unsigned int>(info.height);
			conv.mipLevels = GetMipMaps(static_cast<unsigned int>(conv.width), static_cast<unsigned int>(conv.height));
			ConvertToDDS(conv);
			GetImageAttributes(ddsPath, info);
		}

		json.numFrames(static_cast<unsigned int>(info.depth));
		json.format(info.format);
		json.width(static_cast<unsigned int>(info.width));
		json.height(static_cast<unsigned int>(info.height));
		json.mipLevels(static_cast<unsigned int>(info.mipLevels));
	}

	TextureJson CreateBaseTextureJson(std::string name, unsigned int numFrames, DXGI_FORMAT format)
	{
		return TextureJson(nlohmann::json(
			{
			{ "uuid", getUUID() },
			{ "name", name },
			{ "images", { name } },
			{ "numFrames", numFrames },
			{ "format", DXGI_FORMATToString.at(format) },
			{ "type", TextureTypeToString.at(TextureType_2D) },
			{ "width", 128 },
			{ "height", 128 },
			{ "mipLevels", 1 }
			}
		));
	}

	void CreateTextureFromJsonDefinition(nlohmann::json json)
	{
		TextureJson texJson(json);
		switch (texJson.type())
		{
		case TextureType_2D:
		{
			Create2DDDSFile(texJson);
		}
		break;
		case TextureType_Array:
		{
			CreateArrayDDSFile(texJson);
		}
		break;
		case TextureType_Cube:
		{
			CreateCubeDDSFile(texJson);
		}
		break;
		case TextureType_Skybox:
		{
			CreateCubeDDSFileFromSkyBox(texJson);
		}
		break;
		}

		CreateTexture(texJson.json());
		Editor::MarkTemplatesPanelAssetsAsDirty();
	}

	std::string CreateTextureTemplate(std::string name, DXGI_FORMAT format)
	{
		//used for creating
		TextureJson texj = CreateBaseTextureJson(name, 0, format);
		CreateTexture(texj);

		//used for referencing
		std::shared_ptr<TextureJson> text = GetTextureTemplate(texj.uuid());
		CreateDDSFile(text);
		return text->uuid();
	}

	void CreateDDSFile(std::shared_ptr<TextureJson>& tex)
	{

		std::filesystem::path ddsPath = tex->name();
		ddsPath.replace_extension(".dds");

		if (!std::filesystem::exists(ddsPath))
		{
			switch (tex->type())
			{
			case TextureType_2D:
			{
				Create2DDDSFile(*tex);
			}
			break;
			case TextureType_Array:
			{
				CreateArrayDDSFile(*tex);
			}
			break;
			case TextureType_Cube:
			{
				CreateCubeDDSFile(*tex);
			}
			break;
			case TextureType_Skybox:
			{
				CreateCubeDDSFileFromSkyBox(*tex);
			}
			break;
			}
		}
		Editor::MarkTemplatesPanelAssetsAsDirty();

		/*

			ImageConverter convert = {
				.src = tex->name(),
				.dst = ddsPath
			};
			ConvertToDDS(convert);
		*/

		/*
		DirectX::TexMetadata info{};
		GetImageAttributes(ddsPath, info);
		tex->width(static_cast<unsigned int>(info.width));
		tex->height(static_cast<unsigned int>(info.height));
		tex->format(info.format);
		tex->mipLevels(static_cast<unsigned int>(info.mipLevels));
		tex->numFrames(static_cast<unsigned int>(info.arraySize));
		tex->type(info.IsCubemap() ? (TextureType_Cube) : (info.arraySize > 1ULL) ? TextureType_Array : TextureType_2D);
		*/
	}

	void PreviewTexturesStep(float elapsedSeconds)
	{
		std::vector<std::shared_ptr<TextureJson>> previewsToPlay;

		for (auto& [uuid, textureTemplate] : Texturetemplates)
		{
			std::shared_ptr<TextureJson>& tex = std::get<1>(textureTemplate);
			if (!tex->preview || !tex->previewIsPlaying || tex->numFrames() <= 1) continue;

			previewsToPlay.push_back(tex);
		}

		auto previewStep = [elapsedSeconds](auto& texture)
			{
				float animationLength = texture->numFrames() * (1.0f / 60.0f);
				float currentAnimationTime = texture->previewTime;
				unsigned int currentFrame = static_cast<unsigned int>(texture->numFrames() * (currentAnimationTime / animationLength));

				if (animationLength > 0.0f)
				{
					currentAnimationTime += (texture->previewIsPlaying) ? texture->previewTimeFactor * elapsedSeconds : 0.0f;
					if (texture->previewTimeFactor > 0.0f)
					{
						if (currentAnimationTime >= animationLength)
							currentAnimationTime = (texture->previewIsLooping) ? fmodf(currentAnimationTime, animationLength) : animationLength;
					}
					else if (texture->previewTimeFactor < 0.0f)
					{
						if (currentAnimationTime < 0.0f)
							currentAnimationTime = (texture->previewIsLooping) ? (animationLength - fmodf(currentAnimationTime, animationLength)) : 0.0f;
					}
					texture->previewTime = currentAnimationTime;
					unsigned int newFrame = static_cast<unsigned int>(texture->numFrames() * (currentAnimationTime / animationLength));
					if (currentFrame != newFrame)
					{
						texture->previewFrame = std::clamp(newFrame, 0U, texture->numFrames() - 1);
						texture->reloadPreview = true;
					}
				}
			};

		std::for_each(previewsToPlay.begin(), previewsToPlay.end(), previewStep);
	}

	void ReloadPreviewTextures()
	{
		std::vector<std::shared_ptr<TextureJson>> previewsToReload;

		for (auto& [uuid, textureTemplate] : Texturetemplates)
		{
			std::shared_ptr<TextureJson>& tex = std::get<1>(textureTemplate);
			if (!tex->preview || !tex->reloadPreview) continue;

			previewsToReload.push_back(tex);
		}

		if (previewsToReload.empty()) return;

		renderer->Flush();
		renderer->RenderCriticalFrame([&previewsToReload]
			{
				for (auto& tex : previewsToReload)
				{
					tex->CreatePreviewTexture();
				}
			}
		);
	}

	/*
	void ReleaseTexturesTemplates()
	{
	}
	*/

	/*
	std::map<TextureShaderUsage, std::shared_ptr<TextureInstance>> GetTextures(const std::map<TextureShaderUsage, std::string>& textures)
	{
		//std::map<TextureShaderUsage, std::shared_ptr<TextureInstance>> texInstances;
		//
		//std::transform(textures.begin(), textures.end(), std::inserter(texInstances, texInstances.end()), [](auto pair)
		//	{
		//		std::string& texture = pair.second;
		//		std::shared_ptr<TextureInstance> instance = refTracker.AddRef(texture, [&texture]()
		//			{
		//				using namespace Templates;
		//				std::shared_ptr<TextureInstance> instance = std::make_shared<TextureInstance>();
		//				instance->Load(texture);
		//				return instance;
		//			}
		//		);
		//		return std::pair<TextureShaderUsage, std::shared_ptr<TextureInstance>>(pair.first, instance);
		//	}
		//);
		//
		//return texInstances;
	}
	*/

	/*
	std::shared_ptr<TextureInstance> GetTextureFromGPUHandle(const std::string& texture, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle)
	{
		//return refTracker.AddRef(texture, [texture, gpuHandle]()
		//	{
		//		std::shared_ptr<TextureInstance> instance = std::make_shared<TextureInstance>();
		//		instance->textureName = texture;
		//		instance->gpuHandle = gpuHandle;
		//		return instance;
		//	}
		//);
	}
	*/

	/*
	void DestroyTextureInstance(std::shared_ptr<TextureInstance>& texture)
	{
		//auto key = refTracker.FindKey(texture);
		//refTracker.RemoveRef(key, texture);
	}
	*/

	/*
	void ReloadTextureInstances()
	{
		//if (Texture::popupModalId) return;
		//
		//std::vector<std::shared_ptr<TextureInstance>> texturesToRelease;
		//std::vector<std::shared_ptr<TextureInstance>> texturesToLoad;
		//bool destroyPreview = false;
		//refTracker.ForEach([&texturesToRelease, &texturesToLoad](std::shared_ptr<TextureInstance> texture)
		//	{
		//		if (texture->updateFlag & TextureUpdateFlags_Release) { texturesToRelease.push_back(texture); }
		//		if (texture->updateFlag & TextureUpdateFlags_Load) { texturesToLoad.push_back(texture); }
		//	}
		//);
		//if (texturePreview)
		//{
		//	if (texturePreview->updateFlag & TextureUpdateFlags_Release) { texturesToRelease.push_back(texturePreview); destroyPreview = true; }
		//	if (texturePreview->updateFlag & TextureUpdateFlags_Load) { texturesToLoad.push_back(texturePreview); destroyPreview = false; }
		//}
		//
		//if (texturesToRelease.size() == 0ULL && texturesToLoad.size() == 0ULL && !Texture::createDiffuseCubeIBL && !Texture::createSpecularCubeIBL && !Texture::createBRDFCubeIBL) return;
		//
		//using namespace ComputeShader;
		//std::shared_ptr<DiffuseIrradianceMap> iblDiffuseIrradianceMap = nullptr;
		//std::shared_ptr<PreFilteredEnvironmentMap> iblPreFilteredEnvironmentMap = nullptr;
		//std::shared_ptr<BRDFLUT> iblBRDFLUT = nullptr;
		//
		//renderer->Flush();
		//renderer->RenderCriticalFrame([texturesToRelease, &texturesToLoad, destroyPreview, &iblDiffuseIrradianceMap, &iblPreFilteredEnvironmentMap, &iblBRDFLUT]
		//	{
		//		if (Texture::createDiffuseCubeIBL)
		//		{
		//			TextureTemplate& envMap = GetTextureTemplates().at(Texture::createCubeUUID);
		//			std::string iblDiffuseMap = std::get<0>(envMap) + "_irradiance.dds";
		//			std::filesystem::path iblDiffuseMapPath = iblDiffuseMap;
		//
		//			iblDiffuseIrradianceMap = std::make_shared<DiffuseIrradianceMap>(Texture::createCubeUUID, iblDiffuseMapPath);
		//			iblDiffuseIrradianceMap->Compute();
		//		}
		//
		//		if (Texture::createSpecularCubeIBL)
		//		{
		//			TextureTemplate& envMap = GetTextureTemplates().at(Texture::createCubeUUID);
		//			std::string iblPrefilteredEnvMap = std::get<0>(envMap) + "_prefiltered_env.dds";
		//			std::filesystem::path iblPrefilteredEnvMapPath = iblPrefilteredEnvMap;
		//
		//			iblPreFilteredEnvironmentMap = std::make_shared<PreFilteredEnvironmentMap>(Texture::createCubeUUID, iblPrefilteredEnvMapPath);
		//			iblPreFilteredEnvironmentMap->Compute();
		//		}
		//
		//		if (Texture::createBRDFCubeIBL)
		//		{
		//			TextureTemplate& envMap = GetTextureTemplates().at(Texture::createCubeUUID);
		//			std::string iblLUT = std::get<0>(envMap) + "_brdf_lut.dds";
		//			std::filesystem::path iblLUTPath = iblLUT;
		//
		//			iblBRDFLUT = std::make_shared<BRDFLUT>(iblLUTPath);
		//			iblBRDFLUT->Compute();
		//		}
		//
		//		std::for_each(texturesToRelease.begin(), texturesToRelease.end(), [](std::shared_ptr<TextureInstance> texture)
		//			{
		//				texture->ReleaseResources();
		//				texture->updateFlag &= ~TextureUpdateFlags_Release;
		//			}
		//		);
		//		if (destroyPreview) { texturePreview = nullptr; }
		//		std::for_each(texturesToLoad.begin(), texturesToLoad.end(), [](std::shared_ptr<TextureInstance> texture)
		//			{
		//				nlohmann::json& json = GetTextureTemplate(texture->materialTexture);
		//				if (texture != texturePreview)
		//				{
		//					texture->Load(texture->materialTexture);
		//				}
		//				else
		//				{
		//					texture->Load(texture->materialTexture, Texture::imageFrame - 1);
		//				}
		//				texture->updateFlag &= ~TextureUpdateFlags_Load;
		//				std::for_each(texture->onChangeCallbacks.begin(), texture->onChangeCallbacks.end(), [](auto& pair)
		//					{
		//						pair.second();
		//					}
		//				);
		//			}
		//		);
		//	}
		//);
		//
		//bool resetIBL = false;
		//if (iblDiffuseIrradianceMap)
		//{
		//	resetIBL = true;
		//	iblDiffuseIrradianceMap->Solution();
		//
		//	nlohmann::json iblDiffuseJson = CreateBaseTextureJson(
		//		iblDiffuseIrradianceMap->outputFile.string(),
		//		iblDiffuseIrradianceMap->numFaces,
		//		iblDiffuseIrradianceMap->dataFormat);
		//	iblDiffuseJson.at("height") = iblDiffuseIrradianceMap->faceHeight;
		//	iblDiffuseJson.at("width") = iblDiffuseIrradianceMap->faceWidth;
		//	iblDiffuseJson.at("images").push_back(iblDiffuseIrradianceMap->envMap->textureName);
		//	iblDiffuseJson.at("images").push_back(iblDiffuseIrradianceMap->envMap->textureName);
		//	iblDiffuseJson.at("images").push_back(iblDiffuseIrradianceMap->envMap->textureName);
		//	iblDiffuseJson.at("images").push_back(iblDiffuseIrradianceMap->envMap->textureName);
		//	iblDiffuseJson.at("images").push_back(iblDiffuseIrradianceMap->envMap->textureName);
		//	iblDiffuseJson.at("images").push_back(iblDiffuseIrradianceMap->envMap->textureName);
		//	iblDiffuseJson.at("mipLevels") = 1;
		//	iblDiffuseJson.at("type") = TextureTypeToString.at(TextureType_Cube);
		//
		//	CreateTexture(iblDiffuseJson);
		//
		//	iblDiffuseIrradianceMap = nullptr;
		//}
		//
		//if (iblPreFilteredEnvironmentMap)
		//{
		//	resetIBL = true;
		//	iblPreFilteredEnvironmentMap->Solution();
		//
		//	nlohmann::json iblPreFilteredJson = CreateBaseTextureJson(
		//		iblPreFilteredEnvironmentMap->outputFile.string(),
		//		iblPreFilteredEnvironmentMap->numFaces,
		//		iblPreFilteredEnvironmentMap->dataFormat);
		//	iblPreFilteredJson.at("height") = iblPreFilteredEnvironmentMap->faceHeight;
		//	iblPreFilteredJson.at("width") = iblPreFilteredEnvironmentMap->faceWidth;
		//	iblPreFilteredJson.at("images").push_back(iblPreFilteredEnvironmentMap->envMap->textureName);
		//	iblPreFilteredJson.at("images").push_back(iblPreFilteredEnvironmentMap->envMap->textureName);
		//	iblPreFilteredJson.at("images").push_back(iblPreFilteredEnvironmentMap->envMap->textureName);
		//	iblPreFilteredJson.at("images").push_back(iblPreFilteredEnvironmentMap->envMap->textureName);
		//	iblPreFilteredJson.at("images").push_back(iblPreFilteredEnvironmentMap->envMap->textureName);
		//	iblPreFilteredJson.at("images").push_back(iblPreFilteredEnvironmentMap->envMap->textureName);
		//	iblPreFilteredJson.at("mipLevels") = iblPreFilteredEnvironmentMap->numMipMaps;
		//	iblPreFilteredJson.at("type") = TextureTypeToString.at(TextureType_Cube);
		//
		//	CreateTexture(iblPreFilteredJson);
		//
		//	iblPreFilteredEnvironmentMap = nullptr;
		//}
		//
		//if (iblBRDFLUT)
		//{
		//	resetIBL = true;
		//	iblBRDFLUT->Solution();
		//
		//	nlohmann::json iblBRDFLUTJson = CreateBaseTextureJson(iblBRDFLUT->outputFile.string(), 1U, iblBRDFLUT->dataFormat);
		//	iblBRDFLUTJson["images"] = nlohmann::json::array({ "" });
		//
		//	CreateTexture(iblBRDFLUTJson);
		//
		//	iblBRDFLUT = nullptr;
		//}
		//
		//if (resetIBL)
		//{
		//	ResetTexturePopupParameters();
		//	ResetTexturePopupIBLParameters();
		//}
	}
	*/

	/*
	TextureInstance::~TextureInstance()
	{
		//onChangeCallbacks.clear();
	}
	*/

	/*
	void TextureInstance::BindChangeCallback(std::string uuid, std::function<void()> cb)
	{
		//onChangeCallbacks.insert_or_assign(uuid, cb);
	}
	*/

	TextureInstance::TextureInstance(std::string uuid) :
		TextureInstance(uuid, 0U) {
	}

	TextureInstance::TextureInstance(std::string uuid, unsigned int startFrame)
	{
		using namespace Templates;
		materialTexture = uuid;
		std::shared_ptr<TextureJson> tex = GetTextureTemplate(uuid);
		std::filesystem::path path = tex->name();
		path.replace_extension(".dds");
#if defined(_DEVELOPMENT)
		if (!std::filesystem::exists(path))
		{
			CreateDDSFile(tex);
		}
#endif
		std::string pathS = path.string();
		CreateTextureResource(pathS, tex->format(), tex->type(), tex->numFrames(), tex->mipLevels(), startFrame);
	}

	void TextureInstance::CreateTextureResource(std::string& path, DXGI_FORMAT format, TextureType type, unsigned int numFrames, unsigned int nMipMaps, unsigned int firstArraySlice)
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
			NonLinearDxgiFormats.contains(format) ? DDS_LOADER_FORCE_SRGB : DDS_LOADER_IGNORE_SRGB,
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

		switch (type)
		{
		case TextureType_2D:
		{
			//simple static textures
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipLevels = nMipMaps;
			viewDesc.Texture2D.MostDetailedMip = 0;
			viewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			viewDesc.Texture2D.PlaneSlice = 0;
		}
		break;
		case TextureType_Array:
		{
			//array textures(animated gifs)
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			viewDesc.Texture2DArray.MipLevels = nMipMaps;
			viewDesc.Texture2DArray.MostDetailedMip = 0;
			viewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
			viewDesc.Texture2DArray.ArraySize = numFrames - firstArraySlice;
			viewDesc.Texture2DArray.PlaneSlice = 0;
			//viewDesc.Texture2DArray.ArraySize = -1;
			viewDesc.Texture2DArray.FirstArraySlice = firstArraySlice;
		}
		break;
		case TextureType_Cube:
		case TextureType_Skybox:
		{
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			viewDesc.TextureCube = {
				.MostDetailedMip = 0,
				.MipLevels = nMipMaps,
				.ResourceMinLODClamp = 0.0f
			};
		}
		break;
		}

		//allocate descriptors handles for the SRV and kick the resource creation
		AllocCSUDescriptor(cpuHandle, gpuHandle);
		renderer->d3dDevice->CreateShaderResourceView(texture, &viewDesc, cpuHandle);
	}

	void TextureInstance::ReleaseResources()
	{
		using namespace DeviceUtils;
		FreeCSUDescriptor(cpuHandle, gpuHandle);
		texture = nullptr;
		upload = nullptr;
	}
}
