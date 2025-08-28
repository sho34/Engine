#pragma once
#include <string>
#include <functional>
#include <Application.h>
#include <string_view>
#include <imgui_internal.h>
#include <filesystem>
#include <UUID.h>
#include <NoStd.h>

#if defined(_EDITOR)

static const std::string defaultLevelName = "";

namespace Editor
{
	extern bool NonGameMode;
	extern void OpenTemplateOnNextFrame(std::string uuid);
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

	/*
	inline bool drawFromCheckBox(nlohmann::json& json, const std::string attribute, std::string label = "")
	{
		bool value = json.at(attribute);
		if (ImGui::Checkbox(label.c_str(), &value)) { json[attribute] = value; return true; }
		return false;
	}
	*/

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

	/*
	inline void ImDrawObject(
		std::string label,
		nlohmann::json& object,
		std::map<std::string, std::function<void(nlohmann::json&)>> addAttribute,
		std::map<std::string, std::function<void(nlohmann::json&)>> drawAttribute
	)
	{
		//get all the attributes in the object
		std::vector<std::string> attributes = nostd::GetKeysFromMap(addAttribute);

		//make a list of the attributes that can be added
		std::vector<std::string> selectables = { " " };
		for (auto& attribute : attributes)
		{
			if (!object.contains(attribute)) selectables.push_back(attribute);
		}

		//draw a combo for adding attributes to the object
		ImGui::Text(label.c_str());
		if (selectables.size() > 1)
		{
			ImGui::PushID((label + "-add-combo").c_str());
			DrawComboSelection(selectables[0], selectables, [&object, addAttribute](std::string attribute)
				{
					addAttribute.at(attribute)(object);
				}, ""
			);
			ImGui::PopID();
		}

		//if the object is empty go no further
		if (object.empty()) return;

		//go through each attribute in the object and render the attribute representation
		for (auto attribute : attributes)
		{
			if (!object.contains(attribute)) continue;

			ImGui::PushID((label + "-" + attribute).c_str());
			{
				if (ImGui::SmallButton(ICON_FA_TIMES))
				{
					auto pos = object.find(attribute);
					object.erase(pos);
					ImGui::PopID();
					return;
				}
				ImGui::SameLine();
				ImGui::Text(attribute.c_str());

				drawAttribute.at(attribute)(object);
			}
			ImGui::PopID();
		}
	}
	*/

