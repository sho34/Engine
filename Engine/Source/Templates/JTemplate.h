#pragma once

#include <JObject.h>
#include <nlohmann/json.hpp>
#if defined(_EDITOR)
#include <IconsFontAwesome5.h>
#endif

enum TemplateType {
	T_None,
	T_Shaders,
	T_Materials,
	T_Models3D,
	T_Sounds,
	T_Textures,
	T_RenderPasses
};

inline const std::map<TemplateType, std::string> TemplateTypeToString = {
	{ T_Shaders, "Shaders" },
	{ T_Materials, "Materials" },
	{ T_Models3D, "Models3D" },
	{ T_Sounds, "Sounds" },
	{ T_Textures, "Textures" },
	{ T_RenderPasses, "RenderPasses"}
};

inline const std::map<std::string, TemplateType> StringToTemplateType = {
	{ "Shaders", T_Shaders },
	{ "Materials", T_Materials },
	{ "Models3D", T_Models3D },
	{ "Sounds", T_Sounds },
	{ "Textures", T_Textures },
	{ "RenderPasses", T_RenderPasses }
};

#if defined(_EDITOR)
inline const std::map<TemplateType, const char* > TemplateTypePanelMenuItems = {
	{ T_Shaders, ICON_FA_FILE "Shaders" },
	{ T_Materials, ICON_FA_TSHIRT "Materials" },
	{ T_Models3D, ICON_FA_CUBE "Models3D" },
	{ T_Sounds, ICON_FA_MUSIC "Sounds" },
	{ T_Textures, ICON_FA_IMAGE "Textures" },
	{ T_RenderPasses, ICON_FA_TV "RenderPasses"}
};
#endif

namespace Templates
{
	struct JTemplate : JObject
	{
		JTemplate(nlohmann::json json) :JObject(json) {}
		virtual TemplateType JType() { return T_None; }
	};
};