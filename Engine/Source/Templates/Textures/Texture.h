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
#endif

	typedef std::tuple<
		std::string, //name
		nlohmann::json
	> TextureTemplate;
	TemplatesContainer<TextureTemplate>& GetTextureTemplates();

	namespace Texture
	{
		inline static const std::string templateName = "textures.json";
#if defined(_EDITOR)
		void DrawEditorInformationAttributes(std::string uuid);
		void DrawEditorAssetAttributes(std::string uuid);
		void DrawEditorTexturePreview(std::string uuid);
#endif
	}

	TEMPDECL_FULL(Texture);
	void CreateDDSFile(std::string uuid, std::filesystem::path path, DXGI_FORMAT format);
	void CreateTexturesTemplatesFromMaterial(nlohmann::json json);
	std::string CreateTextureTemplate(std::string name, DXGI_FORMAT format);

	//DESTROY
	void DestroyTexture(std::string uuid);
	void ReleaseTexturesTemplates();

#if defined(_EDITOR)
	void DrawTexturePanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop);
	void CreateNewTexture();
	void CreateDDSTexture(std::filesystem::path path);
	void DeleteTexture(std::string uuid);
	void DrawTexturesPopups();
	void WriteTexturesJson(nlohmann::json& json);
#endif

	struct TextureInstance
	{
		std::string materialTexture;

		//D3D12
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		CComPtr<ID3D12Resource> texture;
		CComPtr<ID3D12Resource> upload;
		std::string textureName;

		~TextureInstance() { Destroy(); }
		void Load(std::string& texture);
		void CreateTextureResource(std::filesystem::path path, DXGI_FORMAT format, unsigned int numFrames);
		void Destroy();
	};
};

