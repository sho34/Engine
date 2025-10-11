#pragma once
#include <string>
#include <functional>
#include <Application.h>
#include <string_view>
#include <imgui_internal.h>
#include <filesystem>
#include <UUID.h>
#include <NoStd.h>
#include <nlohmann/json.hpp>

#if defined(_EDITOR)

static const std::string defaultLevelName = "";

namespace Editor
{
	extern bool NonGameMode;
	extern void OpenTemplateOnNextFrame(std::string uuid);
	extern void OpenSceneObjectOnNextFrame(std::string uuid);
};

namespace ImGui
{
	enum ItemLabelFlag
	{
		Left = 1u << 0u,
		Right = 1u << 1u,
		Default = Left,
	};

	bool DrawComboSelection(UUIDName selected, std::vector<UUIDName> selectables, std::function<void(UUIDName)> onSelect, std::string label = "##");
	bool DrawComboSelection(std::string selected, std::vector<std::string> selectables, std::function<void(std::string)> onSelect, std::string label = "##");
	bool DrawComboSelection(nlohmann::json& json, std::string attribute, std::vector<std::string> selectables, std::string label = "##");

	void ItemLabel(std::string_view title, ItemLabelFlag flags);

	void DrawItemWithEnabledState(std::function<void()> draw, bool enabled);

	/*
	template<typename T>
	inline bool ImDrawColorEdit3(std::string tableName, T& Tcolor, std::function<void(T)> onChange)
	{
		bool ret = false;
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
		{
			std::string tableId = tableName.substr(tableName.find_first_of("-") + 1);
			ImGui::PushID(tableId.c_str());
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				std::string label = tableId.substr(tableId.find_first_of("-") + 1);
				ImGui::Text(label.c_str());

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				if (ImGui::ColorEdit3(label.c_str(), (float*)&Tcolor)) {
					onChange(Tcolor);
					ret = true;
				}
			}
			ImGui::PopID();
			ImGui::EndTable();
		}
		return ret;
	}
	*/

	/*
	template<typename T>
	inline bool ImDrawColorEdit4(std::string tableName, T& Tcolor, std::function<void(T)> onChange)
	{
		bool ret = false;
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
		{
			std::string tableId = tableName.substr(tableName.find_first_of("-") + 1);
			ImGui::PushID(tableId.c_str());
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				std::string label = tableId.substr(tableId.find_first_of("-") + 1);
				ImGui::Text(label.c_str());

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				if (ImGui::ColorEdit4(label.c_str(), (float*)&Tcolor)) {
					onChange(Tcolor);
					ret = true;
				}
			}
			ImGui::PopID();
			ImGui::EndTable();
		}
		return ret;
	}
	*/

	/*
	template<typename T>
	inline bool ImDrawDegreesValues(std::string tableName, std::vector<std::string> componentsLabel, T& rot, std::function<void(T)> onChange, float minDeg = -180.0f, float maxDeg = 180.0f)
	{
		bool ret = false;
		if (ImGui::BeginTable(tableName.c_str(), static_cast<int>(componentsLabel.size()) + 1, ImGuiTableFlags_NoSavedSettings))
		{
			std::string tableId = tableName.substr(tableName.find_first_of("-") + 1);
			ImGui::PushID(tableId.c_str());
			{
				T rads;
				for (int i = 0; i < componentsLabel.size(); i++) {
					*(((float*)&rads) + i) = XMConvertToRadians(*(((float*)&rot) + i));
				}
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				std::string label = tableId.substr(tableId.find_first_of("-") + 1);
				ImGui::Text(label.c_str());

				ImGui::PushID("sliders");
				{
					for (int i = 0; i < componentsLabel.size(); i++) {
						ImGui::TableSetColumnIndex(i + 1);
						if (ImGui::SliderAngle(componentsLabel[i].c_str(), (((float*)&rads) + i), minDeg, maxDeg, "%.2f", ImGuiSliderFlags_AlwaysClamp))
						{
							*(((float*)&rot) + i) = XMConvertToDegrees(*(((float*)&rads) + i));
							onChange(rot);
							ret = true;
							}
					}
				}
				ImGui::PopID();
			}
			ImGui::PopID();
			ImGui::EndTable();
		}
		return ret;
	}
	*/

	/*
	template<typename T>
	inline bool ImDrawFloatValues(std::string tableName, std::vector<std::string> componentsLabel, T& values, std::function<void(T)> onChange)
	{
		bool ret = false;
		if (ImGui::BeginTable(tableName.c_str(), static_cast<int>(componentsLabel.size()) + 1, ImGuiTableFlags_NoSavedSettings))
		{
			std::string tableId = tableName.substr(tableName.find_first_of("-") + 1);
			ImGui::PushID(tableId.c_str());
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				std::string label = tableId.substr(tableId.find_first_of("-") + 1);
				ImGui::Text(label.c_str());

				ImGui::PushID("floats");
				{
					for (int i = 0; i < componentsLabel.size(); i++) {
						ImGui::TableSetColumnIndex(i + 1);
						if (ImGui::InputFloat(componentsLabel[i].c_str(), ((float*)&values) + i))
						{
							onChange(values);
							ret = true;
						}
					}
				}
				ImGui::PopID();

			}
			ImGui::PopID();
			ImGui::EndTable();
		}
		return ret;
	}
	*/

	void DrawTextureImage(ImTextureID textureId, unsigned int textureWidth, unsigned int textureHeight);

	inline bool DrawFromCombo(nlohmann::json& json, const std::string attribute, auto& listMap, std::string label = "")
	{
		std::string value = json.at(attribute);
		std::vector<std::string> selectables = nostd::GetKeysFromMap(listMap);
		bool ret = false;
		DrawComboSelection(value, selectables, [&json, &attribute, &ret](std::string newValue)
			{
				json[attribute] = newValue;
				ret = true;
			},
			label.c_str()
		);
		return ret;
	}

	bool DrawJsonCheckBox(nlohmann::json& json, const std::string attribute);

	bool DrawFromFloat(nlohmann::json& json, const std::string attribute, std::string label = "");

	bool DrawFromInt(nlohmann::json& json, const std::string attribute, std::string label = "");

	bool DrawFromUInt(nlohmann::json& json, const std::string attribute, std::string label = "");

	void DrawDynamicArray(
		std::string label,
		nlohmann::json& arr,
		std::function<void(nlohmann::json&, unsigned int)> insert,
		std::function<void(nlohmann::json&, unsigned int)> remove,
		std::function<void(nlohmann::json&, unsigned int)> draw,
		unsigned int maxItems,
		unsigned int minItems = 0U
	);

	bool DrawJsonInputText(nlohmann::json& json, std::string att);
	bool OpenFileDialog(std::wstring& path, std::wstring defaultDirectory, std::wstring defaultFileName, std::vector<std::pair<std::wstring, std::wstring>>& specs);
	void OpenFile(std::function<void(std::filesystem::path)> onFileSelected, std::string defaultDirectory, std::vector<std::string> filterName = { "JSON files. (*.json)" }, std::vector<std::string> filterPattern = { "*.json" }, bool detach = false);
	void OpenTemplate(const char* iconCode, UUIDName uuidName);
	void OpenSceneObject(const char* iconCode, UUIDName uuidName);
}
#endif
