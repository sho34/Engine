#pragma once

#if defined(_EDITOR)
#include <imgui.h>
#endif
#include <vector>
#include <UUID.h>
#include <string>
#include <functional>
#include <nlohmann/json.hpp>
#include <StepTimer.h>

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

#if defined(_EDITOR)
inline static std::function<std::vector<UUIDName>()> SortUUIDNameByName(std::function<std::vector<UUIDName>()> getUUIDNames)
{
	return [getUUIDNames]()
		{
			std::vector<UUIDName> uuidNames = getUUIDNames();
			std::sort(uuidNames.begin(), uuidNames.end(), [](UUIDName a, UUIDName b)
				{
					return std::get<1>(a) < std::get<1>(b);
				}
			);
			return uuidNames;
		};
};

inline static void SortUUIDByName(std::vector<UUIDName>& uuidNames)
{
	std::sort(uuidNames.begin(), uuidNames.end(), [](UUIDName a, UUIDName b)
		{
			return std::get<1>(a) < std::get<1>(b);
		}
	);
}

inline auto selectTemplate(std::string from, std::string& to) { to = from; }
inline auto getX(std::string s) { return s; }
inline auto deSelectTemplate(std::string& s) { s = ""; }

#endif

namespace Templates {

	nlohmann::json& GetSystemShaders();
	nlohmann::json& GetSystemMaterials();
	nlohmann::json& GetSystemRenderPasses();

#if defined(_EDITOR)
	void SaveTemplates(const std::string folder, const std::string fileName, std::function<void(nlohmann::json&)> writer);
#endif

	void LoadTemplates(nlohmann::json templates, std::function<void(nlohmann::json)> loader);
	void LoadTemplates(const std::string folder, const std::string fileName, std::function<void(nlohmann::json)> loader);

	void DestroyTemplates();
#if defined(_EDITOR)
	void DestroyTemplatesReferences();
#endif
	void FreeGPUIntermediateResources();

	template<typename T, typename J>
	inline void CreateJsonTemplate(nlohmann::json json, auto getTemplates)
	{
		std::string uuid = json.at("uuid");

		auto& templates = getTemplates();

		if (templates.contains(uuid))
		{
			assert(!!!"creation collision");
		}

		std::shared_ptr<J> jT = std::make_shared<J>(json);

		templates.insert_or_assign(uuid, std::make_tuple(json.at("name"), jT));
	}

	inline std::string GetName(std::string uuid, auto getTemplates)
	{
		auto& templates = getTemplates();
		return std::get<0>(templates.at(uuid));
	}

	std::string FindUUIDByName(std::string name, auto getTemplates)
	{
		auto& templates = getTemplates();
		for (auto& [uuid, T] : templates)
		{
			if (std::get<0>(T) == name) return uuid;
		}

		return "";
	}

	inline std::vector<std::string> GetNames(auto& items)
	{
		std::vector<std::string> names;
		std::transform(items.begin(), items.end(), std::back_inserter(names), [](auto pair)
			{
				return std::get<0>(pair.second);
			}
		);
		return names;
	}

	inline std::vector<UUIDName> GetUUIDsNames(auto& items)
	{
		std::vector<std::tuple<std::string, std::string>> uuidsNames;
		std::transform(items.begin(), items.end(), std::back_inserter(uuidsNames), [](auto pair)
			{
				return std::make_tuple(pair.first, std::get<0>(pair.second));
			}
		);
		return uuidsNames;
	}

	template<typename T>
	void WriteTemplateJson(nlohmann::json& json, std::map<std::string, T> Ts)
	{
		std::map<std::string, T> filtered;

		std::copy_if(Ts.begin(), Ts.end(), std::inserter(filtered, filtered.end()), [](auto pair)
			{
				std::shared_ptr<nlohmann::json> j = std::get<1>(pair.second);
				return !(j->contains("systemCreated") && j->at("systemCreated") == true);
			}
		);

		std::transform(filtered.begin(), filtered.end(), std::inserter(json, json.end()), [](const auto& pair)
			{
				std::shared_ptr<nlohmann::json> j = std::get<1>(pair.second);
				nlohmann::json jw(*j);
				jw["uuid"] = pair.first;
				jw["name"] = std::get<0>(pair.second);
				return jw;
			}
		);
	}

	void TemplatesStep(DX::StepTimer& timer);

#if defined(_EDITOR)
	bool AnyTemplatePopupOpen();
	void DrawTemplatesPopups(TemplateType t);
	bool TemplatesPopupIsOpen(TemplateType t);

	std::shared_ptr<JObject> GetTemplate(std::string uuid);
	std::map<TemplateType, std::vector<UUIDName>> GetTemplates();
	std::vector<UUIDName> GetTemplates(TemplateType t);
	TemplateType GetTemplateType(std::string uuid);
	std::vector<std::pair<std::string, JsonToEditorValueType>> GetTemplateAttributes(TemplateType t);
	std::map<std::string, JEdvDrawerFunction> GetTemplateDrawers(TemplateType t);
	std::vector<std::pair<std::string, bool>> GetTemplateRequiredAttributes(TemplateType t);
	nlohmann::json GetTemplateJson(TemplateType t);

	std::string GetTemplateName(TemplateType t, std::string uuid);
	void CreateTemplate(TemplateType t);
	void DeleteTemplate(TemplateType t, std::string uuid);
	void DeleteTemplate(std::string uuid);

#endif

}
