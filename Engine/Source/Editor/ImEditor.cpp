#include "pch.h"
#include <imgui.h>
#include <ImEditor.h>

namespace ImGui
{
	bool DrawComboSelection(UUIDName selected, std::vector<UUIDName> selectables, std::function<void(UUIDName)> onSelect, std::string label)
	{
		bool ret = false;
		int current_item = static_cast<int>(std::find(selectables.begin(), selectables.end(), selected) - selectables.begin());
		std::string& preview = std::get<1>(selected);
		if (ImGui::BeginCombo(label.c_str(), preview.c_str()))
		{
			Editor::NonGameMode = true;
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

	bool DrawComboSelection(std::string selected, std::vector<std::string> selectables, std::function<void(std::string)> onSelect, std::string label)
	{
		bool ret = false;
		int current_item = static_cast<int>(std::find(selectables.begin(), selectables.end(), selected) - selectables.begin());
		if (ImGui::BeginCombo(label.c_str(), selectables[current_item].c_str()/*, ImGuiComboFlags_PopupAlignLeft*/))
		{
			Editor::NonGameMode = true;
			for (int i = 0; i < selectables.size(); i++)
			{
				if (ImGui::Selectable((selectables[i] == "") ? "##" : selectables[i].c_str(), current_item == i, (current_item == i) ? ImGuiSelectableFlags_Highlight : 0)) {
					ret = true;
					onSelect(selectables[i]);
				}
				if (current_item == i)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		return ret;
	}

	void ItemLabel(std::string_view title, ItemLabelFlag flags)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		const ImVec2 lineStart = ImGui::GetCursorScreenPos();
		const ImGuiStyle& style = ImGui::GetStyle();
		float fullWidth = ImGui::GetContentRegionAvail().x;
		float itemWidth = ImGui::CalcItemWidth() + style.ItemSpacing.x;
		ImVec2 textSize = ImGui::CalcTextSize(title.data(), title.data() + title.size());
		ImRect textRect;
		textRect.Min = ImGui::GetCursorScreenPos();
		if (flags & ItemLabelFlag::Right)
			textRect.Min.x = textRect.Min.x + itemWidth;
		textRect.Max = textRect.Min;
		textRect.Max.x += fullWidth - itemWidth;
		textRect.Max.y += textSize.y;

		ImGui::SetCursorScreenPos(textRect.Min);

		ImGui::AlignTextToFramePadding();
		// Adjust text rect manually because we render it directly into a drawlist instead of using public functions.
		textRect.Min.y += window->DC.CurrLineTextBaseOffset;
		textRect.Max.y += window->DC.CurrLineTextBaseOffset;

		ItemSize(textRect);
		if (ItemAdd(textRect, window->GetID(title.data(), title.data() + title.size())))
		{
			RenderTextEllipsis(ImGui::GetWindowDrawList(), textRect.Min, textRect.Max, textRect.Max.x,
				textRect.Max.x, title.data(), title.data() + title.size(), &textSize);

			if (textRect.GetWidth() < textSize.x && ImGui::IsItemHovered())
				ImGui::SetTooltip("%.*s", (int)title.size(), title.data());
		}
		if (flags & ItemLabelFlag::Left)
		{
			ImGui::SetCursorScreenPos(
				ImVec2
				{
					textRect.Max.x,
					textRect.Max.y - textSize.y + window->DC.CurrLineTextBaseOffset
				}
			);
			ImGui::SameLine();
		}
		else if (flags & ItemLabelFlag::Right)
			ImGui::SetCursorScreenPos(lineStart);
	}

	void DrawItemWithEnabledState(std::function<void()> draw, bool enabled)
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

	void DrawTextureImage(ImTextureID textureId, unsigned int textureWidth, unsigned int textureHeight) {
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

	bool DrawJsonCheckBox(nlohmann::json& json, const std::string attribute)
	{
		bool value = json.at(attribute);
		if (ImGui::Checkbox("##", &value))
		{
			json[attribute] = value;
			return true;
		}
		return false;
	}

	bool DrawFromFloat(nlohmann::json& json, const std::string attribute, std::string label)
	{
		float value = json.at(attribute);
		if (ImGui::InputFloat(label.c_str(), &value)) { json[attribute] = value; return true; }
		return false;
	}

	bool DrawFromInt(nlohmann::json& json, const std::string attribute, std::string label)
	{
		int value = json.at(attribute);
		if (ImGui::InputInt(label.c_str(), &value)) { json[attribute] = value; return true; }
		return false;
	}

	bool DrawFromUInt(nlohmann::json& json, const std::string attribute, std::string label)
	{
		int value = json.at(attribute);
		if (ImGui::InputInt(label.c_str(), &value)) { value = max(0, value); json[attribute] = value; return true; }
		return false;
	}

	void DrawDynamicArray(
		std::string label,
		nlohmann::json& arr,
		std::function<void(nlohmann::json&, unsigned int)> insert,
		std::function<void(nlohmann::json&, unsigned int)> remove,
		std::function<void(nlohmann::json&, unsigned int)> draw,
		unsigned int maxItems, unsigned int minItems)
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
						remove(arr, i);
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

	bool DrawJsonInputText(nlohmann::json& json, std::string att)
	{
		if (ImGui::InputText("##", json.at(att).get_ptr<std::string*>()))
		{
			nostd::trim(json.at(att).get_ref<std::string&>());
			return true;
		}
		return false;
	}

	bool OpenFileDialog(std::wstring& path, std::wstring defaultDirectory, std::wstring defaultFileName, std::vector<std::pair<std::wstring, std::wstring>>& specs)
	{
		IFileOpenDialog* p_file_open = nullptr;
		bool are_all_operation_success = false;

		while (!are_all_operation_success)
		{
			HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
				IID_IFileOpenDialog, reinterpret_cast<void**>(&p_file_open));
			if (FAILED(hr))
				break;

			if (specs.empty())
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
				std::vector<COMDLG_FILTERSPEC> open_filter(specs.size());
				for (unsigned int i = 0; i < specs.size(); i++)
				{
					open_filter[i].pszName = specs[i].first.c_str();
					open_filter[i].pszSpec = specs[i].second.c_str();
				}
				hr = p_file_open->SetFileTypes(static_cast<unsigned int>(open_filter.size()), open_filter.data());
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

	void OpenFile(std::function<void(std::filesystem::path)> onFileSelected, std::string defaultDirectory, std::vector<std::string> filterName, std::vector<std::string> filterPattern, bool detach)
	{
		std::thread load([onFileSelected, defaultDirectory, filterName, filterPattern]()
			{
				//first create the directory if needed
				std::filesystem::path directory(defaultDirectory);
				std::filesystem::create_directory(directory);

				std::wstring path = L"";
				std::vector<std::pair<std::wstring, std::wstring>> specs;
				for (unsigned int i = 0; i < filterName.size(); i++)
				{
					std::wstring pszName = nostd::StringToWString(filterName[i]);
					std::wstring pszSpec = nostd::StringToWString(filterPattern[i]);
					specs.push_back(std::make_pair(pszName, pszSpec));
				}
				if (!OpenFileDialog(path, std::filesystem::absolute(directory), L"", specs)) return;
				if (path.empty()) return;

				onFileSelected(path);
				/*
				std::wstring pszName = nostd::StringToWString(filterName);
				std::wstring pszSpec = nostd::StringToWString(filterPattern);
				COMDLG_FILTERSPEC filters[] = { {.pszName = pszName.c_str(), .pszSpec = pszSpec.c_str() } };
				std::pair<COMDLG_FILTERSPEC*, int> filter_info = std::make_pair<COMDLG_FILTERSPEC*, int>(filters, _countof(filters));
				if (!OpenFileDialog(path, std::filesystem::absolute(directory), L"", &filter_info)) return;
				if (path.empty()) return;

				onFileSelected(path);
				*/
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

	void OpenTemplate(const char* iconCode, UUIDName uuidName)
	{
		ImGui::DrawItemWithEnabledState([uuidName, iconCode]
			{
				if (ImGui::Button(iconCode))
				{
					Editor::OpenTemplateOnNextFrame(std::get<0>(uuidName));
				}
			}, std::get<0>(uuidName) != "");
	}

	void OpenSceneObject(const char* iconCode, UUIDName uuidName)
	{
		ImGui::DrawItemWithEnabledState([uuidName, iconCode]
			{
				if (ImGui::Button(iconCode))
				{
					Editor::OpenSceneObjectOnNextFrame(std::get<0>(uuidName));
				}
			}, std::get<0>(uuidName) != "");
	}
};

