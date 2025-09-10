#include "pch.h"
#include "RightPanelComponent.h"
#include <imgui.h>

RightPanelComponent::RightPanelComponent(std::string panel, std::vector<std::string> nonEdAtts, std::vector<std::string> detailTabs, std::vector<std::string> noDetailTabs)
{
	panelName = panel;
	nonEditableAttributes = nonEdAtts;
	detailAbleTabs = detailTabs;
	noDetailAbleTabs = noDetailTabs;
	selectedTab = detailTabs.at(0);
}

void RightPanelComponent::Destroy()
{
	assetsJsons.clear();
	assetsConditioner.clear();
}

void RightPanelComponent::DrawAttributes()
{
	std::vector<std::shared_ptr<JObject>> jobs;
	for (auto& uuid : editables)
	{
		jobs.push_back(assetsJsons.at(uuid));
	}
	{
		for (auto& att : drawersOrder)
		{
			if (!drawers.contains(att) || !drawers.at(att)) continue;

			auto& drawer = drawers.at(att);
			drawer(att, jobs);
		}
	}
}

void RightPanelComponent::BuildAssetsTree(std::map<std::string, std::any>& assets, std::tuple<std::string, std::vector<std::string>>& uuidParts)
{
	std::string& uuid = std::get<0>(uuidParts);
	std::vector<std::string>& parts = std::get<1>(uuidParts);

	std::string part = *parts.begin();
	if (!assets.contains(part))
	{
		assets.insert_or_assign(part, std::map<std::string, std::any>());
	}

	if (parts.begin() != parts.end() - 1)
	{
		std::map<std::string, std::any>& child = std::any_cast<std::map<std::string, std::any>&>(assets.at(part));
		std::tuple<std::string, std::vector<std::string>> nextParts = std::make_tuple(uuid, std::vector<std::string>());
		std::copy(parts.begin() + 1, parts.end(), std::back_inserter(std::get<1>(nextParts)));
		BuildAssetsTree(child, nextParts);
	}
}

bool RightPanelComponent::HasSelectedChildren(std::map<std::string, std::any>& dump, std::string path)
{
	bool ret = false;
	for (auto it = dump.begin(); it != dump.end(); it++)
	{
		std::map<std::string, std::any>& child = std::any_cast<std::map<std::string, std::any>&>(it->second);
		std::string p = path + (path.empty() ? "" : "/") + it->first;

		if (child.empty())
		{
			std::string uuid = it->first;
			if (selected.contains(uuid)) return true;
		}
		else
		{
			ret |= HasSelectedChildren(child, path);
		}
	}

	return ret;
}

void RightPanelComponent::DrawTabs(std::function<void(std::string)> onChangeTab)
{
	std::vector<std::string>& tabs = (selected.size() == 0) ? noDetailAbleTabs : detailAbleTabs;
	std::string selectedTab = selectedTab;

	std::string tabBarName = "tabBar-" + panelName;
	ImGui::BeginTabBar(tabBarName.c_str());
	{
		for (auto& tabTitle : tabs)
		{
			ImGuiTabItemFlags_ flag = (tabTitle == selectedTab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
			if (flag == ImGuiTabItemFlags_SetSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.230f, 0.230f, 0.230f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.230f, 0.230f, 0.230f, 1.0f));
			}
			if (ImGui::TabItemButton(tabTitle.c_str(), flag))
			{
				onChangeTab(tabTitle);
			}
			if (flag == ImGuiTabItemFlags_SetSelected)
			{
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}
		}
	}
	ImGui::EndTabBar();
}

