#pragma once

#if defined(_EDITOR)
#include "../Templates/Material/Variables.h"

static const std::string defaultLevelName = "baseLevel.json";

inline void DrawComboSelection(std::string selected, std::vector<std::string> selectables, std::function<void(std::string)> onSelect, std::string label = "") {

	int current_item = static_cast<int>(std::find(selectables.begin(), selectables.end(), selected) - selectables.begin());

	if (ImGui::BeginCombo(label.c_str(), selectables[current_item].c_str()))
	{
		for (int i = 0; i < selectables.size(); i++)
		{
			std::string selectableId = "combo#selectable#" + label + "#" + std::to_string(i);
			ImGui::PushID(selectableId.c_str());
			if (ImGui::Selectable(selectables[i].c_str(), current_item == i)) {
				onSelect(selectables[i]);
			}
			ImGui::PopID();
		}

		ImGui::EndCombo();
	}
};

template<typename T>
inline void ImDrawColorEdit3(std::string tableName, T& Tcolor, std::function<void(T)> onChange) {

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
			}
		}
		ImGui::PopID();
		ImGui::EndTable();
	}
}

template<typename T>
inline void ImDrawColorEdit4(std::string tableName, T& Tcolor, std::function<void(T)> onChange) {

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
			}
		}
		ImGui::PopID();
		ImGui::EndTable();
	}
}

template<typename T>
inline void ImDrawDegreesValues(std::string tableName, std::vector<std::string> componentsLabel, T& rot, std::function<void(T)> onChange) {
	if (ImGui::BeginTable(tableName.c_str(), static_cast<int>(componentsLabel.size()) + 1, ImGuiTableFlags_NoSavedSettings))
	{
		std::string tableId = tableName.substr(tableName.find_first_of("-") + 1);
		ImGui::PushID(tableId.c_str());
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string label = tableId.substr(tableId.find_first_of("-") + 1);
			ImGui::Text(label.c_str());

			ImGui::PushID("sliders");
			{
				for (int i = 0; i < componentsLabel.size(); i++) {
					ImGui::TableSetColumnIndex(i + 1);
					if (ImGui::SliderAngle(componentsLabel[i].c_str(), (((float*)&rot) + i), 0.0f, 360.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp)) { onChange(rot); }
				}
			}
			ImGui::PopID();
		}
		ImGui::PopID();
		ImGui::EndTable();
	}
}

template<typename T>
inline void ImDrawFloatValues(std::string tableName, std::vector<std::string> componentsLabel, T& values, std::function<void(T)> onChange) {
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
					if (ImGui::InputFloat(componentsLabel[i].c_str(), ((float*)&values) + i)) {
						onChange(values);
					}
				}
			}
			ImGui::PopID();

		}
		ImGui::PopID();
		ImGui::EndTable();
	}
}

inline void ImDrawTextureImage(ImTextureID textureId, unsigned int textureWidth, unsigned int textureHeight) {
	ImVec2 currentRegionAvail = ImGui::GetContentRegionAvail();
	float sizeY = 0.0f;
	if (textureWidth < textureHeight) {
		sizeY = (float)currentRegionAvail.x / (static_cast<float>(textureWidth) / static_cast<float>(textureHeight));
	}
	else {
		sizeY = (float)currentRegionAvail.x * (static_cast<float>(textureHeight) / static_cast<float>(textureWidth));
	}
	ImGui::Image(textureId, ImVec2((float)currentRegionAvail.x, sizeY));
}

inline void drawFromCombo(nlohmann::json& json, const std::string attribute, auto& listMap, std::string label = "")
{
	std::string value = json.at(attribute);
	std::vector<std::string> selectables = nostd::GetKeysFromMap(listMap);
	DrawComboSelection(value, selectables, [&json, &attribute](std::string newValue)
		{
			json[attribute] = newValue;
		}, label.c_str()
			);
};

inline void drawFromCheckBox(nlohmann::json& json, const std::string attribute, std::string label = "")
{
	bool value = json.at(attribute);
	if (ImGui::Checkbox(label.c_str(), &value)) { json[attribute] = value; }
};

