#pragma once
#include <string>
#include <tuple>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
#include <imgui.h>
#include <JTemplate.h>
#include <TemplateDecl.h>
#include <Json.h>
#include <JExposeTypes.h>

enum TextureType;

namespace Templates
{
#include <JExposeAttOrder.h>
#include <TextureAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttDrawersDecl.h>
#include <TextureAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttRequired.h>
#include <TextureAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttJsonDecl.h>
#include <TextureAtt.h>
#include <JExposeEnd.h>

	void TextureJsonsStep();

	struct TextureInstance;
	struct TextureJson : public JTemplate
	{
		TEMPLATE_DECL(Texture);

#include <JExposeAttFlags.h>
#include <TextureAtt.h>
#include <JExposeEnd.h>

#include <JExposeDecl.h>
#include <TextureAtt.h>
#include <JExposeEnd.h>

#if defined(_EDITOR)
		virtual void EditorPreview(size_t flags);
		virtual void DestroyEditorPreview();

		//load&reload
		std::shared_ptr<TextureInstance> preview;
		int previewFrame = 0U;
		bool reloadPreview = false;
		void CreatePreviewTexture();

		//controller for array(animated) textures
		bool previewIsPlaying = false;
		bool previewIsLooping = false;
		float previewTime = 0.0f;
		float previewTimeFactor = 1.0f;
#endif
	};

	TEMPDECL_FULL(Texture);

#if defined(_EDITOR)
	enum TexturePopupModal
	{
		TexturePopupModal_CannotDelete = 1,
		TexturePopupModal_CreateNew = 2
	};

	/*
	enum TextureUpdateFlags {
		TextureUpdateFlags_Load = 0x1,
		TextureUpdateFlags_Release = 0x2,
		TextureUpdateFlags_Reload = TextureUpdateFlags_Load | TextureUpdateFlags_Release,
	};
	*/
#endif

	namespace Texture
	{
		inline static const std::string templateName = "textures.json";
		//#if defined(_EDITOR)
		//		bool DrawEditorInformationAttributes(std::string uuid);
		//		bool DrawEditorAssetAttributes(std::string uuid);
		//		void RebuildInstance(std::string& uuid);
		//		void DrawEditorTexturePreview(std::string uuid);
		//#endif
	}

#if defined(_EDITOR)
	void ResetTexturePopupIBLParameters();
#endif

	//void Create2DDDSFile(TextureJson& json, bool overwrite);
	//void CreateArrayDDSFile(TextureJson& json, bool overwrite);
	//void CreateCubeDDSFile(TextureJson& json, bool overwrite);
	//void CreateDDSFile(TextureJson& json, bool overwrite);
	//void RebuildTexture(std::string uuid);
	//void CreateTexturesTemplatesFromMaterial(nlohmann::json json);
	std::string CreateTextureTemplate(std::string name, DXGI_FORMAT format);
	void CreateDDSFile(std::shared_ptr<TextureJson>& tex);

	//DESTROY
	//void DestroyTexture(std::string uuid);
	//void ReleaseTexturesTemplates();

	//std::map<TextureShaderUsage, std::shared_ptr<TextureInstance>> GetTextures(const std::map<TextureShaderUsage, std::string>& textures);
	//std::shared_ptr<TextureInstance> GetTextureFromGPUHandle(const std::string& texture, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle);
	//void DestroyTextureInstance(std::shared_ptr<TextureInstance>& texture);
	//void ReloadTextureInstances();

	void PreviewTexturesStep(float delta);
	void ReloadPreviewTextures();

	struct TextureInstance
	{
		TextureInstance(std::string uuid);
		TextureInstance(std::string uuid, unsigned int startFrame);
		~TextureInstance() {}
		std::string materialTexture;
		//unsigned int updateFlag = 0U;

		//D3D12
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		CComPtr<ID3D12Resource> texture;
		CComPtr<ID3D12Resource> upload;
		size_t bufferSize;
		//std::string textureName;
		//std::map<std::string, std::function<void()>> onChangeCallbacks;
		//
		//~TextureInstance();
		//void Load(std::string uuid, unsigned int startFrame = 0);
		void CreateTextureResource(std::string& path, DXGI_FORMAT format, TextureType type, unsigned int numFrames, unsigned int nMipMaps, unsigned int startFrame = 0U);
		void ReleaseResources();
		//void BindChangeCallback(std::string uuid, std::function<void()> cb);
	};
	TEMPDECL_REFTRACKER(Texture);
};

inline auto ToTextureJson(std::vector<std::shared_ptr<JObject>>& json)
{
	std::vector<std::shared_ptr<TextureJson>> textures;
	std::transform(json.begin(), json.end(), std::back_inserter(textures), [](auto& j)
		{
			return std::dynamic_pointer_cast<TextureJson>(j);
		}
	);
	return textures;
}
