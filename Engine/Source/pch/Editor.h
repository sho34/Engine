#pragma once
#include <string>
#include <functional>
#include <Application.h>

#if defined(_EDITOR)

static const std::string defaultLevelName = "";

inline bool DrawComboSelection(UUIDName selected, std::vector<UUIDName> selectables, std::function<void(UUIDName)> onSelect, std::string label = "")
{
	bool ret = false;
	int current_item = static_cast<int>(std::find(selectables.begin(), selectables.end(), selected) - selectables.begin());
	std::string& preview = std::get<1>(selected);
	if (ImGui::BeginCombo(label.c_str(), preview.c_str()))
	{
		for (int i = 0; i < selectables.size(); i++)
		{
			std::string selectableId = "combo#selectable#" + label + "#" + std::to_string(i);
			ImGui::PushID(selectableId.c_str());
			std::string selectableName = std::get<1>(selectables[i]);
			if (ImGui::Selectable(selectableName.c_str(), current_item == i, (current_item == i) ? ImGuiSelectableFlags_Highlight : 0))
			{
				ret = true;
				onSelect(selectables[i]);
			}
			if (current_item == i)
			{
				ImGui::SetItemDefaultFocus();
			}
			ImGui::PopID();
		}

		ImGui::EndCombo();
	}
	return ret;
}

inline bool DrawComboSelection(std::string selected, std::vector<std::string> selectables, std::function<void(std::string)> onSelect, std::string label = "")
{
	bool ret = false;
	int current_item = static_cast<int>(std::find(selectables.begin(), selectables.end(), selected) - selectables.begin());
	if (ImGui::BeginCombo(label.c_str(), selectables[current_item].c_str()))
	{
		for (int i = 0; i < selectables.size(); i++)
		{
			std::string selectableId = "combo#selectable#" + label + "#" + std::to_string(i);
			ImGui::PushID(selectableId.c_str());
			if (ImGui::Selectable(selectables[i].c_str(), current_item == i, (current_item == i) ? ImGuiSelectableFlags_Highlight : 0)) {
				ret = true;
				onSelect(selectables[i]);
			}
			if (current_item == i)
			{
				ImGui::SetItemDefaultFocus();
			}
			ImGui::PopID();
		}

		ImGui::EndCombo();
	}
	return ret;
};

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

template<typename T>
inline bool ImDrawDegreesValues(std::string tableName, std::vector<std::string> componentsLabel, T& rot, std::function<void(T)> onChange)
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

			ImGui::PushID("sliders");
			{
				for (int i = 0; i < componentsLabel.size(); i++) {
					ImGui::TableSetColumnIndex(i + 1);
					if (ImGui::SliderAngle(componentsLabel[i].c_str(), (((float*)&rot) + i), 0.0f, 360.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp))
					{
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

inline bool drawFromCombo(nlohmann::json& json, const std::string attribute, auto& listMap, std::string label = "")
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
};

inline bool drawFromCheckBox(nlohmann::json& json, const std::string attribute, std::string label = "")
{
	bool value = json.at(attribute);
	if (ImGui::Checkbox(label.c_str(), &value)) { json[attribute] = value; return true; }
	return false;
};

inline bool drawFromFloat(nlohmann::json& json, const std::string attribute, std::string label = "")
{
	float value = json.at(attribute);
	if (ImGui::InputFloat(label.c_str(), &value)) { json[attribute] = value; return true; }
	return false;
};

inline bool drawFromInt(nlohmann::json& json, const std::string attribute, std::string label = "")
{
	int value = json.at(attribute);
	if (ImGui::InputInt(label.c_str(), &value)) { json[attribute] = value; return true; }
	return false;
};

inline bool drawFromUInt(nlohmann::json& json, const std::string attribute, std::string label = "")
{
	int value = json.at(attribute);
	if (ImGui::InputInt(label.c_str(), &value)) { value = max(0, value); json[attribute] = value; return true; }
	return false;
};

inline void ImDrawDynamicArray(
	std::string label,
	nlohmann::json& arr,
	std::function<void(nlohmann::json&, unsigned int)> insert,
	std::function<void(nlohmann::json&, unsigned int)> draw,
	unsigned int maxItems,
	unsigned int minItems = 0U
)
{
	bool canAdd = arr.size() < maxItems;
	bool canDelete = arr.size() > minItems;
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

static std::map<MaterialVariablesTypes, std::function<bool(unsigned int, nlohmann::json&)>> ImMaterialVariablesDraw =
{
	{ MAT_VAR_BOOLEAN, [](unsigned int index, nlohmann::json& variable) { return drawFromCheckBox(variable,"value",variable.at("variable")); }},
	{ MAT_VAR_INTEGER, [](unsigned int index, nlohmann::json& variable) { return drawFromInt(variable,"value",variable.at("variable")); }},
	{ MAT_VAR_UNSIGNED_INTEGER, [](unsigned int index, nlohmann::json& variable) { return drawFromUInt(variable,"value",variable.at("variable")); }},
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
};

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
				ret |= ImMaterialVariablesDraw.at(StrToMaterialVariablesTypes.at(std::string(mappedV.at("variableType"))))(i, mappedV);
			}
			ImGui::PopID();
		}
	}
	return ret;
}