inline void drawFromFloat(nlohmann::json& json, const std::string attribute, std::string label = "")
{
	float value = json.at(attribute);
	if (ImGui::InputFloat(label.c_str(), &value)) { json[attribute] = value; }
};

inline void drawFromInt(nlohmann::json& json, const std::string attribute, std::string label = "")
{
	int value = json.at(attribute);
	if (ImGui::InputInt(label.c_str(), &value)) { json[attribute] = value; }
};

inline void drawFromUInt(nlohmann::json& json, const std::string attribute, std::string label = "")
{
	int value = json.at(attribute);
	if (ImGui::InputInt(label.c_str(), &value)) { value = max(0, value); json[attribute] = value; }
};

inline void ImDrawDynamicArray(
	std::string label,
	nlohmann::json& arr,
	unsigned int maxItems,
	std::function<void(nlohmann::json&, unsigned int)> insert,
	std::function<void(nlohmann::json&, unsigned int)> draw
)
{
	bool canAdd = arr.size() < maxItems;
	bool canDelete = arr.size() > 1U;
	int addIndex = -1;
	int deleteIndex = -1;

	for (unsigned int i = 0U; i < arr.size(); i++)
	{
		std::string itemID = label + "#" + std::to_string(i + 1U);
		ImGui::PushID(itemID.c_str());
		{
			if (canAdd)
			{
				if (ImGui::SmallButton(ICON_FA_PLUS))
				{
					insert(arr, i);
					ImGui::PopID();
					break;
				}
				ImGui::SameLine();
			}

			if (canDelete)
			{
				if (ImGui::SmallButton(ICON_FA_TIMES))
				{
					arr.erase(i);
					ImGui::PopID();
					break;
				}
				ImGui::SameLine();
			}

			draw(arr[i], i);
		}
		ImGui::PopID();
	}
}

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
		ImGui::PushID("pipelineState-add-combo");
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