	/*
	static std::map<MaterialVariablesTypes, std::function<bool(unsigned int, nlohmann::json&)>> ImMaterialVariablesDraw =
	{
		{ MAT_VAR_BOOLEAN, [](unsigned int index, nlohmann::json& variable) { return drawFromCheckBox(variable,"value",variable.at("variable")); }},
		{ MAT_VAR_INTEGER, [](unsigned int index, nlohmann::json& variable) { return DrawFromInt(variable,"value",variable.at("variable")); }},
		{ MAT_VAR_UNSIGNED_INTEGER, [](unsigned int index, nlohmann::json& variable) { return DrawFromUInt(variable,"value",variable.at("variable")); }},
		{ MAT_VAR_RGB, [](unsigned int index, nlohmann::json& variable) {
			XMFLOAT3 value = { variable.at("value").at(0), variable.at("value").at(1), variable.at("value").at(2) };
			return ImDrawColorEdit3<XMFLOAT3>("material-" + std::string(variable.at("variable")), value, [&variable](XMFLOAT3 f3)
				{
					nlohmann::json& j = variable.at("value");
					j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
				}
			);
		}},
		{ MAT_VAR_RGBA, [](unsigned int index, nlohmann::json& variable) {
			XMFLOAT4 value = { variable.at("value").at(0), variable.at("value").at(1), variable.at("value").at(2), variable.at("value").at(3) };
			return ImDrawColorEdit4<XMFLOAT4>("material-" + std::string(variable.at("variable")), value, [&variable](XMFLOAT4 f4)
				{
					nlohmann::json& j = variable.at("value");
					j.at(0) = f4.x; j.at(1) = f4.y; j.at(2) = f4.z; j.at(3) = f4.w;
				}
			);
		}},
		{ MAT_VAR_FLOAT, [](unsigned int index, nlohmann::json& variable) {
			float value = variable.at("value");
			return ImDrawFloatValues<float>("material-" + std::string(variable.at("variable")), { "" }, value, [&variable](float f)
				{
					variable.at("value") = f;
				}
			);
		}},
		{ MAT_VAR_FLOAT2, [](unsigned int index, nlohmann::json& variable) {
			XMFLOAT2 value = { variable.at("value").at(0), variable.at("value").at(1) };
			return ImDrawFloatValues<XMFLOAT2>("material-" + std::string(variable.at("variable")), { "x","y" }, value, [&variable](XMFLOAT2 f2)
				{
					nlohmann::json& j = variable.at("value");
					j.at(0) = f2.x; j.at(1) = f2.y;
				}
			);
		}},
		{ MAT_VAR_FLOAT3, [](unsigned int index, nlohmann::json& variable)
		{
			XMFLOAT3 value = { variable.at("value").at(0), variable.at("value").at(1), variable.at("value").at(2)};
			return ImDrawFloatValues<XMFLOAT3>("material-" + std::string(variable.at("variable")), { "x","y","z" }, value, [&variable](XMFLOAT3 f3)
				{
					nlohmann::json& j = variable.at("value");
					j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
				}
			);
		}},
		{ MAT_VAR_FLOAT4, [](unsigned int index, nlohmann::json& variable) {
			XMFLOAT4 value = { variable.at("value").at(0), variable.at("value").at(1), variable.at("value").at(2), variable.at("value").at(3)};
			return ImDrawFloatValues<XMFLOAT4>("material-" + std::string(variable.at("variable")), { "x","y","z","w"}, value, [&variable](XMFLOAT4 f4)
				{
					nlohmann::json& j = variable.at("value");
					j.at(0) = f4.x; j.at(1) = f4.y; j.at(2) = f4.z;; j.at(3) = f4.w;
				}
			);
		}},
		{ MAT_VAR_MATRIX4X4, [](unsigned int index, nlohmann::json& variable) {
			bool ret = false;
			{
				XMFLOAT4 value = { variable.at("value").at(0), variable.at("value").at(1), variable.at("value").at(2), variable.at("value").at(3)};
				ret |= ImDrawFloatValues<XMFLOAT4>("material-" + std::string(variable.at("variable")), { "0.x","0.y","0.z","0.w"}, value, [&variable](XMFLOAT4 f4)
					{
						nlohmann::json& j = variable.at("value");
						j.at(0) = f4.x; j.at(1) = f4.y; j.at(2) = f4.z; j.at(3) = f4.w;
					}
				);
			}
			{
				XMFLOAT4 value = { variable.at("value").at(4), variable.at("value").at(5), variable.at("value").at(6), variable.at("value").at(7)};
				ret |= ImDrawFloatValues<XMFLOAT4>("material-" + std::string(variable.at("variable")), { "1.x","1.y","1.z","1.w" }, value, [&variable](XMFLOAT4 f4)
					{
						nlohmann::json& j = variable.at("value");
						j.at(4) = f4.x; j.at(5) = f4.y; j.at(6) = f4.z; j.at(7) = f4.w;
					}
				);
			}
			{
				XMFLOAT4 value = { variable.at("value").at(8), variable.at("value").at(9), variable.at("value").at(10), variable.at("value").at(11)};
				ret |= ImDrawFloatValues<XMFLOAT4>("material-" + std::string(variable.at("variable")), { "2.x","2.y","2.z","2.w" }, value, [&variable](XMFLOAT4 f4)
					{
						nlohmann::json& j = variable.at("value");
						j.at(8) = f4.x; j.at(9) = f4.y; j.at(10) = f4.z; j.at(11) = f4.w;
					}
				);
			}
			{
				XMFLOAT4 value = { variable.at("value").at(12), variable.at("value").at(13), variable.at("value").at(14), variable.at("value").at(15)};
				ret |= ImDrawFloatValues<XMFLOAT4>("material-" + std::string(variable.at("variable")), { "3.x","3.y","3.z","3.w" }, value, [&variable](XMFLOAT4 f4)
					{
						nlohmann::json& j = variable.at("value");
						j.at(12) = f4.x; j.at(13) = f4.y; j.at(14) = f4.z; j.at(15) = f4.w;
					}
				);
			}
			return ret;
		}},
	}
	*/

