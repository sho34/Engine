#pragma once
#include <string>
#include <tuple>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
#include <imgui.h>
#include "../TemplateDecl.h"

//#define TEMPDECL_CREATE(TemplateName) void Create##TemplateName(nlohmann::json json)
//#define TEMPDECL_GET(TemplateName) nlohmann::json Get##TemplateName##Template(std::string uuid)
//#define TEMPDECL_GETUUIDNAMES(TemplateName) std::vector<UUIDName> Get##TemplateName##sUUIDsNames()
//#define TEMPDECL_GETNAMES(TemplateName) std::vector<std::string> Get##TemplateName##sNames()
//#define TEMPDECL_GETNAME(TemplateName) std::string Get##TemplateName##Name(std::string uuid)
//#define TEMPDECL_FINDUUIDBYNAME(TemplateName) std::string Find##TemplateName##UUIDByName(std::string name)

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
	}

	TEMPDECL_FULL(Texture);
	void CreateTexturesTemplatesFromMaterial(nlohmann::json json);

	//DESTROY
	void DestroyTexture(std::string uuid);
	void ReleaseTexturesTemplates();

#if defined(_EDITOR)
	void DrawTexturePanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop);
	void CreateNewTexture();
	void DeleteTexture(std::string uuid);
	void DrawTexturesPopups();
	void WriteTexturesJson(nlohmann::json& json);
#endif
};