static std::map<MaterialVariablesTypes, std::function<void(unsigned int, nlohmann::json&)>> ImMaterialVariablesDraw =
{
	{ MAT_VAR_BOOLEAN, [](unsigned int index, nlohmann::json& variable) { drawFromCheckBox(variable,"value",variable.at("variable")); }},
	{ MAT_VAR_INTEGER, [](unsigned int index, nlohmann::json& variable) { drawFromInt(variable,"value",variable.at("variable")); }},
	{ MAT_VAR_UNSIGNED_INTEGER, [](unsigned int index, nlohmann::json& variable) { drawFromUInt(variable,"value",variable.at("variable")); }},
	{ MAT_VAR_RGB, [](unsigned int index, nlohmann::json& variable) {
		XMFLOAT3 value = { variable.at("value").at(0), variable.at("value").at(1), variable.at("value").at(2) };
		ImDrawColorEdit3<XMFLOAT3>("material-" + std::string(variable.at("variable")), value, [&variable](XMFLOAT3 f3)
			{
				nlohmann::json& j = variable.at("value");
				j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
			}
		);
	}},
	{ MAT_VAR_RGBA, [](unsigned int index, nlohmann::json& variable) {
		XMFLOAT4 value = { variable.at("value").at(0), variable.at("value").at(1), variable.at("value").at(2), variable.at("value").at(3) };
		ImDrawColorEdit4<XMFLOAT4>("material-" + std::string(variable.at("variable")), value, [&variable](XMFLOAT4 f4)
			{
				nlohmann::json& j = variable.at("value");
				j.at(0) = f4.x; j.at(1) = f4.y; j.at(2) = f4.z; j.at(3) = f4.w;
			}
		);
	}},
	{ MAT_VAR_FLOAT, [](unsigned int index, nlohmann::json& variable) {
		float value = variable.at("value");
		ImDrawFloatValues<float>("material-" + std::string(variable.at("variable")), { "" }, value, [&variable](float f)
			{
				variable.at("value") = f;
			}
		);
	}},
	{ MAT_VAR_FLOAT2, [](unsigned int index, nlohmann::json& variable) {
		XMFLOAT2 value = { variable.at("value").at(0), variable.at("value").at(1) };
		ImDrawFloatValues<XMFLOAT2>("material-" + std::string(variable.at("variable")), { "x","y" }, value, [&variable](XMFLOAT2 f2)
			{
				nlohmann::json& j = variable.at("value");
				j.at(0) = f2.x; j.at(1) = f2.y;
			}
		);
	}},
	{ MAT_VAR_FLOAT3, [](unsigned int index, nlohmann::json& variable)
	{
		XMFLOAT3 value = { variable.at("value").at(0), variable.at("value").at(1), variable.at("value").at(2)};
		ImDrawFloatValues<XMFLOAT3>("material-" + std::string(variable.at("variable")), { "x","y","z" }, value, [&variable](XMFLOAT3 f3)
			{
				nlohmann::json& j = variable.at("value");
				j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
			}
		);
	}},
	{ MAT_VAR_FLOAT4, [](unsigned int index, nlohmann::json& variable) {
		XMFLOAT4 value = { variable.at("value").at(0), variable.at("value").at(1), variable.at("value").at(2), variable.at("value").at(3)};
		ImDrawFloatValues<XMFLOAT4>("material-" + std::string(variable.at("variable")), { "x","y","z","w"}, value, [&variable](XMFLOAT4 f4)
			{
				nlohmann::json& j = variable.at("value");
				j.at(0) = f4.x; j.at(1) = f4.y; j.at(2) = f4.z;; j.at(3) = f4.w;
			}
		);
	}},
	{ MAT_VAR_MATRIX4X4, [](unsigned int index, nlohmann::json& variable) {
		{
			XMFLOAT4 value = { variable.at("value").at(0), variable.at("value").at(1), variable.at("value").at(2), variable.at("value").at(3)};
			ImDrawFloatValues<XMFLOAT4>("material-" + std::string(variable.at("variable")), { "0.x","0.y","0.z","0.w"}, value, [&variable](XMFLOAT4 f4)
				{
					nlohmann::json& j = variable.at("value");
					j.at(0) = f4.x; j.at(1) = f4.y; j.at(2) = f4.z; j.at(3) = f4.w;
				}
			);
		}
		{
			XMFLOAT4 value = { variable.at("value").at(4), variable.at("value").at(5), variable.at("value").at(6), variable.at("value").at(7)};
			ImDrawFloatValues<XMFLOAT4>("material-" + std::string(variable.at("variable")), { "1.x","1.y","1.z","1.w" }, value, [&variable](XMFLOAT4 f4)
				{
					nlohmann::json& j = variable.at("value");
					j.at(4) = f4.x; j.at(5) = f4.y; j.at(6) = f4.z; j.at(7) = f4.w;
				}
			);
		}
		{
			XMFLOAT4 value = { variable.at("value").at(8), variable.at("value").at(9), variable.at("value").at(10), variable.at("value").at(11)};
			ImDrawFloatValues<XMFLOAT4>("material-" + std::string(variable.at("variable")), { "2.x","2.y","2.z","2.w" }, value, [&variable](XMFLOAT4 f4)
				{
					nlohmann::json& j = variable.at("value");
					j.at(8) = f4.x; j.at(9) = f4.y; j.at(10) = f4.z; j.at(11) = f4.w;
				}
			);
		}
		{
			XMFLOAT4 value = { variable.at("value").at(12), variable.at("value").at(13), variable.at("value").at(14), variable.at("value").at(15)};
			ImDrawFloatValues<XMFLOAT4>("material-" + std::string(variable.at("variable")), { "3.x","3.y","3.z","3.w" }, value, [&variable](XMFLOAT4 f4)
				{
					nlohmann::json& j = variable.at("value");
					j.at(12) = f4.x; j.at(13) = f4.y; j.at(14) = f4.z; j.at(15) = f4.w;
				}
			);
		}
	}},
};

#endif
