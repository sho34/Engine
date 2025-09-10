#pragma once
#include <string>
#include <vector>
#include <set>
#include <NoStd.h>
#include <unordered_map>
#include <map>
#include <JTypes.h>

struct RightPanelComponent
{
	RightPanelComponent(std::string panel, std::vector<std::string> nonEdAtts, std::vector<std::string> detailTabs, std::vector<std::string> noDetailTabs);

	bool dirtyAssetsTree = true;
	std::string panelName;
	std::vector<std::string> nonEditableAttributes;
	std::vector<std::string> detailAbleTabs;
	std::vector<std::string> noDetailAbleTabs;
	std::string selectedTab;
	std::string selectedNextFrame;
	std::set<std::string> selected;
	std::set<std::string> editables;
	nostd::VectorSet<std::string> drawersOrder;
	std::unordered_map<std::string, JEdvEditorDrawerFunction> drawers;
	std::map<std::string, std::any> assets;
	std::map<std::string, std::string> assetsNames;
	std::map<std::string, std::shared_ptr<JObject>> assetsJsons;
	std::map<std::string, std::function<bool(std::shared_ptr<JObject>)>> assetsConditioner;

	void Destroy();

	void DrawAttributes();

	void BuildAssetsTree(std::map<std::string, std::any>& assets, std::tuple<std::string, std::vector<std::string>>& uuidParts);

	bool HasSelectedChildren(std::map<std::string, std::any>& dump, std::string path);

