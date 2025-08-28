#pragma once

#include <nlohmann/json.hpp>
#include <any>

struct JObject : nlohmann::json
{
	virtual ~JObject() = default;

	std::map<std::string, std::tuple<size_t, bool>> UpdateFlagsMap;
	std::map<std::string, nlohmann::json> UpdatePrevValues;
	unsigned int updateFlag = 0U;

	JObject(nlohmann::json json) :nlohmann::json(json) {}
	void JUpdate(nlohmann::json p)
	{
		for (auto& [key, value] : p.items())
		{
			UpdatePrevValues.insert_or_assign(key, at(key));
			bool update = std::get<1>(UpdateFlagsMap.at(key));
			if (!update) continue;
			size_t flag = std::get<0>(UpdateFlagsMap.at(key));
			updateFlag |= flag;
		}
		merge_patch(p);
	}

	bool dirty(size_t flag) const
	{
		return !!(updateFlag & (1 << flag));
	}

	void clean(size_t flag)
	{
		updateFlag &= ~(1 << flag);
	}

	void clear()
	{
		updateFlag = 0ULL;
	}

	virtual void EditorPreview(size_t flags) {}
	virtual void DestroyEditorPreview() {}

};