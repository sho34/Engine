#pragma once
#include <string>
#include <tuple>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
#include <imgui.h>
#include "../TemplateDecl.h"

namespace Templates
{
#if defined(_EDITOR)
	enum TexturePopupModal
	{
		TexturePopupModal_CannotDelete = 1,
		TexturePopupModal_CreateNew = 2
	};

	enum TextureUpdateFlags {
		TextureUpdateFlags_Load = 0x1,
		TextureUpdateFlags_Release = 0x2,
		TextureUpdateFlags_Reload = TextureUpdateFlags_Load | TextureUpdateFlags_Release,
	};
#endif

	TEMPDECL_FULL(Texture);

	namespace Texture
	{
		inline static const std::string templateName = "textures.json";
#if defined(_EDITOR)
		bool DrawEditorInformationAttributes(std::string uuid);
		bool DrawEditorAssetAttributes(std::string uuid);
		void RebuildInstance(std::string& uuid);
		void DrawEditorTexturePreview(std::string uuid);
#endif
	}

	void Create2DDDSFile(nlohmann::json json, bool overwrite);
	void CreateArrayDDSFile(nlohmann::json json, bool overwrite);
	void CreateCubeDDSFile(nlohmann::json json, bool overwrite);
	void CreateDDSFile(nlohmann::json& json, bool overwrite);
	void RebuildTexture(std::string uuid);
	void CreateTexturesTemplatesFromMaterial(nlohmann::json json);
	std::string CreateTextureTemplate(std::string name, DXGI_FORMAT format);

	//DESTROY
	//void DestroyTexture(std::string uuid);
	void ReleaseTexturesTemplates();

#if defined(_EDITOR)
	void DrawTexturePanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop);
	void CreateNewTexture();
	void ResetTexturePopupParameters();
	void ResetTexturePopupIBLParameters();
	void DeleteTexture(std::string uuid);
	void DrawTexturesPopups();
	bool TexturesPopupIsOpen();
#endif

	struct TextureInstance
	{
		std::string materialTexture;
		unsigned int updateFlag = 0U;

		//D3D12
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		CComPtr<ID3D12Resource> texture;
		CComPtr<ID3D12Resource> upload;
		std::string textureName;
		std::map<std::string, std::function<void()>> onChangeCallbacks;

		~TextureInstance();
		void Load(std::string& texture, DXGI_FORMAT format, unsigned int numFrames, unsigned int nMipMaps, unsigned int startFrame = 0U);
		void CreateTextureResource(std::string& path, DXGI_FORMAT format, unsigned int numFrames, unsigned int nMipMaps, unsigned int startFrame = 0U);
		void ReleaseResources();
		void BindChangeCallback(std::string uuid, std::function<void()> cb);
	};
	TEMPDECL_REFTRACKER(Texture);

	std::map<TextureShaderUsage, std::shared_ptr<TextureInstance>> GetTextures(const std::map<TextureShaderUsage, std::string>& textures);
	std::shared_ptr<TextureInstance> GetTextureFromGPUHandle(const std::string& texture, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle);
	void DestroyTextureInstance(std::shared_ptr<TextureInstance>& texture);
	void ReloadTextureInstances();

#if defined(_EDITOR)
	void SetSelectedTexture(std::string uuid);
	void DeSelectTexture();
#endif
};