inline bool OpenFileDialog(std::wstring& path, std::wstring defaultDirectory = L"", std::wstring defaultFileName = L"", std::pair<COMDLG_FILTERSPEC*, int>* pFilterInfo = nullptr)
{
	IFileOpenDialog* p_file_open = nullptr;
	bool are_all_operation_success = false;
	while (!are_all_operation_success)
	{
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&p_file_open));
		if (FAILED(hr))
			break;

		if (!pFilterInfo)
		{
			COMDLG_FILTERSPEC open_filter[1];
			open_filter[0].pszName = L"All files";
			open_filter[0].pszSpec = L"*.*";
			hr = p_file_open->SetFileTypes(1, open_filter);
			if (FAILED(hr))
				break;
			hr = p_file_open->SetFileTypeIndex(1);
			if (FAILED(hr))
				break;
		}
		else
		{
			hr = p_file_open->SetFileTypes(pFilterInfo->second, pFilterInfo->first);
			if (FAILED(hr))
				break;
			hr = p_file_open->SetFileTypeIndex(1);
			if (FAILED(hr))
				break;
		}

		if (!defaultDirectory.empty()) {
			IShellItem* pCurFolder = NULL;
			hr = SHCreateItemFromParsingName(defaultDirectory.c_str(), NULL, IID_PPV_ARGS(&pCurFolder));
			if (FAILED(hr))
				break;
			p_file_open->SetFolder(pCurFolder);
			pCurFolder->Release();
		}

		if (!defaultFileName.empty())
		{
			hr = p_file_open->SetFileName(defaultFileName.c_str());
			if (FAILED(hr))
				break;
		}

		hr = p_file_open->Show(NULL);
		if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) // No item was selected.
		{
			are_all_operation_success = true;
			break;
		}
		else if (FAILED(hr))
			break;

		IShellItem* p_item;
		hr = p_file_open->GetResult(&p_item);
		if (FAILED(hr))
			break;

		PWSTR item_path;
		hr = p_item->GetDisplayName(SIGDN_FILESYSPATH, &item_path);
		if (FAILED(hr))
			break;
		path = item_path;
		CoTaskMemFree(item_path);
		p_item->Release();

		are_all_operation_success = true;
	}

	if (p_file_open)
		p_file_open->Release();
	return are_all_operation_success;
}

inline void OpenFile(std::function<void(std::filesystem::path)> onFileSelected, std::string defaultDirectory, std::string filterName = "JSON files. (*.json)", std::string filterPattern = "*.json", bool detach = false)
{
	std::thread load([onFileSelected, defaultDirectory, filterName, filterPattern]()
		{
			//first create the directory if needed
			std::filesystem::path directory(defaultDirectory);
			std::filesystem::create_directory(directory);

			std::wstring path = L"";
			std::wstring pszName = nostd::StringToWString(filterName);
			std::wstring pszSpec = nostd::StringToWString(filterPattern);
			COMDLG_FILTERSPEC filters[] = { {.pszName = pszName.c_str(), .pszSpec = pszSpec.c_str() } };
			std::pair<COMDLG_FILTERSPEC*, int> filter_info = std::make_pair<COMDLG_FILTERSPEC*, int>(filters, _countof(filters));
			if (!OpenFileDialog(path, std::filesystem::absolute(directory), L"", &filter_info)) return;
			if (path.empty()) return;

			onFileSelected(path);
		}
	);
	if (detach)
	{
		load.detach();
	}
	else
	{
		load.join();
	}
}

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

inline void ImDrawJsonInputText(nlohmann::json& json, std::string att)
{
	if (ImGui::InputText("##", json.at(att).get_ptr<std::string*>()))
	{
		nostd::trim(json.at(att).get_ref<std::string&>());
	}
}

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

inline void DrawItemWithEnabledState(std::function<void()> draw, bool enabled)
{
	if (!enabled)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}
	draw();
	if (!enabled)
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}
}

#endif