	void DrawAssetTreeNodes(std::map<std::string, std::any>& dump, std::string path, auto OnPick, auto OnDelete)
	{
		std::map<std::string, std::any> nodes;
		std::copy_if(dump.begin(), dump.end(), std::inserter(nodes, nodes.begin()), [](auto& pair)
			{
				std::map<std::string, std::any>& child = std::any_cast<std::map<std::string, std::any>&>(pair.second);
				return !child.empty();
			}
		);

		for (auto& [first, second] : nodes)
		{
			std::map<std::string, std::any>& child = std::any_cast<std::map<std::string, std::any>&>(second);
			std::string p = path + (path.empty() ? "" : "/") + first;
			if (HasSelectedChildren(child, p))
				ImGui::SetNextItemOpen(true);
			if (ImGui::TreeNodeEx(first.c_str()))
			{
				DrawAssetTreeNodes(child, p, OnPick, OnDelete);
				ImGui::TreePop();
			}
		}

		std::map<std::string, std::any> leafs;
		std::vector<std::string> orderedLeafs;
		std::copy_if(dump.begin(), dump.end(), std::inserter(leafs, leafs.begin()), [](auto& pair)
			{
				std::map<std::string, std::any>& child = std::any_cast<std::map<std::string, std::any>&>(pair.second);
				return child.empty();
			}
		);
		std::transform(leafs.begin(), leafs.end(), std::back_inserter(orderedLeafs), [](auto& pair) { return pair.first; });
		std::sort(orderedLeafs.begin(), orderedLeafs.end(), [this](std::string uuidA, std::string uuidB)
			{
				std::shared_ptr<JObject> assetA = assetsJsons.at(uuidA);
				std::shared_ptr<JObject> assetB = assetsJsons.at(uuidB);
				return assetA->at("name") < assetB->at("name");
			}
		);

		for (std::string uuid : orderedLeafs)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.15f));
			{
				ImGui::PushID((std::string("select-") + uuid).c_str());
				{
					bool checked = selected.contains(uuid);
					std::shared_ptr<JObject> asset = assetsJsons.at(uuid);
					bool enabled = true;
					for (auto& [_, cond] : assetsConditioner)
					{
						enabled &= cond(asset);
						if (!enabled) break;
					}
					ImGui::DrawItemWithEnabledState([this, uuid, &checked]
						{
							if (ImGui::Checkbox("##", &checked))
							{
								if (checked)
								{
									selected.insert(uuid);
									assetsConditioner.insert_or_assign(uuid, assetsJsons.at(uuid)->GetAssetsConditioner());
								}
								else
								{
									selected.erase(uuid);
									assetsConditioner.erase(uuid);
								}
							}
						}, checked || enabled
					);
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::PushID((std::string("delete-") + uuid).c_str());
				{
					if (ImGui::Button(ICON_FA_TIMES, ImVec2(16.0f, 16.0f)))
					{
						OnDelete(uuid);
					}
				}
				ImGui::PopID();
			}
			ImGui::PopStyleVar();

			ImGui::SameLine();
			std::string name = assetsNames.at(uuid);
			std::string nodeID = "node-" + uuid;
			ImGui::PushID(nodeID.c_str());
			{
				if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf))
				{
					if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiPopupFlags_MouseButtonLeft))
					{
						OnPick(uuid);
					}
					ImGui::TreePop();
				}
			}
			ImGui::PopID();
		}
	}

	void DrawAssetsTree(auto GetObjects, auto GetPanelObject, auto OnPick, auto OnDelete, std::string ignorePrefix)
	{
		if (assets.empty() || dirtyAssetsTree)
		{
			assets.clear();
			assetsNames.clear();
			for (UUIDName uuidName : GetObjects())
			{
				std::string uuid = std::get<0>(uuidName);
				std::string path = std::get<1>(uuidName);

				if (!ignorePrefix.empty())
				{
					path = std::regex_replace(path, std::regex(ignorePrefix), "");
				}

				std::vector<std::string> splitParts = nostd::split(path, "/");
				assetsNames.insert_or_assign(uuid, splitParts.back());
				assetsJsons.insert_or_assign(uuid, GetPanelObject(uuid));
				splitParts.pop_back();
				splitParts.push_back(uuid);
				std::tuple<std::string, std::vector<std::string>> uuidParts = std::make_tuple(uuid, splitParts);
				BuildAssetsTree(assets, uuidParts);
			}
			dirtyAssetsTree = false;
		}

		DrawAssetTreeNodes(assets, "", OnPick, OnDelete);
	}

	void DrawTabs(std::function<void(std::string)> onChangeTab);

	void DrawPanel(ImVec2 pos, ImVec2 size,
		auto menuItems,
		auto GetObjects,
		auto GetPanelObject,
		auto OnChangeTab,
		auto MatchAttributes,
		auto SendEditorPreview,
		auto OnNew,
		auto OnDelete
	)
	{
		ImGui::BeginChild((panelName + "panel").c_str(), size, ImGuiChildFlags_None);
		{
			DrawTabs(OnChangeTab);
			if (selectedTab == detailAbleTabs.at(0))
			{
				std::vector<std::string> selectables = { "New" };
				std::transform(menuItems.begin(), menuItems.end(), std::back_inserter(selectables), [](auto& p) { return p.second; });
				ImGui::PushID((panelName + "-new").c_str());
				ImGui::DrawComboSelection(selectables.at(0), selectables, [OnNew, menuItems](std::string value)
					{
						auto getKey = [menuItems, value]
							{
								for (auto it : menuItems) {
									if (it.second == value) return it.first;
								}
								return menuItems.begin()->first; //sorry but this hacky thing is because i didn't have better options. as it will also be captured as ""
							};
						if (value != "")
							OnNew(getKey());
					}, "Add");
				ImGui::PopID();
			}

			float padH = 57.0f;
			ImVec2 scrollableSize = ImVec2(size.x, size.y - padH);
			if (ImGui::BeginChild((panelName + "panel").c_str(), scrollableSize, ImGuiChildFlags_AlwaysUseWindowPadding))
			{
				std::string uuidToPick = ""; //why this, why here, i don't recall?

				if (selectedTab == detailAbleTabs.at(0))
				{
					DrawAssetsTree(GetObjects, GetPanelObject,
						[&uuidToPick, this, MatchAttributes, SendEditorPreview](auto toPick)
						{
							uuidToPick = toPick;
							selected.insert(uuidToPick);
							editables.clear();
							editables.insert(uuidToPick);
							selectedTab = detailAbleTabs.at(1);
							MatchAttributes();
							SendEditorPreview(uuidToPick);
						}, OnDelete, "");
				}
				else
				{
					DrawAttributes();
				}
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();
	}

	template<typename Type>
	void CreateEditableAttributesToMatch(auto GetTypes, auto GetJson, auto GetAttributes, auto GetDrawers)
	{
		std::set<Type> objectTypes;
		//create an attributes counter map for all the attributes in every selected scene object
		std::map<std::string, unsigned int> attributesMatch;
		for (auto& uuid : editables)
		{
			//at the same time gather the types of the objects, this will be used later for sorting purposes
			objectTypes.insert(GetTypes(uuid));
			std::shared_ptr<JObject> so = GetJson(uuid);
			for (auto it = so->begin(); it != so->end(); it++)
			{
				attributesMatch[it.key()]++;
			}
		}

		//remove attributes that are not part of every selected object
		for (auto& att : nonEditableAttributes) attributesMatch.erase(att);
		for (auto it = attributesMatch.begin(); it != attributesMatch.end(); )
		{
			if (it->second < editables.size())
			{
				it = attributesMatch.erase(it);
			}
			else
			{
				it++;
			}
		}

		//gather the attributes orders
		drawersOrder.clear();
		for (auto& type : objectTypes)
		{
			auto attlist = GetAttributes(type);
			for (auto& att : attlist)
			{
				if (attributesMatch.contains(att.first) && drawersOrder.count(att.first) == 0ULL)
					drawersOrder.insert(att.first);
			}
		}

		//now construct the callbacks using the orders
		drawers.clear();
		std::map<Type, std::map<std::string, JEdvEditorDrawerFunction>> typeDrawers;
		for (auto& type : objectTypes)
		{
			typeDrawers.insert_or_assign(type, GetDrawers(type));
		}

		for (auto& att : drawersOrder)
		{
			if (!attributesMatch.contains(att) || drawersOrder.count(att) == 0ULL) continue;

			for (auto& type : objectTypes)
			{
				//auto attDrawers = GetDrawers(type);
				//if (!attDrawers.contains(att)) continue;
				if (!typeDrawers.at(type).contains(att)) continue;

				//auto& drawer = attDrawers.at(att);
				auto& drawer = typeDrawers.at(type).at(att);
				drawers.insert_or_assign(att, drawer);
			}
		}
	}
};