	/*
	inline bool ImDrawMappedValues(nlohmann::json& sha, std::map<std::string, MaterialVariablesTypes> mappeables)
	{
		if (mappeables.size() == 0ULL) return false;

		ImGui::Separator();

		std::set<std::string> existingVariables;
		if (sha.contains("mappedValues"))
		{
			unsigned int sz = static_cast<unsigned int>(sha.at("mappedValues").size());
			for (unsigned int i = 0; i < sz; i++)
			{
				existingVariables.insert(sha.at("mappedValues").at(i).at("variable"));
			}
		}

		std::vector<std::string> selectables = { " " };

		for (auto it = mappeables.begin(); it != mappeables.end(); it++)
		{
			if (existingVariables.contains(it->first)) continue;
			selectables.push_back(it->first);
		}

		//draw a combo for adding attributes to the object
		ImGui::Text("Mapped Values");
		if (selectables.size() > 1)
		{
			ImGui::PushID("mapped-values-add-combo");
			{
				DrawComboSelection(selectables[0], selectables, [&sha, mappeables](std::string variable)
					{
						if (!sha.contains("mappedValues")) { sha["mappedValues"] = nlohmann::json::array(); }
						MaterialVariablesMappedJsonInitializer.at(mappeables.at(variable))(sha["mappedValues"], variable);
					}, ""
				);
			}
			ImGui::PopID();
		}

		bool ret = false;
		if (sha.contains("mappedValues"))
		{
			unsigned int sz = static_cast<unsigned int>(sha.at("mappedValues").size());
			for (unsigned int i = 0; i < sz; i++)
			{
				ImGui::PushID(("mapped-values-draw-" + std::to_string(i + 1)).c_str());
				{
					nlohmann::json& mappedV = sha.at("mappedValues").at(i);
					ret |= ImMaterialVariablesDraw.at(StringToMaterialVariablesTypes.at(std::string(mappedV.at("variableType"))))(i, mappedV);
				}
				ImGui::PopID();
			}
		}
		return ret;
	}
	*/

	//bool OpenFileDialog(std::wstring& path, std::wstring defaultDirectory = L"", std::wstring defaultFileName = L"", std::vector<std::pair<COMDLG_FILTERSPEC, int>> pFilterInfo = {});
	bool OpenFileDialog(std::wstring& path, std::wstring defaultDirectory, std::wstring defaultFileName, std::vector<std::pair<std::wstring, std::wstring>>& specs);
	void OpenFile(std::function<void(std::filesystem::path)> onFileSelected, std::string defaultDirectory, std::vector<std::string> filterName = { "JSON files. (*.json)" }, std::vector<std::string> filterPattern = { "*.json" }, bool detach = false);

	/*
	inline void ImDrawFileSelector(std::string label, std::string fileName, std::function<void(std::filesystem::path)> onFileSelected, std::string defaultDirectory, std::string filterName, std::string filterPattern, bool enabled = true)
	{
		if (ImGui::Button(ICON_FA_ELLIPSIS_H))
		{
			if (enabled)
			{
				OpenFile(onFileSelected, defaultDirectory, filterName, filterPattern);
			}
		}
		ImGui::SameLine();
		ImGui::InputText(label.c_str(), fileName.data(), fileName.size(), ImGuiInputTextFlags_ReadOnly);
	}
	*/

	/*
	inline void ImDrawJsonFilePicker(nlohmann::json& json, std::string att, std::string defaultDirectory, std::string filterName, std::string filterPattern, std::function<void()> onSelectFile = [] {}, std::string label = "##")
	{
		std::string fileName = "";
		if (json.contains(att) && json.at(att) != "")
		{
			fileName = json.at(att);
			std::filesystem::path rootFolder = fileName;
			defaultDirectory = rootFolder.parent_path().string();
		}

		ImDrawFileSelector(label, fileName, [&json, att, onSelectFile](std::filesystem::path path)
			{
				std::filesystem::path curPath = std::filesystem::current_path();
				std::filesystem::path relPath = std::filesystem::relative(path, curPath);
				json.at(att) = relPath.string();
				onSelectFile();
			},
			defaultDirectory, filterName, filterPattern
		);

	}
	*/

	void OpenTemplate(const char* iconCode, UUIDName uuidName);

}
#endif
