#pragma once

#if defined(_EDITOR)
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

#endif
