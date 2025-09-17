#pragma once

#include <vector>
#include <UUID.h>
#include <string>
#include <functional>
#include <nlohmann/json.hpp>
#include <StepTimer.h>
#include <JTemplate.h>

namespace Templates {

	nlohmann::json& GetSystemShaders();
	nlohmann::json& GetSystemSounds();
	nlohmann::json& GetSystemMaterials();
	nlohmann::json& GetSystemRenderPasses();
	nlohmann::json& GetSystemTextures();

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
	std::shared_ptr<JObject> GetTemplate(std::string uuid);
	std::map<TemplateType, std::vector<UUIDName>> GetTemplates();
	std::vector<UUIDName> GetTemplates(TemplateType t);
	TemplateType GetTemplateType(std::string uuid);
	std::vector<std::pair<std::string, JsonToEditorValueType>> GetTemplateAttributes(TemplateType t);
	std::map<std::string, JEdvEditorDrawerFunction> GetTemplateDrawers(TemplateType t);
	std::map<std::string, JEdvEditorDrawerFunction> GetTemplatePreviewers(TemplateType t);
	std::vector<std::string> GetTemplateRequiredAttributes(TemplateType t);
	nlohmann::json GetTemplateJson(TemplateType t);
	std::map<std::string, JEdvCreatorDrawerFunction> GetTemplateCreatorDrawers(TemplateType t);
	std::map<std::string, JEdvCreatorValidatorFunction> GetTemplateValidators(TemplateType t);

	std::string GetTemplateName(TemplateType t, std::string uuid);
	void CreateTemplate(TemplateType t, nlohmann::json json);
	void DeleteTemplate(TemplateType t, std::string uuid);
	void DeleteTemplate(std::string uuid);

	void RecursiveIterateArray(nlohmann::json object, const nlohmann::json& json, std::string uuid, std::function<void(nlohmann::json)> callkeyvalue);
	void RecursiveIterate(nlohmann::json object, const nlohmann::json& json, std::string uuid, std::function<void(nlohmann::json)> callkeyvalue);
	void FindTemplateReferencesInLevels(std::vector<nlohmann::json>& references, std::string uuid, std::string name);
	void FindTemplateReferencesInTemplates(std::vector<nlohmann::json>& references, std::string uuid, std::string name);
#endif

}
