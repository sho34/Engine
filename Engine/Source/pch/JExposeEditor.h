#pragma once

namespace Templates
{
	struct ShaderInstance;
	extern std::vector<UUIDName> GetMaterialsUUIDsNames();
	extern std::vector<UUIDName> GetMeshesUUIDsNames();
	extern std::vector<UUIDName> GetModel3DsUUIDsNames();
	extern std::vector<UUIDName> GetRenderPasssUUIDsNames();
	extern std::vector<UUIDName> GetShadersUUIDsNames();
	extern std::vector<UUIDName> GetSoundsUUIDsNames();
	extern std::vector<UUIDName> GetTexturesUUIDsNames();
	extern std::string GetMeshName(std::string uuid);
	extern std::string GetModel3DName(std::string uuid);
	extern std::string GetMaterialName(std::string uuid);
	extern std::string GetShaderName(std::string uuid);
	extern std::string GetSoundName(std::string uuid);
	extern std::string GetTextureName(std::string uuid);
	extern std::shared_ptr<ShaderInstance> FindShaderInstance(std::string uuid);
};

namespace Scene
{
	extern std::vector<UUIDName> GetCamerasUUIDNames();
	extern std::vector<UUIDName> GetLightsUUIDNames();
	extern std::vector<UUIDName> GetRenderablesUUIDNames();
	extern std::vector<UUIDName> GetSoundEffectsUUIDNames();
};

namespace Editor
{
	extern void MarkTemplatesPanelAssetsAsDirty();
};

const int defaultTableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX;

inline void drawAnimationController(
	std::function<bool()> animationsArePlaying,
	std::function<void(bool)> setPlayAnimation,
	std::function<void(float)> setAnimationTime,
	std::function<float()> getAnimationTimeFactor,
	std::function<void(float)> setAnimationTimeFactor,
	std::function<void()> gotoPrevAnimation,
	std::function<void()> gotoNextAnimation,
	std::function<bool()> animationsAreLooping,
	std::function<void(bool)> setAnimationLoop
)
{
	if (ImGui::Button(ICON_FA_BACKWARD))
	{
		gotoPrevAnimation();
	}

	ImGui::SameLine();
	if (animationsArePlaying())
	{
		if (ImGui::Button(ICON_FA_PAUSE)) { setPlayAnimation(false); }
	}
	else
	{
		if (ImGui::Button(ICON_FA_PLAY)) { setPlayAnimation(true); }
	}

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_STOP))
	{
		setPlayAnimation(false);
		setAnimationTime(0.0f);
	}

	float animationTimeFactor = getAnimationTimeFactor();
	ImGui::SameLine();
	if (animationTimeFactor > 0.0f)
	{
		if (ImGui::Button(ICON_FA_UNDO))
		{
			setAnimationTimeFactor(-animationTimeFactor);
		}
	}
	else if (animationTimeFactor < 0.0f)
	{
		if (ImGui::Button(ICON_FA_REDO))
		{
			setAnimationTimeFactor(-animationTimeFactor);
		}
	}
	else
	{
		if (ImGui::Button(ICON_FA_SYNC))
		{
			setAnimationTimeFactor(1.0f);
		}
	}

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_FORWARD))
	{
		gotoNextAnimation();
	}

	ImGui::SameLine();
	auto loop = animationsAreLooping();
	if (loop)
	{
		if (ImGui::Button(ICON_FA_PAUSE_CIRCLE))
		{
			setAnimationLoop(false);
		}
	}
	else
	{
		if (ImGui::Button(ICON_FA_CIRCLE))
		{
			setAnimationLoop(true);
		}
	}
}

inline void drawAudioController(
	std::function<bool()> isPlaying,
	std::function<bool()> isPaused,
	std::function<void()> play,
	std::function<void()> stop,
	std::function<void()> pause,
	std::function<float()> getTime,
	std::function<float()> getDuration
)
{
	if (!isPlaying())
	{
		if (ImGui::Button(ICON_FA_PLAY))
		{
			play();
		}
	}
	else
	{
		if (ImGui::Button(ICON_FA_PAUSE))
		{
			pause();
		}
	}

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_STOP))
	{
		stop();
	}

	std::ostringstream ossTime;
	ossTime << std::fixed << std::setprecision(2) << getTime();
	std::string roundedTimeStr = ossTime.str();
	std::ostringstream ossDuration;
	ossDuration << std::fixed << std::setprecision(2) << getDuration();
	std::string roundedDurationStr = ossDuration.str();

	ImGui::SameLine();
	std::string timeStr = roundedTimeStr + "s / " + roundedDurationStr + "s";
	ImGui::Text(timeStr.c_str());
}

#include "JExposeTypes.h"

template<typename T, JsonToEditorValueType J>
JEdvDrawerFunction DrawValue() { return nullptr; }

template<typename T, JsonToEditorValueType J>
JEdvDrawerFunction DrawVector() { return nullptr; }

template<typename Ta, typename Tb>
JEdvDrawerFunction DrawMap() { return nullptr; }

template<typename E, JsonToEditorValueType J>
JEdvDrawerFunction DrawEnum(
	std::map<E, std::string>& EtoS,
	std::map<std::string, E>& StoE
) {
	if (J == jedv_t_hidden) return nullptr;
	return [&EtoS, &StoE](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			auto allSame = [attribute, &json]()
				{
					std::set<std::string> s;
					for (auto& j : json)
					{
						s.insert(j->at(attribute));
						if (s.size() > 1) return false;
					}
					return true;
				};
			auto update = [attribute, &json](auto value)
				{
					nlohmann::json patch = { {attribute,value} };
					for (auto& j : json)
					{
						j->JUpdate(patch);
					}
				};
			bool allEq = allSame();
			ImGui::PushID(attribute.c_str());

			std::vector<std::string> options{};
			std::string selected = "";
			if (!allEq)
			{
				options.push_back("");
			}
			else
			{
				selected = json.at(0)->at(attribute);
			}
			std::transform(StoE.begin(), StoE.end(), std::back_inserter(options), [](auto& p) { return p.first; });

			std::string tableName = "tables-" + attribute + "-table";
			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(attribute.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::DrawComboSelection(selected, options, [update](std::string newOption)
					{
						update(newOption);
					}
				);
				ImGui::EndTable();
			}
			ImGui::PopID();
		};
}

#include <regex>
#include <Material/SamplerDesc.h>
#include <Material/BlendDesc.h>
#include <ShaderMaterials.h>
#include <ImEditor.h>
#include <Shader/ShaderInstance.h>
#include <NoStd.h>
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>
#include <Lights/Lights.h>
#include <Lights/ShadowMap.h>
#include <Sound/SoundFX.h>

inline void EditorDrawFloat(std::string attribute, std::shared_ptr<JObject>& json, const char* format = "%.3f", std::function<float(float)> cb = [](float v) {return v; })
{
	ImGui::TableNextRow();

	ImGui::TableSetColumnIndex(0);
	ImGui::Text(attribute.c_str());

	ImGui::TableSetColumnIndex(1);
	ImGui::PushID((std::string(json->at("uuid")) + "-" + attribute).c_str());
	float value = float(json->at(attribute));
	if (ImGui::InputFloat("", &value, 0.0f, 0.0f, format))
	{
		value = cb(value);
		nlohmann::json patch = { {attribute,value} };
		json->JUpdate(patch);
	}
	ImGui::PopID();
}

inline void EditorDrawFloatArray(std::string attribute, std::vector<std::shared_ptr<JObject>>& json, std::vector<std::string> labels, const char* format = "%.3f")
{
	auto updateValues = [&json, attribute](unsigned int i, float value)
		{
			for (auto& j : json)
			{
				nlohmann::json cp = j->at(attribute);
				cp.at(i) = value;
				nlohmann::json patch = { { attribute, cp } };
				j->JUpdate(patch);
			}
		};

	std::vector<std::set<float>> fs;
	nostd::VecN_push_back(static_cast<unsigned int>(labels.size()), fs);

	for (auto& j : json)
		for (unsigned int i = 0; i < labels.size(); i++)
			fs[i].insert(float(j->at(attribute).at(i)));

	ImGui::PushID((std::string(json[0]->at("uuid")) + "-" + attribute).c_str());
	std::string tableName = "tables-" + attribute + "-table";
	if (ImGui::BeginTable(tableName.c_str(), static_cast<int>(labels.size()), defaultTableFlags))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text(attribute.c_str());
		ImGui::TableNextRow();

		for (unsigned int i = 0U; i < labels.size(); i++)
		{
			ImGui::TableSetColumnIndex(i);

			if (fs[i].size() > 1)
			{
				std::string value = "";
				if (ImGui::InputText(labels[i].c_str(), &value, ImGuiInputTextFlags_CharsDecimal))
				{
					value = std::regex_replace(value, std::regex(".+-*/"), "");
					if (value.size() > 0ULL)
					{
						updateValues(i, (value.size() > 0ULL) ? std::stof(value.c_str()) : 0.0f);
					}
				}
			}
			else
			{
				float value = json.at(0)->at(attribute).at(i);
				if (ImGui::InputFloat(labels[i].c_str(), &value, 0.0f, 0.0f, format))
				{
					updateValues(i, value);
				}
			}
		}

		ImGui::EndTable();
	}
	ImGui::PopID();
}

inline void EditorDrawFloatAngleArray(std::string attribute, std::vector<std::shared_ptr<JObject>>& json, std::vector<std::string> labels, const char* format = "%.3f")
{
	auto updateValues = [&json, attribute](unsigned int i, float value)
		{
			for (auto& j : json)
			{
				nlohmann::json cpy = j->at(attribute);
				cpy.at(i) = XMConvertToDegrees(value);
				nlohmann::json patch = { { attribute, cpy } };
				j->JUpdate(patch);
			}
		};

	std::vector<std::set<float>> fs;
	nostd::VecN_push_back(static_cast<unsigned int>(labels.size()), fs);

	for (auto& j : json)
		for (unsigned int i = 0; i < labels.size(); i++)
			fs[i].insert(float(j->at(attribute).at(i)));

	ImGui::PushID((std::string(json[0]->at("uuid")) + "-" + attribute).c_str());
	std::string tableName = "tables-" + attribute + "-table";
	if (ImGui::BeginTable(tableName.c_str(), static_cast<int>(labels.size()), defaultTableFlags))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text(attribute.c_str());
		ImGui::TableNextRow();

		for (unsigned int i = 0U; i < labels.size(); i++)
		{
			ImGui::TableSetColumnIndex(i);

			ImGui::PushID((std::string("sliders") + labels[i]).c_str());
			if (fs[i].size() > 1)
			{
				std::string value = "";
				if (ImGui::InputText(labels[i].c_str(), &value, ImGuiInputTextFlags_CharsDecimal))
				{
					value = std::regex_replace(value, std::regex(".+-*/"), "");
					if (value.size() > 0ULL)
					{
						updateValues(i, (value.size() > 0ULL) ? std::stof(value.c_str()) : 0.0f);
					}
				}
			}
			else
			{
				float minDeg = -180.0f;
				float maxDeg = 180.0f;
				float value = XMConvertToRadians(json.at(0)->at(attribute).at(i));
				if (ImGui::SliderAngle(labels[i].c_str(), &value, minDeg, maxDeg, format, ImGuiSliderFlags_AlwaysClamp))
				{
					updateValues(i, value);
				}
			}
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	ImGui::PopID();
}

inline void EditorDrawVector(
	std::string attribute,
	std::vector<std::shared_ptr<JObject>>& json,
	const char* iconCode,
	std::function<std::vector<UUIDName>()> GetSelectableItems,
	std::function<std::string(std::string)> GetNameFromUUID,
	std::function<void(const char*, UUIDName)> OpenItem = [](const char* icon, UUIDName item) {},
	std::function<bool(unsigned int, UUIDName)> FilterItem = [](unsigned int index, UUIDName item) {return true; },
	std::function<bool(unsigned int, unsigned int, unsigned int, UUIDName, UUIDName)> CanSwap = [](unsigned int index1, unsigned int index2, unsigned int numItems, UUIDName item1, UUIDName item2) { return true; }
)
{
	auto allSame = [attribute, &json]()
		{
			//check sizes are the same
			std::set<unsigned int> sz;
			for (auto& j : json)
			{
				sz.insert((unsigned int)(j->at(attribute).size()));
				if (sz.size() > 1) return false;
			}

			//check contents are the same
			std::map<unsigned int, std::string> orderedUUIDs;
			for (auto& j : json)
			{
				for (unsigned int i = 0; i < j->at(attribute).size(); i++)
				{
					std::string uuid = std::string(j->at(attribute).at(i));
					if (!orderedUUIDs.contains(i))
					{
						orderedUUIDs.insert_or_assign(i, uuid);
						continue;
					}
					if (orderedUUIDs.at(i) != uuid) return false;
				}
			}

			return true;
		};
	auto setValue = [attribute, &json](unsigned int index, std::string nuuid)
		{
			for (auto& j : json)
			{
				nlohmann::json cpy = j->at(attribute);
				cpy.at(index) = nuuid;
				nlohmann::json patch = { { attribute, cpy } };
				j->JUpdate(patch);
			}
		};
	auto append = [attribute, &json](unsigned int index)
		{
			auto push_back = [attribute, &json](std::string value)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.push_back("");
						nlohmann::json patch = { { attribute, cpy } };
						j->JUpdate(patch);
					}
				};
			auto fit = [attribute, &json](unsigned int index, std::string value)
				{
					auto& v0 = json.at(0)->at(attribute);
					std::vector<std::string> v1;
					v1.insert(v1.begin(), v0.begin(), std::next(v0.begin(), index));
					v1.push_back(value);
					v1.insert(v1.end(), std::next(v0.begin(), index), v0.end());

					for (auto& j : json)
					{
						nlohmann::json patch = { { attribute, v1 } };
						j->JUpdate(patch);
					}
				};

			unsigned int size = static_cast<unsigned int>(json.at(0)->at(attribute).size());
			//only push_back if the vector is empty or if we are gonna push after the last item, otherwise we need to fit the values between
			if (size == 0U || index == size)
			{
				push_back("");
			}
			else
			{
				fit(index, "");
			}
		};
	auto remove = [attribute, &json](unsigned int index)
		{
			for (auto& j : json)
			{
				nlohmann::json value = j->at(attribute);
				value.erase(index);
				nlohmann::json patch = { { attribute, value } };
				j->JUpdate(patch);
			}
		};
	auto swap = [attribute, &json](unsigned int from, unsigned int to)
		{
			std::string uuidFrom = json.at(0)->at(attribute).at(from);
			std::string uuidTo = json.at(0)->at(attribute).at(to);
			for (auto& j : json)
			{
				nlohmann::json value = j->at(attribute);
				value.at(from) = uuidTo;
				value.at(to) = uuidFrom;
				nlohmann::json patch = { {attribute,value} };
				j->JUpdate(patch);
			}
		};
	auto reset = [attribute, &json](std::string value)
		{
			for (auto& j : json)
			{
				j->JUpdate({ {attribute,nlohmann::json::array({ value })} });
			}
		};
	ImGui::PushID(attribute.c_str());

	int removeIndex = -1;
	std::string resetUUID = "";

	bool allEq = allSame();

	std::vector<UUIDName> items = GetSelectableItems();

	std::string tableName = "tables-" + attribute + "-table";
	if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text(attribute.c_str());
		ImGui::TableSetColumnIndex(1);

		bool disabled = !allEq;

		ImGui::DrawItemWithEnabledState([disabled, append]()
			{
				ImGui::PushID("PlusTop");
				if (ImGui::Button(ICON_FA_PLUS, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f)) && !disabled)
				{
					append(0);
				}
				ImGui::PopID();
			}
		, allEq);

		if (allEq)
		{
			unsigned int numItems = static_cast<unsigned int>(json.at(0)->at(attribute).size());
			auto getItemsInVector = [attribute, numItems, &json, GetNameFromUUID]
				{
					std::vector<UUIDName> itemsIV;
					for (unsigned int i = 0; i < numItems; i++)
					{
						UUIDName uuidName;
						std::string& uuid = std::get<0>(uuidName);
						std::string& name = std::get<1>(uuidName);
						uuid = json.at(0)->at(attribute).at(i);
						if (uuid != "")
						{
							name = GetNameFromUUID(uuid);
						}
						itemsIV.push_back(uuidName);
					}
					return itemsIV;
				};

			std::vector<UUIDName> itemsInVector = getItemsInVector();

			for (unsigned int index = 0U; index < numItems; index++)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				//draw the plus button to append items
				ImGui::PushID((std::string("PlusLeft-") + std::to_string(index)).c_str());
				if (ImGui::Button(ICON_FA_PLUS))
				{
					append(index);
				}
				ImGui::PopID();

				//draw the minus button to delete items
				ImGui::SameLine();
				ImGui::PushID((std::string("DeleteLeft-") + std::to_string(index)).c_str());
				if (ImGui::Button(ICON_FA_TIMES))
				{
					removeIndex = index;
				}
				ImGui::PopID();

				//if there is more than a single item we enable the swap buttons
				if (numItems > 1U)
				{
					bool canSwapUp = index > 0 ? CanSwap(index, index - 1, numItems, itemsInVector[index], itemsInVector[index - 1]) : false;
					bool canSwapDown = (index < numItems - 1) ? CanSwap(index, index + 1, numItems, itemsInVector[index], itemsInVector[index + 1]) : false;

					//draw the swap arrow to move something up
					ImGui::SameLine();
					ImGui::PushID((std::string("UpLeft-") + std::to_string(index)).c_str());
					ImGui::DrawItemWithEnabledState([index, swap, canSwapUp]()
						{
							if (ImGui::Button(ICON_FA_ARROW_UP) && (index != 0) && canSwapUp)
							{
								swap(index, index - 1);
							}
						}
					, index != 0 && canSwapUp);
					ImGui::PopID();

					//draw the swap arrow to move something down
					ImGui::SameLine();
					ImGui::PushID((std::string("DownLeft-") + std::to_string(index)).c_str());
					ImGui::DrawItemWithEnabledState([index, numItems, swap, canSwapDown]()
						{
							if (ImGui::Button(ICON_FA_ARROW_DOWN) && (index != (numItems - 1)) && canSwapDown)
							{
								swap(index, index + 1);
							}
						}
					, index != (numItems - 1) && canSwapDown);
					ImGui::PopID();
				}

				//move to second column where the actual list is displayed
				ImGui::TableSetColumnIndex(1);

				//create a list of uuidnames but apply a filtering criteria
				std::vector<UUIDName> uuidNames;
				std::copy_if(items.begin(), items.end(), std::back_inserter(uuidNames), [index, FilterItem](UUIDName item)
					{
						return FilterItem(index, item);
					}
				);
				//build the final list appending the empty tuple at the beginning
				std::vector<UUIDName> selectables = { std::make_tuple("","") };
				selectables.insert(selectables.end(), uuidNames.begin(), uuidNames.end());

				//get the index of the selected item and get the selected uuidname from it's index
				int selectedIndex = FindSelectableIndex(selectables, json.at(0)->at(attribute), index);
				UUIDName selected = selectedIndex < selectables.size() ? selectables.at(selectedIndex) : std::tie("", "");

				//have a goto button to go to the selected item template//FIX this we might want to go to a scene object too
				ImGui::DrawItemWithEnabledState([selected, iconCode, index, OpenItem]()
					{
						ImGui::PushID((std::string("goto-selected") + std::to_string(index)).c_str());
						//ImGui::OpenTemplate(iconCode, selected);
						OpenItem(iconCode, selected);
						ImGui::PopID();
					}, std::get<0>(selected) != "");
				ImGui::SameLine();

				//draw the actual 
				ImGui::PushID((std::string("selectables-") + std::to_string(index)).c_str());
				ImGui::DrawComboSelection(selected, selectables, [setValue, index](UUIDName option)
					{
						std::string& nuuid = std::get<0>(option);
						setValue(index, nuuid);
					}
				);
				ImGui::PopID();
			}

			if (numItems >= 1U)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(1);
				ImGui::DrawItemWithEnabledState([disabled, numItems, append]()
					{
						ImGui::PushID("PlusBottom");
						if (ImGui::Button(ICON_FA_PLUS, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f)) && !disabled)
						{
							append(numItems);
						}
						ImGui::PopID();
					}
				, allEq);
			}
		}
		else
		{
			std::vector<UUIDName> uuidNames;
			std::copy_if(items.begin(), items.end(), std::back_inserter(uuidNames), [FilterItem](UUIDName item)
				{
					return FilterItem(0, item);
				}
			);
			std::vector<UUIDName> selectables = { std::make_tuple("","") };
			selectables.insert(selectables.end(), uuidNames.begin(), uuidNames.end());

			ImGui::TableSetColumnIndex(1);
			UUIDName& selected = selectables.at(0);

			ImGui::DrawItemWithEnabledState([selected, iconCode]()
				{
					ImGui::PushID((std::string("goto-selected") + std::to_string(0)).c_str());
					ImGui::OpenTemplate(iconCode, selected);
					ImGui::PopID();
				}, std::get<0>(selected) != "");
			ImGui::SameLine();

			ImGui::PushID((std::string("selectables-neq")).c_str());
			ImGui::DrawComboSelection(selected, selectables, [&resetUUID](UUIDName option)
				{
					std::string& nuuid = std::get<0>(option);
					resetUUID = nuuid;
				}
			);
			ImGui::PopID();
		}

		ImGui::EndTable();
	}
	ImGui::PopID();

	if (removeIndex != -1)
		remove(removeIndex);
	if (resetUUID != "")
		reset(resetUUID);
}

inline void EditorDrawColor3(std::string attribute, std::shared_ptr<JObject>& json, std::vector<std::string> labels)
{
	auto update = [attribute, &json](auto value)
		{
			nlohmann::json patch = { {attribute,nlohmann::json::array({value[0],value[1],value[2]})} };
			json->JUpdate(patch);
		};

	float color[3];
	for (unsigned int i = 0; i < 3; i++)
	{
		color[i] = json->at(attribute).at(i);
	}

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::Text(attribute.c_str());

	ImGui::TableSetColumnIndex(1);
	ImGui::PushID((std::string(json->at("uuid")) + "-" + attribute).c_str());
	if (ImGui::ColorEdit3("##", color))
	{
		update(color);
	}
	ImGui::PopID();
}

inline void EditorDrawString(std::string attribute, std::shared_ptr<JObject>& json)
{
	ImGui::TableNextRow();

	ImGui::TableSetColumnIndex(0);
	ImGui::Text(attribute.c_str());

	ImGui::TableSetColumnIndex(1);
	ImGui::PushID((std::string(json->at("uuid")) + "-" + attribute).c_str());
	ImGui::DrawJsonInputText(*json, attribute);
	ImGui::PopID();
}

inline void EditorDrawCheckBox(std::string attribute, std::shared_ptr<JObject>& json)
{
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::Text(attribute.c_str());
	ImGui::TableSetColumnIndex(1);
	ImGui::PushID((std::string(json->at("uuid")) + "-" + attribute).c_str());
	bool value = bool(json->at(attribute));
	if (ImGui::Checkbox("", &value))
	{
		nlohmann::json patch = { {attribute, value} };
		json->JUpdate(patch);
	}
	ImGui::PopID();
}

inline void EditorDrawEnum(std::string attribute, auto strOptions, std::shared_ptr<JObject>& json)
{
	ImGui::TableNextRow();

	ImGui::TableSetColumnIndex(0);
	ImGui::Text(attribute.c_str());

	ImGui::TableSetColumnIndex(1);
	std::string selected = json->at(attribute);
	std::vector<std::string> selectables = nostd::GetKeysFromMap(strOptions);
	ImGui::PushID((std::string(json->at("uuid")) + "-" + attribute).c_str());
	ImGui::DrawComboSelection(selected, selectables, [attribute, &json](std::string option)
		{
			nlohmann::json patch = { {attribute, option} };
			json->JUpdate(patch);
		}
	);
	ImGui::PopID();
}

inline void EditorDrawSelectableInt(std::string attribute, std::vector<std::string> selectables, std::shared_ptr<JObject>& json)
{
	ImGui::TableNextRow();

	ImGui::TableSetColumnIndex(0);
	ImGui::Text(attribute.c_str());

	ImGui::TableSetColumnIndex(1);
	ImGui::PushID((std::string(json->at("uuid")) + " - " + attribute).c_str());
	int selectedV = int(json->at(attribute));
	int lastSelectableV = std::stoi(selectables.back());
	if (selectedV > lastSelectableV)
	{
		selectedV = lastSelectableV;
		nlohmann::json patch = { {attribute, selectedV} };
		json->JUpdate(patch);
	}
	std::string selected = std::to_string(selectedV);
	ImGui::DrawComboSelection(selected, selectables, [attribute, &json](std::string option)
		{
			unsigned int selected = std::stoi(option);
			nlohmann::json patch = { {attribute, selected} };
			json->JUpdate(patch);
		}
	);
	ImGui::PopID();
}

template<>
inline JEdvDrawerFunction DrawEnum<LightType, jedv_t_lighttype>(
	std::map<LightType, std::string>& EtoS,
	std::map<std::string, LightType>& StoE
)
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			auto getLights = [&json]
				{
					std::vector<std::shared_ptr<Light>> lights;
					std::transform(json.begin(), json.end(), std::back_inserter(lights), [](auto& j)
						{
							return std::dynamic_pointer_cast<Light>(j);
						}
					);
					return lights;
				};
			auto drawLightName = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawString("name", j);
				};
			auto drawLightType = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawEnum("lightType", StringToLightType, j);
				};
			auto drawPosition = [](auto& light)
				{
					std::vector<std::shared_ptr<JObject>> lightV = { light };
					EditorDrawFloatArray("position", lightV, { "x","y","z" });
				};
			auto drawRotation = [](auto& light)
				{
					std::vector<std::shared_ptr<JObject>> lightV = { light };
					EditorDrawFloatAngleArray("rotation", lightV, { "pitch","yaw","roll" });
				};
			auto drawColor = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawColor3("color", j, { "r","g","b" });
				};
			auto drawBrightness = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawFloat("brightness", j);
				};
			auto drawAttenuation = [](auto& light)
				{
					std::vector<std::shared_ptr<JObject>> lightV = { light };
					EditorDrawFloatArray("attenuation", lightV, { "C","X","X\xC2\xB2" });
				};
			auto drawDirectionalDistance = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawFloat("dirDist", j, "%.3f", [](float v) {return max(1.0f, v); });
				};
			auto drawConeAngle = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawFloat("coneAngle", j, "%.3f", [](float v) {return std::clamp(v, 10.0f, 150.0f); });
				};
			auto drawHasShadowMaps = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawCheckBox("hasShadowMaps", j);
				};
			auto drawShadowMapWidth = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawSelectableInt("shadowMapWidth", shadowMapSizes.at(light->lightType()), j);
				};
			auto drawShadowMapHeight = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawSelectableInt("shadowMapHeight", shadowMapSizes.at(light->lightType()), j);
				};
			auto drawViewWidth = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawFloat("viewWidth", j, "%.1f");
				};
			auto drawViewHeight = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawFloat("viewHeight", j, "%.1f");
				};
			auto drawNearZ = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawFloat("nearZ", j, "%.5f");
				};
			auto drawFarZ = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawFloat("farZ", j, "%.5f");
				};
			auto drawZBias = [](auto& light)
				{
					std::shared_ptr<JObject> j = light;
					EditorDrawFloat("zBias", j, "%.9f");
				};
			auto drawLight = [
				drawLightName, drawLightType, drawPosition, drawRotation, drawColor, drawBrightness,
				drawAttenuation, drawDirectionalDistance, drawConeAngle, drawHasShadowMaps,
				drawShadowMapWidth, drawShadowMapHeight, drawViewWidth, drawViewHeight, drawNearZ,
				drawFarZ, drawZBias](auto& l)
				{
					std::string tableName = l->uuid();
					if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags | ImGuiTableFlags_Hideable))
					{
						ImGui::TableSetupColumn("attribute", ImGuiTableColumnFlags_WidthFixed);
						ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

						drawLightName(l);
						drawLightType(l);
						LightType type = l->lightType();
						if (type != LT_Directional && type != LT_Ambient) drawPosition(l);
						if (type != LT_Point && type != LT_Ambient) drawRotation(l);
						drawColor(l);
						drawBrightness(l);
						if (type != LT_Directional && type != LT_Ambient) drawAttenuation(l);
						if (type == LT_Spot) drawConeAngle(l);
						if (type != LT_Ambient) drawHasShadowMaps(l);
						if (type != LT_Ambient && l->hasShadowMaps())
						{
							switch (type)
							{
							case LT_Directional:
							{
								drawShadowMapWidth(l);
								drawShadowMapHeight(l);
								drawDirectionalDistance(l);
								drawViewWidth(l);
								drawViewHeight(l);
								drawNearZ(l);
								drawFarZ(l);
							}
							break;
							case LT_Spot:
							{
								drawShadowMapWidth(l);
								drawShadowMapHeight(l);
								drawNearZ(l);
								drawFarZ(l);
							}
							break;
							case LT_Point:
							{
								drawShadowMapWidth(l);
								drawNearZ(l);
								drawFarZ(l);
							}
							break;
							}

							drawZBias(l);
						}

						ImGui::EndTable();
					}
				};
			auto drawShadowMap = [](auto& light)
				{
					if (light->shadowMapMinMaxChainResultRenderPass)
					{
						ImGui::DrawTextureImage(
							(ImTextureID)
							light->shadowMapMinMaxChainResultRenderPass->rendererToTexturePass->renderToTexture.at(0)->gpuTextureHandle.ptr,
							light->shadowMapMinMaxChainResultRenderPass->rendererToTexturePass->renderToTexture.at(0)->width,
							light->shadowMapMinMaxChainResultRenderPass->rendererToTexturePass->renderToTexture.at(0)->height
						);
					}
				};

			auto lights = getLights();
			for (auto& l : lights)
			{
				drawLight(l);
				if (l->hasShadowMaps())
					drawShadowMap(l);

				if (l != *(lights.end() - 1))
					ImGui::Separator();
			}
		};
}

inline void EditorDrawFilePath(
	std::string attribute,
	std::vector<std::shared_ptr<JObject>>& json,
	const char* buttonIcon,
	const std::string defaultFolder,
	std::vector<std::string> filterName,
	std::vector<std::string> filterPattern
)
{
	auto getFilePath = [attribute, &json]()
		{
			std::set<std::string> path;
			for (auto& j : json)
			{
				path.insert(j->at(attribute));
				if (path.size() > 1ULL) return std::string();
			}
			return *path.begin();
		};
	auto setFilePath = [attribute, &json](std::string path)
		{
			nlohmann::json patch = { {attribute,path} };
			for (auto& j : json)
			{
				j->JUpdate(patch);
			}
		};

	if (ImGui::BeginTable(attribute.c_str(), 2, defaultTableFlags))
	{
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::Text(attribute.c_str());

		ImGui::TableSetColumnIndex(1);

		std::filesystem::path path = getFilePath();

		if (ImGui::Button(buttonIcon))
		{
			ImGui::OpenFile([setFilePath, defaultFolder](std::filesystem::path p)
				{
					std::filesystem::path absfilepath = std::filesystem::current_path().append(defaultFolder);
					std::filesystem::path rel = std::filesystem::relative(p, absfilepath);
					setFilePath(rel.generic_string());
				}, defaultFolder, filterName, filterPattern);
		}
		ImGui::SameLine();
		ImGui::InputText("##", path.string().data(), path.string().size(), ImGuiInputTextFlags_ReadOnly);

		ImGui::EndTable();
	}
}

template<>
inline JEdvDrawerFunction DrawValue<std::string, jedv_t_model3d_filepath>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawFilePath(attribute, json, ICON_FA_CUBE, default3DModelsFolder, { "3D Models files. (*.gltf)" }, { "*.gltf" });
		};
}

template<>
inline JEdvDrawerFunction DrawValue<std::string, jedv_t_shaders_filepath>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawFilePath(attribute, json, ICON_FA_FILE_CODE, defaultShadersFolder, { "HLSL files. (*.hlsl)" }, { "*.hlsl" });
		};
}

template<>
inline JEdvDrawerFunction DrawValue<std::string, jedv_t_sounds_filepath>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawFilePath(attribute, json, ICON_FA_FILE_AUDIO, defaultAssetsFolder,
				{ "WAV files. (*.wav)", "MP3 files. (*.mp3)", "OGG files. (*.ogg)" },
				{ "*.wav", "*.mp3", "*.ogg" }
			);
		};
}

template<>
inline JEdvDrawerFunction DrawValue<unsigned int, jedv_t_unsigned>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			auto allSame = [attribute, &json]()
				{
					std::set<unsigned int> s;
					for (auto& j : json)
					{
						s.insert(static_cast<unsigned int>(j->at(attribute)));
						if (s.size() > 1) return false;
					}
					return true;
				};
			auto updateValues = [attribute, &json](unsigned int value)
				{
					for (auto& j : json)
					{
						nlohmann::json patch = { { attribute, value } };
						j->JUpdate(patch);
					}
				};

			ImGui::PushID(attribute.c_str());
			std::string tableName = "tables-" + attribute + "-table";
			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(attribute.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(ImGui::GetWindowWidth());
				if (!allSame())
				{
					std::string value = "";
					if (ImGui::InputText("", &value, ImGuiInputTextFlags_CharsDecimal))
					{
						value = std::regex_replace(value, std::regex(".+-*/"), "");
						if (value.size() > 0ULL)
						{
							updateValues((value.size() > 0ULL) ? static_cast<unsigned int>(std::stoi(value.c_str())) : 0U);
						}
					}
				}
				else
				{
					int value = static_cast<int>(json.at(0)->at(attribute));
					if (ImGui::InputInt("", &value))
					{
						value = max(value, 0);
						updateValues(value);
					}
				}
				ImGui::PopItemWidth();
				ImGui::EndTable();
			}
			ImGui::PopID();
		};
}

template<>
inline JEdvDrawerFunction DrawVector<std::string, jedv_t_filepath_vector_image >()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			auto textures = ToTextureJson(json);

			auto drawTextureName = [](auto& texture)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("name");

					ImGui::TableSetColumnIndex(1);
					ImGui::PushItemWidth(ImGui::GetWindowWidth());
					ImGui::PushID((texture->uuid() + "-name").c_str());
					ImGui::InputText("##", texture->name().data(), texture->name().size(), ImGuiInputTextFlags_ReadOnly);
					ImGui::PopID();
					ImGui::PopItemWidth();
				};
			auto drawTextureFormat = [](auto& texture)
				{
					std::vector<std::string> options = nostd::GetKeysFromMap(StringToDXGI_FORMAT);
					std::string selected = DXGI_FORMATToString.at(texture->format());

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("format");
					ImGui::TableSetColumnIndex(1);
					ImGui::PushID((texture->uuid() + "-format").c_str());
					ImGui::DrawComboSelection(selected, options, [&texture](std::string value)
						{
							nlohmann::json patch = { {"format",value} };
							texture->JUpdate(patch);
						}
					);
					ImGui::PopID();
				};
			auto drawTextureType = [](auto& texture)
				{
					//std::vector<std::string> options = nostd::GetKeysFromMap(StringToTextureType);
					//std::string selected = TextureTypeToString.at(texture->type());
					std::string texType = TextureTypeToString.at(texture->type());
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("type");
					ImGui::TableSetColumnIndex(1);
					ImGui::PushID((texture->uuid() + "-type").c_str());
					ImGui::PushItemWidth(ImGui::GetWindowWidth());
					ImGui::InputText("##", texType.data(), texType.size(), ImGuiInputTextFlags_ReadOnly);
					ImGui::PopID();
				};
			auto drawTextureDimension = [](auto& texture, std::string attribute)
				{
					static std::vector<unsigned int> dimensions =
					{
						16384U, 8192U, 4096U, 2048U, 1024U, 512U, 256U, 128U, 64U, 32U, 16U, 8U, 4U, 2U, 1U,
					};

					std::vector<std::string> dimensionsS;
					std::transform(dimensions.begin(), dimensions.end(), std::back_inserter(dimensionsS), [](unsigned int i) { return std::to_string(i); });
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(attribute.c_str());

					ImGui::TableSetColumnIndex(1);
					unsigned int dim = texture->at(attribute);
					int index = nostd::findElementIndex(dim, dimensions);
					ImGui::PushID((texture->uuid() + "-" + attribute).c_str());
					ImGui::DrawComboSelection(dimensionsS[index], dimensionsS, [&texture, attribute](std::string value)
						{
							nlohmann::json patch = { {attribute,std::stoi(value)} };
							texture->JUpdate(patch);
						}
					);
					ImGui::PopID();
				};
			auto drawTextureMipLevels = [](auto& texture)
				{
					std::vector<std::string> options;
					unsigned int maxDimension = max(texture->width(), texture->height());
					unsigned int possibleMipMaps = 1U + std::popcount(maxDimension - 1);
					for (unsigned int i = 0; i < possibleMipMaps; i++)
					{
						options.push_back(std::to_string(i));
					}
					unsigned int mipLevels = std::min(texture->mipLevels(), possibleMipMaps);
					std::string selected = options.at(mipLevels - 1);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("mipLevels");

					ImGui::TableSetColumnIndex(1);
					ImGui::PushID((texture->uuid() + "-mipLevels").c_str());
					ImGui::DrawComboSelection(selected, options, [&texture](std::string value)
						{
							nlohmann::json patch = { {"mipLevels", std::stoi(value)} };
							texture->JUpdate(patch);
						}
					);
					ImGui::PopID();
				};
			auto drawTextureNumFrames = [](auto& texture)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("numFrames");

					ImGui::TableSetColumnIndex(1);
					ImGui::PushID((texture->uuid() + "-numFrames").c_str());
					int numFrames = texture->numFrames();
					ImGui::InputInt("##", &numFrames, 1, 100, ImGuiInputTextFlags_ReadOnly);
					ImGui::PopID();
				};
			auto drawTextureFilePaths = [](auto& texture)
				{
					if (texture->type() != TextureType_Cube)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("image");

						ImGui::TableSetColumnIndex(1);
						ImGui::PushItemWidth(ImGui::GetWindowWidth());
						ImGui::PushID((texture->uuid() + "-image").c_str());

						if (ImGui::Button(ICON_FA_FILE_IMAGE))
						{
							std::filesystem::path assetPath = texture->images().at(0);
							assetPath = assetPath.remove_filename();
							ImGui::OpenFile([texture](std::filesystem::path p)
								{
									std::filesystem::path abspath = std::filesystem::current_path();
									std::filesystem::path rel = std::filesystem::relative(p, abspath);
									nlohmann::json patch = {
										{"name", rel.generic_string()},
										{"images", nlohmann::json::array({ rel.generic_string() })}
									};
									texture->JUpdate(patch);
									RenameTexture(texture->uuid(), rel.generic_string());
									Editor::MarkTemplatesPanelAssetsAsDirty();
								},
								assetPath.string(),
								defaultTexturesFilters,
								defaultTexturesExtensions
							);
						}
						ImGui::SameLine();
						ImGui::InputText("##", texture->images().at(0).data(), texture->images().at(0).size(), ImGuiInputTextFlags_ReadOnly);
						ImGui::SameLine();

						ImGui::PopID();
						ImGui::PopItemWidth();
					}
					else
					{

					}
				};
			auto drawTexturePreview = [](auto& texture)
				{
					ImGui::DrawTextureImage((ImTextureID)texture->preview->gpuHandle.ptr, texture->width(), texture->height());
					if (texture->type() == TextureType_Array)
					{
						ImGui::PushID(std::string(texture->uuid() + "-controller").c_str());
						drawAnimationController(
							[&texture] { return texture->previewIsPlaying; },
							[&texture](auto play) { texture->previewIsPlaying = play; },
							[&texture](float time) { texture->previewTime = time; },
							[&texture] { return texture->previewTimeFactor; },
							[&texture](float timeFactor) { texture->previewTimeFactor = timeFactor; },
							[&texture] {
								texture->previewTime = 0.0f;
								texture->previewIsLooping = false;
								texture->previewFrame = 0;
								texture->reloadPreview = true;
							},
							[&texture] {
								texture->previewTime = 1.0f;
								texture->previewIsLooping = false;
								texture->previewFrame = texture->numFrames() - 1;
								texture->reloadPreview = true;
							},
							[&texture] { return texture->previewIsLooping; },
							[&texture](bool looping) { texture->previewIsLooping = looping; }
						);
						ImGui::PopID();

						ImGui::SameLine();
						ImGui::PushID(std::string(texture->uuid() + "-timeFactor").c_str());
						ImGui::PushItemWidth(100.0f);
						ImGui::InputFloat("timeFactor", &texture->previewTimeFactor);
						ImGui::PopItemWidth();
						ImGui::PopID();

						ImGui::Text("frame");
						ImGui::SameLine();
						ImGui::PushID((std::string(texture->uuid()) + "-frame-slider").c_str());
						if (ImGui::SliderInt("##", &texture->previewFrame, 0, texture->numFrames() - 1))
						{
							texture->reloadPreview = true;
						}
						ImGui::PopID();
					}
				};

			auto drawTexture = [
				drawTextureName, drawTextureFormat, drawTextureType,
				drawTextureDimension, drawTextureMipLevels, drawTextureNumFrames, drawTextureFilePaths
			](auto& texture)
				{
					std::string tableName = texture->name();
					if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
					{
						ImGui::TableSetupColumn("attribute", ImGuiTableColumnFlags_WidthFixed);
						ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

						drawTextureName(texture);
						drawTextureFormat(texture);
						drawTextureType(texture);
						drawTextureDimension(texture, "width");
						drawTextureDimension(texture, "height");
						drawTextureMipLevels(texture);
						if (texture->type() == TextureType_Array)
						{
							drawTextureNumFrames(texture);
						}
						drawTextureFilePaths(texture);
						ImGui::EndTable();
					}
				};

			for (auto& t : textures)
			{
				drawTexture(t);
				drawTexturePreview(t);
				if (t != *(textures.end() - 1))
					ImGui::Separator();
			}
		};
}

template<>
inline JEdvDrawerFunction DrawValue<float, jedv_t_float>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			auto allSame = [attribute, &json]()
				{
					std::set<float> s;
					for (auto& j : json)
					{
						s.insert(float(j->at(attribute)));
						if (s.size() > 1) return false;
					}
					return true;
				};
			auto updateValues = [attribute, &json](float value)
				{
					for (auto& j : json)
					{
						nlohmann::json patch = { { attribute, value } };
						j->JUpdate(patch);
					}
				};

			ImGui::PushID(attribute.c_str());
			std::string tableName = "tables-" + attribute + "-table";
			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(attribute.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(ImGui::GetWindowWidth());
				if (!allSame())
				{
					std::string value = "";
					if (ImGui::InputText("", &value, ImGuiInputTextFlags_CharsDecimal))
					{
						value = std::regex_replace(value, std::regex(".+-*/"), "");
						if (value.size() > 0ULL)
						{
							updateValues((value.size() > 0ULL) ? std::stof(value.c_str()) : 0.0f);
						}
					}
				}
				else
				{
					float value = json.at(0)->at(attribute);
					if (ImGui::InputFloat("", &value))
					{
						updateValues(value);
					}
				}
				ImGui::PopItemWidth();
				ImGui::EndTable();
			}
			ImGui::PopID();
		};
}

template<>
inline JEdvDrawerFunction DrawValue<XMFLOAT2, jedv_t_float2>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawFloatArray(attribute, json, { "x","y" });
		};
}

template<>
inline JEdvDrawerFunction DrawValue<XMFLOAT3, jedv_t_float3>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawFloatArray(attribute, json, { "x","y","z" });
		};
}

template<>
inline JEdvDrawerFunction DrawValue<XMFLOAT3, jedv_t_sound_instance_pos>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			std::vector<std::shared_ptr<JObject>> filtered;
			std::copy_if(json.begin(), json.end(), std::back_inserter(filtered), [](auto& j)
				{
					unsigned int flags = j->at("instanceFlags");
					return !!(flags & SoundEffectInstance_Use3D);
				}
			);
			if (!filtered.empty())
			{
				EditorDrawFloatArray(attribute, filtered, { "x","y","z" });
			}
		};
}

template<>
inline JEdvDrawerFunction DrawValue<unsigned int, jedv_t_sound_instance_flags>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			auto getSoundEffects = [&json]()
				{
					std::vector<std::shared_ptr<Scene::SoundFX>> sfxs;
					std::transform(json.begin(), json.end(), std::back_inserter(sfxs), [](auto& j)
						{
							return std::dynamic_pointer_cast<Scene::SoundFX>(j);
						}
					);
					return sfxs;
				};

			auto drawInstanceFlags = [attribute](auto& sfx)
				{
					auto& StoE = StringToSOUND_EFFECT_INSTANCE_FLAGS;
					unsigned int instanceFlags = sfx->instanceFlags();
					bool sameLine = true;
					std::for_each(StoE.begin(), StoE.end(), [attribute, &instanceFlags, &sfx, &sameLine](auto& pair)
						{
							bool value = !!(instanceFlags & pair.second);
							if (ImGui::Checkbox(pair.first.c_str(), &value))
							{
								if (value)
								{
									instanceFlags |= pair.second;
								}
								else
								{
									instanceFlags &= ~pair.second;
								}
								nlohmann::json patch = { {attribute,instanceFlags} };
								sfx->JUpdate(patch);
							}
							if (sameLine)
								ImGui::SameLine();
							sameLine = !sameLine;
						}
					);
				};

			auto sfxs = getSoundEffects();

			std::string tableName = attribute + "-table";
			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags | ImGuiTableFlags_Hideable))
			{
				ImGui::TableSetupColumn("attribute", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(attribute.c_str());

				for (auto& sfx : sfxs)
				{
					if (!sfx->GetEffect()) continue;

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(sfx->name().c_str());
					ImGui::TableSetColumnIndex(1);
					drawInstanceFlags(sfx);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(1);

					drawAudioController(
						[sfx]() {return sfx->IsPlaying(); },
						[sfx]() {return sfx->IsPaused(); },
						[sfx]() { sfx->Play(); },
						[sfx]() { sfx->Stop(); },
						[sfx]() { sfx->Pause(); },
						[sfx]() {return sfx->Time(); },
						[sfx]() {return sfx->Duration(); }
					);
					if (sfx != *(sfxs.end() - 1))
						ImGui::Separator();
				}

				ImGui::EndTable();
			}
		};
}

template<>
inline JEdvDrawerFunction DrawValue<XMFLOAT4, jedv_t_float4>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawFloatArray(attribute, json, { "x","y","z","w" });
		};
}

template<>
inline JEdvDrawerFunction DrawValue<float, jedv_t_float_angle>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			auto allSame = [attribute, &json]()
				{
					std::set<float> s;
					for (auto& j : json)
					{
						s.insert(float(j->at(attribute)));
						if (s.size() > 1) return false;
					}
					return true;
				};
			auto updateValues = [attribute, &json](float value)
				{
					for (auto& j : json)
					{
						nlohmann::json patch = { { attribute, XMConvertToDegrees(value) } };
						j->JUpdate(patch);
					}
				};

			ImGui::PushID(attribute.c_str());
			std::string tableName = "tables-" + attribute + "-table";
			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(attribute.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(ImGui::GetWindowWidth());
				if (!allSame())
				{
					std::string value = "";
					if (ImGui::InputText("", &value, ImGuiInputTextFlags_CharsDecimal))
					{
						value = std::regex_replace(value, std::regex(".+-*/"), "");
						if (value.size() > 0ULL)
						{
							updateValues((value.size() > 0ULL) ? std::stof(value.c_str()) : 0.0f);
						}
					}
				}
				else
				{
					float minDeg = -180.0f;
					float maxDeg = 180.0f;
					float value = XMConvertToRadians(json.at(0)->at(attribute));
					if (ImGui::SliderAngle("", &value, minDeg, maxDeg, "%.2f", ImGuiSliderFlags_AlwaysClamp))
					{
						updateValues(value);
					}
				}
				ImGui::PopItemWidth();
				ImGui::EndTable();
			}
			ImGui::PopID();
		};
}

template<>
inline JEdvDrawerFunction DrawValue<float, jedv_t_float_coneangle>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			auto allSame = [attribute, &json]()
				{
					std::set<float> s;
					for (auto& j : json)
					{
						s.insert(float(j->at(attribute)));
						if (s.size() > 1) return false;
					}
					return true;
				};
			auto updateValues = [attribute, &json](float value)
				{
					for (auto& j : json)
					{
						nlohmann::json patch = { { attribute, XMConvertToDegrees(value) } };
						j->JUpdate(patch);
					}
				};

			ImGui::PushID(attribute.c_str());
			std::string tableName = "tables-" + attribute + "-table";
			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(attribute.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(ImGui::GetWindowWidth());
				if (!allSame())
				{
					std::string value = "";
					if (ImGui::InputText("", &value, ImGuiInputTextFlags_CharsDecimal))
					{
						value = std::regex_replace(value, std::regex(".+-*/"), "");
						if (value.size() > 0ULL)
						{
							updateValues((value.size() > 0ULL) ? std::stof(value.c_str()) : 0.0f);
						}
					}
				}
				else
				{
					float minDeg = 0.0f;
					float maxDeg = 180.0f;
					float value = XMConvertToRadians(json.at(0)->at(attribute));
					if (ImGui::SliderAngle("", &value, minDeg, maxDeg, "%.2f", ImGuiSliderFlags_AlwaysClamp))
					{
						updateValues(value);
					}
				}
				ImGui::PopItemWidth();
				ImGui::EndTable();
			}
			ImGui::PopID();
		};
}

template<>
inline JEdvDrawerFunction DrawValue<XMFLOAT2, jedv_t_float2_angle>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawFloatAngleArray(attribute, json, { "pitch","yaw" });
		};
}

template<>
inline JEdvDrawerFunction DrawValue<XMFLOAT3, jedv_t_float3_angle>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawFloatAngleArray(attribute, json, { "pitch","yaw","roll" });
		};
}

template<>
inline JEdvDrawerFunction DrawValue<XMFLOAT3, jedv_t_sound_instance_rotation>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			std::vector<std::shared_ptr<JObject>> filtered;
			std::copy_if(json.begin(), json.end(), std::back_inserter(filtered), [](auto& j)
				{
					unsigned int flags = j->at("instanceFlags");
					return !!(flags & SoundEffectInstance_Use3D);
				}
			);
			if (!filtered.empty())
			{
				EditorDrawFloatAngleArray(attribute, filtered, { "pitch","yaw","roll" });
			}
		};
}

template<>
inline JEdvDrawerFunction DrawValue<std::string, jedv_t_string>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			auto allSame = [attribute, &json]()
				{
					std::set<std::string> s;
					for (auto& j : json)
					{
						s.insert(j->at(attribute));
						if (s.size() > 1) return false;
					}
					return true;
				};

			ImGui::PushID(attribute.c_str());
			std::string tableName = "tables-" + attribute + "-table";
			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(attribute.c_str());
				ImGui::TableSetColumnIndex(1);
				std::string str = allSame() ? json.at(0)->at(attribute) : "";
				ImGui::PushItemWidth(ImGui::GetWindowWidth());
				if (ImGui::InputText("", &str))
				{
					for (auto& j : json) {
						nlohmann::json patch = { { attribute,str } };
						j->JUpdate(patch);
					}
				}
				ImGui::PopItemWidth();
				ImGui::EndTable();
			}
			ImGui::PopID();
		};
}

inline void DrawResourceSelection(
	std::string attribute,
	std::vector<std::shared_ptr<JObject>>& json,
	std::function<std::string(std::string)> ResourceUUIDToName,
	std::function<std::vector<UUIDName>()> GetResourcesUUIDsNames,
	const char* iconCode
)
{
	std::vector<UUIDName> selectables;

	auto allEqual = [attribute, &json]()
		{
			std::set<std::string> s;
			for (auto& j : json)
			{
				s.insert(j->at(attribute));
				if (s.size() > 1) return false;
			}
			return true;
		};
	auto update = [attribute, &json](auto value)
		{
			for (auto& j : json)
			{
				nlohmann::json patch = { {attribute,value} };
				j->JUpdate(patch);
			}
		};

	auto allEq = allEqual();
	UUIDName selected = std::make_tuple("", "");
	/*if (!allEq)*/ selectables.push_back(selected);
	/*else*/
	{
		std::string& uuid = std::get<0>(selected);
		std::string& name = std::get<1>(selected);
		uuid = json[0]->at(attribute);
		if (uuid != "") name = ResourceUUIDToName(uuid);
	}
	std::vector<UUIDName> resources = GetResourcesUUIDsNames();
	nostd::AppendToVector(selectables, resources);

	std::string tableName = "tables-" + attribute + "-table";
	if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text(attribute.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::PushID(attribute.c_str());

		ImGui::OpenTemplate(iconCode, selected);
		ImGui::SameLine();
		ImGui::DrawComboSelection(selected, selectables, [attribute, &json, update](UUIDName option)
			{
				std::string uuid = std::get<0>(option);
				//if (uuid != "")
				{
					update(uuid);
				}
			}
		);

		ImGui::PopID();
		ImGui::EndTable();
	}
}

template<>
inline JEdvDrawerFunction DrawValue<std::string, jedv_t_te_shader>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			DrawResourceSelection(attribute, json, Templates::GetShaderName, Templates::GetShadersUUIDsNames, ICON_FA_FILE_CODE);
		};
}

template<>
inline JEdvDrawerFunction DrawValue<std::string, jedv_t_te_model3d>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			DrawResourceSelection(attribute, json, Templates::GetModel3DName, Templates::GetModel3DsUUIDsNames, ICON_FA_CUBE);
		};
}

template<>
inline JEdvDrawerFunction DrawValue<std::string, jedv_t_te_sound>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			DrawResourceSelection(attribute, json, Templates::GetSoundName, Templates::GetSoundsUUIDsNames, ICON_FA_MUSIC);
		};
}

template<>
inline JEdvDrawerFunction DrawValue<bool, jedv_t_boolean>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			auto allSame = [attribute, &json]()
				{
					std::set<bool> s;
					for (auto& j : json)
					{
						s.insert(bool(j->at(attribute)));
						if (s.size() > 1) return false;
					}
					return true;
				};

			ImGui::PushID(attribute.c_str());
			std::string tableName = "tables-" + attribute + "-table";
			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(attribute.c_str());
				ImGui::TableSetColumnIndex(1);
				bool value = allSame() ? bool(json.at(0)->at(attribute)) : false;
				if (ImGui::Checkbox("", &value))
				{
					for (auto& j : json)
					{
						nlohmann::json patch = { { attribute, value } };
						j->JUpdate(patch);
					}
				}
				ImGui::EndTable();
			}
			ImGui::PopID();
		};
}

template<>
inline JEdvDrawerFunction DrawValue<std::string, jdev_t_animation>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			using namespace Scene;

			auto getAnimables = [&json]()
				{
					std::vector<std::shared_ptr<Renderable>> animables;
					for (auto& j : json)
					{
						std::shared_ptr<Renderable> r = std::dynamic_pointer_cast<Renderable>(j);
						if (!r->animable) continue;

						animables.push_back(r);
					}
					return animables;
				};

			auto animables = getAnimables();
			if (animables.size() == 0ULL) return;

			auto getAvailableAnimations = [&animables]()
				{
					std::map<std::string, unsigned int> animsCount;

					for (auto& r : animables)
					{
						std::vector<std::string> anims = nostd::GetKeysFromMap(r->animable->animations->animationsLength);
						for (auto& name : anims)
						{
							animsCount[name] = animsCount.contains(name) ? (animsCount[name] + 1) : 1;
						}
					}
					size_t jsz = animables.size();

					std::map<std::string, unsigned int> animations = { {"",0} };
					std::copy_if(animsCount.begin(), animsCount.end(), std::inserter(animations, animations.end()), [jsz](const auto& count)
						{
							auto& [_, c] = count;
							return c == jsz;
						}
					);
					return nostd::GetKeysFromMap(animations);
				};
			auto getSelectedAnimation = [&animables]()
				{
					std::set<std::string> animations;
					for (auto& r : animables)
					{
						animations.insert(r->animation());
						if (animations.size() > 1ULL) return std::string();
					}
					return *animations.begin();
				};
			auto setAnimation = [&animables](auto value)
				{
					for (auto& r : animables)
					{
						r->animation(value);
					}
				};

			auto animations = getAvailableAnimations();
			auto selected = getSelectedAnimation();

			auto animationsArePlaying = [&animables]()
				{
					std::set<bool> playing;
					for (auto& j : animables)
					{
						std::shared_ptr<Renderable> r = std::dynamic_pointer_cast<Renderable>(j);
						playing.insert(r->animationPlay());
					}
					return (playing.size() == 1) ? *playing.begin() : false;
				};
			auto setPlayAnimation = [&animables](auto value)
				{
					for (auto& r : animables)
					{
						r->animationPlay(value);
					}
				};
			auto getAnimationTime = [&animables]()
				{
					std::set<float> times;
					for (auto& r : animables)
					{
						times.insert(r->animationTime());
						if (times.size() > 1ULL) return 0.0f;
					}
					return *times.begin();
				};
			auto setAnimationTime = [&animables](auto value)
				{
					for (auto& r : animables)
					{
						r->animationTime(value);
					}
				};
			auto getAnimationTimeFactor = [&animables]()
				{
					std::set<float> timeFactors;
					for (auto& r : animables)
					{
						timeFactors.insert(r->animationTimeFactor());
						if (timeFactors.size() > 1ULL) return 0.0f;
					}
					return *timeFactors.begin();
				};
			auto setAnimationTimeFactor = [&animables](auto value)
				{
					for (auto& r : animables)
					{
						r->animationTimeFactor(value);
					}
				};
			auto allAnimationDurationEqual = [&animables, selected]()
				{
					std::set<float> lengths;
					for (auto& r : animables)
					{
						lengths.insert(r->animable->animations->animationsLength[selected]);
						if (lengths.size() > 1ULL) return false;
					}
					return true;
				};
			auto animationsAreLooping = [&animables]()
				{
					std::set<bool> loops;
					for (auto& r : animables)
					{
						loops.insert(r->animationLoop());
						if (loops.size() > 1ULL) return false;
					}
					return *loops.begin();
				};
			auto setAnimationLoop = [&animables](auto value)
				{
					for (auto& r : animables)
					{
						r->animationLoop(value);
					}
				};

			auto gotoPrevAnim = [animations, selected, setAnimation]()
				{
					auto current_item = static_cast<int>(std::find(animations.begin(), animations.end(), selected) - animations.begin());
					if (current_item <= 0)
						setAnimation(animations[animations.size() - 1]);
					else
						setAnimation(animations[current_item - 1]);
				};
			auto gotoNextAnimation = [animations, selected, setAnimation]()
				{
					auto current_item = static_cast<int>(std::find(animations.begin(), animations.end(), selected) - animations.begin());
					if (current_item >= animations.size())
						setAnimation(animations[0]);
					else
						setAnimation(animations[current_item + 1]);
				};

			auto getAnimationsProgress = [&animables]()
				{
					std::map<std::shared_ptr<Renderable>, float> progress;
					for (auto& r : animables)
					{
						float len = r->animable->animations->animationsLength[r->animation()];
						float curr = r->animationTime();
						progress.insert_or_assign(r, curr / len);
					}
					return progress;
				};
			auto animationsProgress = getAnimationsProgress();
			auto allAnimationsProgressAreEqual = [&animationsProgress]()
				{
					std::set<float> progress;
					for (auto [r, p] : animationsProgress)
					{
						progress.insert(p);
						if (progress.size() > 1ULL) return false;
					}
					return true;
				};
			auto setAnimationTimeByProgress = [&animables](auto value)
				{
					for (auto& r : animables)
					{
						float len = r->animable->animations->animationsLength[r->animation()];
						float time = value * len;
						r->animationTime(time);
					}
				};

			ImGui::Text(attribute.c_str());

			std::string tableName = "animation-controller";
			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
			{
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				drawAnimationController(
					animationsArePlaying,
					setPlayAnimation,
					setAnimationTime,
					getAnimationTimeFactor,
					setAnimationTimeFactor,
					gotoPrevAnim,
					gotoNextAnimation,
					animationsAreLooping,
					setAnimationLoop
				);

				ImGui::TableSetColumnIndex(1);
				ImGui::DrawComboSelection(selected, animations, [&animables, setAnimation](std::string value)
					{
						setAnimation(value);
					}
				);

				ImGui::EndTable();
			}

			bool allProgressEqual = allAnimationsProgressAreEqual();
			if (allProgressEqual && selected != "")
			{
				float sliderTime = animationsProgress.begin()->second;
				ImGui::Text("time");
				ImGui::SameLine();
				if (ImGui::SliderFloat("##", &sliderTime, 0.0f, 1.0f))
				{
					setAnimationTimeByProgress(sliderTime);
				}
			}
		};
}

template<>
inline JEdvDrawerFunction DrawValue<unsigned int, jedv_t_tex_dimension>()
{
	static std::vector<unsigned int> dimensions = {
		16384U, 8192U, 4096U, 2048U, 1024U, 512U, 256U, 128U, 64U, 32U, 16U, 8U, 4U, 2U, 1U,
	};

	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			auto textures = ToTextureJson(json);

			auto allEqual = [attribute, &json]
				{
					std::set<unsigned int> dims;
					for (auto& j : json)
					{
						unsigned int d = j->at(attribute);
						dims.insert(d);
						if (dims.size() > 1ULL) return false;
					}
					return true;
				};
			auto updateDimension = [attribute, &json](auto dimension)
				{
					nlohmann::json patch = { {attribute,dimension} };
					for (auto& j : json)
					{
						j->JUpdate(patch);
					}
				};

			bool allEq = allEqual();
			std::vector<std::string> dimensionsS;
			if (!allEq) dimensionsS.push_back("");
			std::transform(dimensions.begin(), dimensions.end(), std::back_inserter(dimensionsS), [](unsigned int i) { return std::to_string(i); });

			std::string tableName = attribute;

			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
			{
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text(attribute.c_str());

				ImGui::TableSetColumnIndex(1);
				int index;
				if (allEq)
				{
					unsigned int dim = json[0]->at(attribute);
					index = nostd::findElementIndex(dim, dimensions);
				}
				else
				{
					index = 0;
				}

				ImGui::DrawComboSelection(dimensionsS[index], dimensionsS, [updateDimension](std::string value)
					{
						if (value != "")
						{
							updateDimension(static_cast<unsigned int>(std::stoi(value)));
						}
					}
				);

				ImGui::EndTable();
			}
		};
}

template<>
inline JEdvDrawerFunction DrawVector<std::string, jedv_t_te_material_vector>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawVector(attribute, json, ICON_FA_TSHIRT, Templates::GetMaterialsUUIDsNames, Templates::GetMaterialName, ImGui::OpenTemplate);
		};
}

template<>
inline JEdvDrawerFunction DrawVector<std::string, jedv_t_te_model3d_vector>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawVector(attribute, json, ICON_FA_CUBE, Templates::GetModel3DsUUIDsNames, Templates::GetModel3DName, ImGui::OpenTemplate);
		};
}

template<>
inline JEdvDrawerFunction DrawVector<std::string, jedv_t_te_renderpass_vector>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawVector(attribute, json, ICON_FA_TV, Templates::GetRenderPasssUUIDsNames, Templates::GetRenderPassName, ImGui::OpenTemplate,
				[](unsigned int index, UUIDName item) //filtering
				{
					std::shared_ptr<RenderPassJson> rp = GetRenderPassTemplate(std::get<0>(item));
					return rp->type() == RenderPassType_RenderToTexturePass || (rp->renderCallbackOverride() == RenderPassRenderCallbackOverride_Resolve && index != 0);
				},
				[](unsigned int index1, unsigned int index2, unsigned int numItems, UUIDName item1, UUIDName item2) //swap
				{
					if (index1 > index2 && index2 == 0U) //moving up
					{
						std::string uuid = std::get<0>(item1);
						if (uuid != "")
						{
							std::shared_ptr<RenderPassJson> rp = GetRenderPassTemplate(uuid);
							if (rp->renderCallbackOverride() == RenderPassRenderCallbackOverride_Resolve) return false;
						}
					}
					else if (index1 < index2 && index1 == 0U) //moving down
					{
						std::string uuid = std::get<0>(item2);
						if (uuid != "")
						{
							std::shared_ptr<RenderPassJson> rp = GetRenderPassTemplate(uuid);
							if (rp->renderCallbackOverride() == RenderPassRenderCallbackOverride_Resolve) return false;
						}
					}
					return true;
				}
			);
		};
}

template<>
inline JEdvDrawerFunction DrawVector<std::string, jedv_t_te_shader_vector>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawVector(attribute, json, ICON_FA_FILE, Templates::GetShadersUUIDsNames, Templates::GetShaderName, ImGui::OpenTemplate);
		};
}

template<>
inline JEdvDrawerFunction DrawVector<std::string, jedv_t_te_sound_vector>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawVector(attribute, json, ICON_FA_MUSIC, Templates::GetSoundsUUIDsNames, Templates::GetSoundName, ImGui::OpenTemplate);
		};
}

template<>
inline JEdvDrawerFunction DrawVector<std::string, jedv_t_te_texture_vector>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawVector(attribute, json, ICON_FA_IMAGE, Templates::GetTexturesUUIDsNames, Templates::GetTextureName, ImGui::OpenTemplate);
		};
}

template<>
inline JEdvDrawerFunction DrawVector<std::string, jedv_t_so_camera_vector>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawVector(attribute, json, ICON_FA_CAMERA, Scene::GetCamerasUUIDNames, Scene::FindNameInCameras, ImGui::OpenSceneObject);
		};
}

template<>
inline JEdvDrawerFunction DrawVector<std::string, jedv_t_so_light_vector>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawVector(attribute, json, ICON_FA_LIGHTBULB, Scene::GetLightsUUIDNames, Scene::FindNameInLights, ImGui::OpenSceneObject);
		};
}

template<>
inline JEdvDrawerFunction DrawVector<std::string, jedv_t_so_renderable_vector>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawVector(attribute, json, ICON_FA_SNOWMAN, Scene::GetRenderablesUUIDNames, Scene::FindNameInRenderables, ImGui::OpenSceneObject);
		};
}

template<>
inline JEdvDrawerFunction DrawVector<std::string, jedv_t_so_soundeffect_vector>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			EditorDrawVector(attribute, json, ICON_FA_MUSIC, Scene::GetSoundEffectsUUIDNames, Scene::FindNameInSoundEffects, ImGui::OpenSceneObject);
		};
}

struct MeshMaterial;
template<>
inline JEdvDrawerFunction DrawVector<MeshMaterial, jedv_t_vector>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			int removeIndex = -1;
			std::vector<UUIDName> selectablesMeshes = { std::make_tuple("","") };
			std::vector<UUIDName> selectablesMaterials = { std::make_tuple("","") };
			std::vector<UUIDName> meshesUUIDs = Templates::GetMeshesUUIDsNames();
			std::vector<UUIDName> materialsUUIDs = Templates::GetMaterialsUUIDsNames();

			selectablesMeshes.insert(selectablesMeshes.end(), meshesUUIDs.begin(), meshesUUIDs.end());
			selectablesMaterials.insert(selectablesMaterials.end(), materialsUUIDs.begin(), materialsUUIDs.end());

			auto append = [attribute, &json](unsigned int index)
				{
					auto push_back = [attribute, &json](nlohmann::json value)
						{
							for (auto& j : json)
							{
								nlohmann::json cpy = (*j)[attribute];
								cpy.push_back(value);
								nlohmann::json patch = { {attribute,cpy} };
								j->JUpdate(patch);
								//(*j)[attribute].push_back(value);
							}
						};
					auto fit = [attribute, &json](unsigned int index, nlohmann::json value)
						{
							auto& v0 = json.at(0)->at(attribute);
							std::vector<nlohmann::json> v1;
							v1.insert(v1.begin(), v0.begin(), std::next(v0.begin(), index));
							v1.push_back(value);
							v1.insert(v1.end(), std::next(v0.begin(), index), v0.end());

							for (auto& j : json)
							{
								nlohmann::json patch = { {attribute,v1} };
								j->JUpdate(patch);
								//j->at(attribute) = v1;
							}
						};

					unsigned int size = static_cast<unsigned int>(json.at(0)->at(attribute).size());
					//only push_back if the vector is empty or if we are gonna push after the last item, otherwise we need to fit the values between
					if (size == 0U || index == size)
					{
						push_back(
							nlohmann::json::object(
								{ {"mesh",""}, {"material",""} }
							)
						);
					}
					else
					{
						fit(
							index, nlohmann::json::object(
								{ {"mesh",""}, {"material",""} }
							)
						);
					}
				};
			auto remove = [attribute, &json](unsigned int index)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.erase(index);
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).erase(index);
					}
				};
			auto swap = [attribute, &json](unsigned int from, unsigned int to)
				{
					std::string uuidMeshFrom = json.at(0)->at(attribute).at(from).at("mesh");
					std::string uuidMeshTo = json.at(0)->at(attribute).at(to).at("mesh");
					std::string uuidMaterialFrom = json.at(0)->at(attribute).at(from).at("material");
					std::string uuidMaterialTo = json.at(0)->at(attribute).at(to).at("material");
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);

						cpy.at(from).at("mesh") = uuidMeshTo;
						cpy.at(to).at("mesh") = uuidMeshFrom;
						cpy.at(from).at("material") = uuidMaterialTo;
						cpy.at(to).at("material") = uuidMaterialFrom;

						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);

						//j->at(attribute).at(from).at("mesh") = uuidMeshTo;
						//j->at(attribute).at(to).at("mesh") = uuidMeshFrom;
						//j->at(attribute).at(from).at("material") = uuidMaterialTo;
						//j->at(attribute).at(to).at("material") = uuidMaterialFrom;
					}
				};
			auto skip = [&json](unsigned int index, bool value)
				{
					std::vector<int> newValues{};
					auto& skip0 = json.at(0)->at("skipMeshes");
					size_t sz = skip0.size();
					for (unsigned int i = 0; i < sz; i++)
					{
						int val = skip0.at(i);
						if (val != index)
						{
							newValues.push_back(val);
						}
					}
					for (auto& j : json)
					{
						nlohmann::json patch = { {"skipMeshes",newValues} };
						j->JUpdate(patch);
						//j->at("skipMeshes") = newValues;
					}
				};
			auto setMesh = [attribute, &json](unsigned int index, std::string uuid)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.at(index).at("mesh") = uuid;
						nlohmann::json patch = { { attribute, cpy } };
						j->JUpdate(patch);
					}
				};
			auto setMaterial = [attribute, &json](unsigned int index, std::string uuid)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.at(index).at("material") = uuid;
						nlohmann::json patch = { { attribute, cpy } };
						j->JUpdate(patch);
					}
				};
			auto reset = [attribute, &json](std::string mesh, std::string material)
				{
					for (auto& j : json)
					{
						nlohmann::json patch = {
							{ attribute,
							{
								{
									{"mesh",mesh},
									{"material",material}
								}
							}
							},
							{ "skipMeshes", {}}
						};
						j->JUpdate(patch);
					}
				};
			auto allEmpty = [attribute, &json]()
				{
					for (auto& j : json)
					{
						if (j->contains(attribute) && j->at(attribute).size() > 0) return false;
					}
					return true;
				};
			auto allMeshMaterialsEqual = [attribute, &json]()
				{
					std::vector<std::tuple<std::string, std::string>> meshMaterials;
					bool first = true;
					for (auto& j : json)
					{
						auto& meshMats = j->at(attribute);
						size_t numMeshMats = meshMats.size();
						if (!first && meshMaterials.size() != numMeshMats) return false;

						for (unsigned int i = 0; i < numMeshMats; i++)
						{
							auto& mm = meshMats.at(i);
							std::string mesh = mm.at("mesh");
							std::string material = mm.at("material");
							std::tuple<std::string, std::string> tup = std::make_tuple(mesh, material);
							if (first)
							{
								meshMaterials.push_back(tup);
							}
							else
							{
								if (meshMaterials.at(i) != tup) return false;
							}
						}

						first = false;
					}

					return true;
				};
			auto allSkipsAreEqual = [&json]()
				{
					std::set<unsigned int> skips;
					bool first = true;

					for (auto& j : json)
					{
						auto& skipMeshes = j->at("skipMeshes");
						size_t numSkips = skipMeshes.size();
						if (!first && skips.size() != numSkips) return false;

						for (unsigned int i = 0; i < numSkips; i++)
						{
							unsigned int meshIdx = skipMeshes.at(i);
							if (first)
							{
								skips.insert(meshIdx);
							}
							else
							{
								if (!skips.contains(meshIdx)) return false;
							}
						}
					}

					return true;
				};
			auto mixedWithModel = [attribute, &json]()
				{
					for (auto& j : json)
					{
						if (j->contains("model") && j->at("model") != "") return true;
					}
					return false;
				};
			auto drawRow = [attribute, &meshesUUIDs, &materialsUUIDs, append, swap, skip, setMesh, setMaterial, &removeIndex](auto& json, auto& skips, unsigned int index, unsigned int size)
				{
					auto& j = json->at(attribute).at(index);
					std::string mesh = j.at("mesh");
					std::string material = j.at("material");
					std::string meshName = mesh.empty() ? "" : Templates::GetMeshName(mesh);
					std::string materialName = material.empty() ? "" : Templates::GetMaterialName(material);

					UUIDName meshUN = std::make_tuple(mesh, meshName);
					UUIDName matUN = std::make_tuple(material, materialName);

					std::set<unsigned int> skipsSet;
					auto& jskip = json->at("skipMeshes");
					size_t nskip = jskip.size();
					for (unsigned int i = 0; i < nskip; i++)
					{
						skipsSet.insert(static_cast<int>(jskip.at(i)));
					}

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::PushID((std::string("PlusLeft-") + std::to_string(index)).c_str());
					if (ImGui::Button(ICON_FA_PLUS))
					{
						append(index);
					}
					ImGui::PopID();

					ImGui::SameLine();
					ImGui::PushID((std::string("DeleteLeft-") + std::to_string(index)).c_str());
					if (ImGui::Button(ICON_FA_TIMES))
					{
						removeIndex = index;
					}
					ImGui::PopID();

					if (size > 1U)
					{
						ImGui::SameLine();
						ImGui::PushID((std::string("UpLeft-") + std::to_string(index)).c_str());
						ImGui::DrawItemWithEnabledState([index, swap]()
							{
								if (ImGui::Button(ICON_FA_ARROW_UP) && (index != 0))
								{
									swap(index, index - 1);
								}
							}
						, index != 0);
						ImGui::PopID();

						ImGui::SameLine();
						ImGui::PushID((std::string("DownLeft-") + std::to_string(index)).c_str());
						ImGui::DrawItemWithEnabledState([index, size, swap]()
							{
								if (ImGui::Button(ICON_FA_ARROW_DOWN) && (index != (size - 1)))
								{
									swap(index, index + 1);
								}
							}
						, index != (size - 1));
						ImGui::PopID();
					}

					ImGui::TableSetColumnIndex(1);
					bool value = skipsSet.contains(index);
					ImGui::PushID((std::string("skip-") + std::to_string(index)).c_str());
					if (ImGui::Checkbox("", &value))
					{
						skip(index, value);
					}
					ImGui::PopID();

					ImGui::TableSetColumnIndex(2);
					ImGui::PushID((std::string("mesh-") + std::to_string(index)).c_str());
					ImGui::DrawComboSelection(meshUN, meshesUUIDs, [index, setMesh](UUIDName option)
						{
							std::string& nuuid = std::get<0>(option);
							setMesh(index, nuuid);
						}
					);
					ImGui::PopID();

					ImGui::TableSetColumnIndex(3);
					ImGui::PushID((std::string("material-goto-") + std::to_string(index)).c_str());
					ImGui::OpenTemplate(ICON_FA_TSHIRT, matUN);
					ImGui::PopID();
					ImGui::SameLine();
					ImGui::PushID((std::string("material-") + std::to_string(index)).c_str());
					ImGui::DrawComboSelection(matUN, materialsUUIDs, [index, setMaterial](UUIDName option)
						{
							std::string& nuuid = std::get<0>(option);
							setMaterial(index, nuuid);
						}
					);
					ImGui::PopID();
				};
			auto drawPlaceholderRow = [attribute, &meshesUUIDs, &materialsUUIDs, reset, &json]()
				{
					UUIDName meshUN = std::make_tuple("", "");
					UUIDName matUN = std::make_tuple("", "");

					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(1);
					bool value = false;
					ImGui::PushID(std::string("skip-ph").c_str());
					ImGui::Checkbox("", &value);
					ImGui::PopID();

					ImGui::TableSetColumnIndex(2);
					ImGui::PushID(std::string("mesh-ph").c_str());
					ImGui::DrawComboSelection(meshUN, meshesUUIDs, [reset](UUIDName option)
						{
							std::string& nuuid = std::get<0>(option);
							reset(nuuid, "");
						}
					);
					ImGui::PopID();

					ImGui::TableSetColumnIndex(3);
					ImGui::PushID(std::string("material-ph").c_str());
					ImGui::DrawComboSelection(matUN, materialsUUIDs, [reset](UUIDName option)
						{
							std::string& nuuid = std::get<0>(option);
							reset("", nuuid);
						}
					);
					ImGui::PopID();
				};
			bool isEmpty = allEmpty();
			bool hasModel = mixedWithModel();
			bool meshMatEq = allMeshMaterialsEqual();
			bool skipsEq = allSkipsAreEqual();

			if (isEmpty)
			{
				std::string tableName = "empty-mesh-materials";
				if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Mesh/Material");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button(ICON_FA_PLUS, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f)))
					{
						append(0);
					}
					ImGui::EndTable();
				}
				return;
			}

			std::string tableName = "skip-mesh-materials";
			if (ImGui::BeginTable(tableName.c_str(), 4, ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableSetupColumn("actions", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("skip", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("mesh", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("material", ImGuiTableColumnFlags_WidthStretch);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(1);
				ImGui::TableHeader("skip");
				ImGui::TableSetColumnIndex(2);
				ImGui::TableHeader("mesh");
				ImGui::TableSetColumnIndex(3);
				ImGui::TableHeader("material");

				//this is the all good case
				if (!hasModel && meshMatEq && skipsEq)
				{
					auto j = json.at(0);
					auto& skips = j->at("skipMeshes");
					unsigned int numMeshes = static_cast<unsigned int>(j->at(attribute).size());
					for (unsigned int i = 0; i < numMeshes; i++)
					{
						drawRow(j, skips, i, numMeshes);
					}
				}
				else
				{
					drawPlaceholderRow();
				}

				ImGui::EndTable();
			}

			if (removeIndex != -1)
			{
				remove(removeIndex);
			}
		};
}

struct MaterialSamplerDesc;
template<>
inline JEdvDrawerFunction DrawVector<MaterialSamplerDesc, jedv_t_vector>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			int removeIndex = -1;
			ImGui::Separator();
			ImGui::Text("Samplers");

			auto insert = [attribute, &json](unsigned int index)
				{
					auto pos = json.at(0)->at(attribute).begin() + index + 1;
					MaterialSamplerDesc base;
					nlohmann::json samplerJ = base.json();
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.insert(pos, samplerJ);
						nlohmann::json patch = { {attribute, cpy} };
						j->JUpdate(patch);
						//j->at(attribute).insert(pos, samplerJ);
					}
				};
			auto remove = [attribute, &json](unsigned int index)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.erase(index);
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
					}
				};
			auto allEmpty = [attribute, &json]()
				{
					for (auto& j : json)
					{
						if (j->contains(attribute) && j->at(attribute).size() > 0ULL) return false;
					}
					return true;
				};
			auto allEq = [attribute, &json]()
				{
					std::set<size_t> hashes;
					for (auto& j : json)
					{
						size_t h = std::hash<std::vector<MaterialSamplerDesc>>()(*j, attribute);
						hashes.insert(h);
						if (hashes.size() > 1) return false;
					}
					return true;
				};
			auto update = [attribute, &json](unsigned int index, nlohmann::json sampler)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.at(index) = sampler;
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).at(index) = sampler;
					}
				};
			bool isEmpty = allEmpty();
			bool isEq = allEq();

			if (isEq && !isEmpty)
			{
				ImGui::DrawDynamicArray(
					attribute,
					json.at(0)->at(attribute),
					[insert](auto j, unsigned int index)
					{
						insert(index);
					},
					[&removeIndex](auto j, unsigned int index)
					{
						removeIndex = index;
					},
					[update](auto j, unsigned int index)
					{
						ImGui::Text(("Sampler#" + std::to_string(index + 1)).c_str());
						ImGui::NewLine();
						bool ret = false;
						std::string tableName = "material-desc-table";
						if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_NoSavedSettings))
						{
							ImGui::TableSetupColumn("attribute", ImGuiTableColumnFlags_WidthFixed);
							ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::TableHeader("attribute");
							ImGui::TableSetColumnIndex(1);
							ImGui::TableHeader("value");

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("AddressU");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("AddressU");
							ret |= ImGui::DrawFromCombo(j, "AddressU", StringToD3D12_TEXTURE_ADDRESS_MODE);
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("AddressV");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("AddressV");
							ret |= ImGui::DrawFromCombo(j, "AddressV", StringToD3D12_TEXTURE_ADDRESS_MODE);
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("AddressW");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("AddressW");
							ret |= ImGui::DrawFromCombo(j, "AddressW", StringToD3D12_TEXTURE_ADDRESS_MODE);
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("BorderColor");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("BorderColor");
							ret |= ImGui::DrawFromCombo(j, "BorderColor", StringToD3D12_STATIC_BORDER_COLOR);
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("ComparisonFunc");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("ComparisonFunc");
							ret |= ImGui::DrawFromCombo(j, "ComparisonFunc", StringToD3D12_COMPARISON_FUNC);
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Filter");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("Filter");
							ret |= ImGui::DrawFromCombo(j, "Filter", StringToD3D12_FILTER);
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("MaxAnisotropy");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("MaxAnisotropy");
							ret |= ImGui::DrawFromUInt(j, "MaxAnisotropy");
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("MaxLOD");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("MaxLOD");
							ret |= ImGui::DrawFromFloat(j, "MaxLOD");
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("MinLOD");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("MinLOD");
							ret |= ImGui::DrawFromFloat(j, "MinLOD");
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("MipLODBias");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("MipLODBias");
							ret |= ImGui::DrawFromFloat(j, "MipLODBias");
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("RegisterSpace");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("RegisterSpace");
							ret |= ImGui::DrawFromUInt(j, "RegisterSpace");
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("ShaderRegister");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("ShaderRegister");
							ret |= ImGui::DrawFromUInt(j, "ShaderRegister");
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("ShaderVisibility");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("ShaderVisibility");
							ret |= ImGui::DrawFromCombo(j, "ShaderVisibility", StringToD3D12_SHADER_VISIBILITY);
							ImGui::PopID();

							ImGui::EndTable();
						}

						if (ret) update(index, j);
					},
					8U
				);
			}
			else if (isEmpty)
			{
				std::string tableName = "empty-dxgi-formats";
				if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Formats");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button(ICON_FA_PLUS, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f)))
					{
						insert(0);
					}
					ImGui::EndTable();
				}
			}

			if (removeIndex != -1)
			{
				remove(removeIndex);
			}
		};
}

inline bool MaterialVariableBooleanDrawer(unsigned int index, nlohmann::json& value)
{
	bool ret = false;
	std::string tableName = "tables-" + std::to_string(index) + "-table-value";
	if (ImGui::BeginTable(tableName.c_str(), 1, defaultTableFlags))
	{
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		{
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("value");
			ImGui::SameLine();
			bool v = static_cast<bool>(value.at("value"));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "value").c_str());
			if (ImGui::Checkbox("##", &v))
			{
				value.at("value") = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}

		ImGui::EndTable();
	}
	return ret;
}
inline bool MaterialVariableIntegerDrawer(unsigned int index, nlohmann::json& value)
{
	bool ret = false;
	std::string tableName = "tables-" + std::to_string(index) + "-table-value";
	if (ImGui::BeginTable(tableName.c_str(), 1, defaultTableFlags))
	{
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		{
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("value");
			ImGui::SameLine();
			int v = static_cast<int>(value.at("value"));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "value").c_str());
			if (ImGui::InputInt("##", &v))
			{
				value.at("value") = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}

		ImGui::EndTable();
	}
	return ret;
}
inline bool MaterialVariableUnsignedIntegerDrawer(unsigned int index, nlohmann::json& value)
{
	bool ret = false;
	std::string tableName = "tables-" + std::to_string(index) + "-table-value";
	if (ImGui::BeginTable(tableName.c_str(), 1, defaultTableFlags))
	{
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		{
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("value");
			ImGui::SameLine();
			int v = static_cast<int>(value.at("value"));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "value").c_str());
			if (ImGui::InputInt("##", &v))
			{
				value.at("value") = std::abs(v);
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}

		ImGui::EndTable();
	}
	return ret;
}
inline bool MaterialVariableRGBDrawer(unsigned int index, nlohmann::json& value)
{
	bool ret = false;
	std::string tableName = "tables-" + std::to_string(index) + "-table-value";
	if (ImGui::BeginTable(tableName.c_str(), 1, defaultTableFlags))
	{
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		{
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("RGB");
			ImGui::SameLine();

			ImGui::PushID((std::to_string(index) + "RGB").c_str());
			XMFLOAT4 rgb = { value.at("value").at(0), value.at("value").at(1), value.at("value").at(2), 0.0f };
			if (ImGui::ColorEdit3("", (float*)&rgb)) {
				nlohmann::json& j = value.at("value");
				j.at(0) = rgb.x; j.at(1) = rgb.y; j.at(2) = rgb.z;
				ret = true;
			}
			ImGui::PopID();
		}

		ImGui::EndTable();
	}
	return ret;
}
inline bool MaterialVariableRGBADrawer(unsigned int index, nlohmann::json& value)
{
	bool ret = false;
	std::string tableName = "tables-" + std::to_string(index) + "-table-value";
	if (ImGui::BeginTable(tableName.c_str(), 1, defaultTableFlags))
	{
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		{
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("RGBA");
			ImGui::SameLine();

			ImGui::PushID((std::to_string(index) + "RGBA").c_str());
			XMFLOAT4 rgba = { value.at("value").at(0), value.at("value").at(1), value.at("value").at(2), value.at("value").at(3) };
			if (ImGui::ColorEdit4("", (float*)&rgba)) {
				nlohmann::json& j = value.at("value");
				j.at(0) = rgba.x; j.at(1) = rgba.y; j.at(2) = rgba.z; j.at(3) = rgba.w;
				ret = true;
			}
			ImGui::PopID();
		}

		ImGui::EndTable();
	}
	return ret;
}
inline bool MaterialVariableFloatDrawer(unsigned int index, nlohmann::json& value)
{
	bool ret = false;
	std::string tableName = "tables-" + std::to_string(index) + "-table-value";
	if (ImGui::BeginTable(tableName.c_str(), 1, defaultTableFlags))
	{
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();

		{
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("X");
			ImGui::SameLine();
			float v = static_cast<float>(value.at("value"));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "X").c_str());
			if (ImGui::InputFloat("##", &v))
			{
				value.at("value") = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}

		ImGui::EndTable();
	}
	return ret;
}
inline bool MaterialVariableFloat2Drawer(unsigned int index, nlohmann::json& value)
{
	bool ret = false;
	std::string tableName = "tables-" + std::to_string(index) + "-table-value";
	if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
	{
		ImGui::TableSetupColumn("value0", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("value1", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();

		{
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("X");
			ImGui::SameLine();
			float v = static_cast<float>(value.at("value").at(0));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "X").c_str());
			if (ImGui::InputFloat("##", &v))
			{
				value.at("value").at(0) = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}
		{
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("Y");
			ImGui::SameLine();
			float v = static_cast<float>(value.at("value").at(1));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "Y").c_str());
			if (ImGui::InputFloat("##", &v))
			{
				value.at("value").at(1) = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}

		ImGui::EndTable();
	}
	return ret;
}
inline bool MaterialVariableFloat3Drawer(unsigned int index, nlohmann::json& value)
{
	bool ret = false;
	std::string tableName = "tables-" + std::to_string(index) + "-table-value";
	if (ImGui::BeginTable(tableName.c_str(), 3, defaultTableFlags))
	{
		ImGui::TableSetupColumn("value0", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("value1", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("value2", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();

		{
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("X");
			ImGui::SameLine();
			float v = static_cast<float>(value.at("value").at(0));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "X").c_str());
			if (ImGui::InputFloat("##", &v))
			{
				value.at("value").at(0) = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}
		{
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("Y");
			ImGui::SameLine();
			float v = static_cast<float>(value.at("value").at(1));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "Y").c_str());
			if (ImGui::InputFloat("##", &v))
			{
				value.at("value").at(1) = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}
		{
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("Z");
			ImGui::SameLine();
			float v = static_cast<float>(value.at("value").at(2));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "Z").c_str());
			if (ImGui::InputFloat("##", &v))
			{
				value.at("value").at(2) = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}

		ImGui::EndTable();
	}
	return ret;
}
inline bool MaterialVariableFloat4Drawer(unsigned int index, nlohmann::json& value)
{
	bool ret = false;
	std::string tableName = "tables-" + std::to_string(index) + "-table-value";
	if (ImGui::BeginTable(tableName.c_str(), 4, defaultTableFlags))
	{
		ImGui::TableSetupColumn("value0", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("value1", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("value2", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("value3", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();

		{
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("X");
			ImGui::SameLine();
			float v = static_cast<float>(value.at("value").at(0));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "X").c_str());
			if (ImGui::InputFloat("##", &v))
			{
				value.at("value").at(0) = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}
		{
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("Y");
			ImGui::SameLine();
			float v = static_cast<float>(value.at("value").at(1));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "Y").c_str());
			if (ImGui::InputFloat("##", &v))
			{
				value.at("value").at(1) = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}
		{
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("Z");
			ImGui::SameLine();
			float v = static_cast<float>(value.at("value").at(2));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "Z").c_str());
			if (ImGui::InputFloat("##", &v))
			{
				value.at("value").at(2) = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}
		{
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("W");
			ImGui::SameLine();
			float v = static_cast<float>(value.at("value").at(3));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(index) + "W").c_str());
			if (ImGui::InputFloat("##", &v))
			{
				value.at("value").at(3) = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}

		ImGui::EndTable();
	}
	return ret;
}
inline bool MaterialVariableMatrix4x4Drawer(unsigned int index, nlohmann::json& value)
{
	bool ret = false;
	std::string tableName = "tables-" + std::to_string(index) + "-table-value";
	if (ImGui::BeginTable(tableName.c_str(), 5, defaultTableFlags))
	{
		ImGui::TableSetupColumn("rows", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("col0", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("col1", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("col2", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("col3", ImGuiTableColumnFlags_WidthStretch);

		unsigned int colId = 0U;
		unsigned int rowId = 0U;
		for (unsigned int i = 0; i < 16U; i++)
		{
			if ((i % 4U) == 0U)
			{
				rowId++;
				ImGui::TableNextRow();
				{
					ImGui::TableSetColumnIndex(0);
					ImGui::Text((std::string("row ") + std::to_string(rowId)).c_str());
				}
				colId = 1U;
			}

			ImGui::TableSetColumnIndex(colId++);
			float v = static_cast<float>(value.at("value").at(i));
			ImGui::PushItemWidth(ImGui::GetWindowWidth());
			ImGui::PushID((std::to_string(i)).c_str());
			if (ImGui::InputFloat("##", &v))
			{
				value.at("value").at(i) = v;
				ret = true;
			}
			ImGui::PopID();
			ImGui::PopItemWidth();
		}

		ImGui::EndTable();
	}
	return ret;
}

inline static std::map<MaterialVariablesTypes, std::function<bool(unsigned int, nlohmann::json&)>> MaterialVariablesTypesDrawers = {
	{ MAT_VAR_BOOLEAN, MaterialVariableBooleanDrawer },
	{ MAT_VAR_INTEGER, MaterialVariableIntegerDrawer },
	{ MAT_VAR_UNSIGNED_INTEGER, MaterialVariableUnsignedIntegerDrawer },
	{ MAT_VAR_RGB, MaterialVariableRGBDrawer },
	{ MAT_VAR_RGBA, MaterialVariableRGBADrawer },
	{ MAT_VAR_FLOAT, MaterialVariableFloatDrawer },
	{ MAT_VAR_FLOAT2, MaterialVariableFloat2Drawer },
	{ MAT_VAR_FLOAT3, MaterialVariableFloat3Drawer },
	{ MAT_VAR_FLOAT4, MaterialVariableFloat4Drawer },
	{ MAT_VAR_MATRIX4X4, MaterialVariableMatrix4x4Drawer },
};

template<>
inline JEdvDrawerFunction DrawVector<MaterialInitialValuePair, jedv_t_vector>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			ImGui::Text("Material Initial Values");

			using namespace Templates;

			int removeIndex = -1;
			MaterialVariablesTypes newType = MAT_VAR_NONE;
			int newTypeIndex = -1;

			auto append = [attribute, &json](unsigned int index)
				{
					unsigned int sz = static_cast<unsigned int>(json.at(0)->at(attribute).size());
					nlohmann::json toPush = {
						{"variable",std::string("V_") + std::to_string(sz)},
						{"variableType",MaterialVariablesTypesToString.at(MAT_VAR_FLOAT)},
						{"value", 0.0f }
					};
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.push_back(toPush);
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).push_back(toPush);
					}
				};
			auto remove = [attribute, &json](unsigned int index)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.erase(index);
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).erase(index);
					}
				};
			auto swap = [attribute, &json](unsigned int from, unsigned int to)
				{
					nlohmann::json jFrom = json.at(0)->at(attribute).at(from);
					nlohmann::json jTo = json.at(0)->at(attribute).at(to);
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.at(from) = jTo;
						cpy.at(to) = jFrom;
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).at(from) = jTo;
						//j->at(attribute).at(to) = jFrom;
					}
				};
			auto updateName = [attribute, &json](unsigned int index, std::string name)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.at(index).at("variable") = name;
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).at(index).at("variable") = name;
					}
				};
			auto updateType = [attribute, &json](unsigned int index, MaterialVariablesTypes type)
				{
					std::string varName = json.at(0)->at(attribute).at(index).at("variable");
					nlohmann::json newValue = nlohmann::json::array();
					MaterialVariablesMappedJsonInitializer.at(type)(newValue, varName);
					newValue = newValue.at(0);
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.at(index) = newValue;
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).at(index) = newValue;
					}
				};
			auto updateValue = [attribute, &json](unsigned int index)
				{
					if (index <= 0)
						return;

					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.at(index).at("value") = j->at(attribute).at(0).at("value");
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).at(index).at("value") = j->at(attribute).at(0).at("value");
					}
				};
			auto allEqual = [attribute, &json]()
				{
					bool first = true;
					std::map<unsigned int, nlohmann::json> valueMap;
					for (auto& j : json)
					{
						auto& jvals = j->at(attribute);
						unsigned int sz = static_cast<unsigned int>(jvals.size());
						if (!first && sz != valueMap.size()) return false;
						for (unsigned int i = 0; i < sz; i++)
						{
							if (first)
							{
								valueMap.insert_or_assign(i, jvals.at(i));
							}
							else if (valueMap.at(i) != jvals.at(i))
							{
								return false;
							}
						}
						first = false;
					}
					return true;
				};
			auto allEmpty = [attribute, &json]()
				{
					for (auto& j : json)
					{
						auto& jvals = j->at(attribute);
						unsigned int sz = static_cast<unsigned int>(jvals.size());
						if (sz > 0) return false;
					}
					return true;
				};
			auto allSelectables = [attribute, &json]
				{
					std::vector<std::string> selectables;

					std::string uuid_vs = json.at(0)->at("shader_vs");
					std::string uuid_ps = json.at(0)->at("shader_ps");

					auto vs = FindShaderInstance(uuid_vs);
					auto ps = FindShaderInstance(uuid_ps);

					if (vs)
					{
						std::transform(vs->constantsBuffersVariables.begin(), vs->constantsBuffersVariables.end(),
							std::back_inserter(selectables), [](auto& vsvar) { return vsvar.first; });
					}
					if (ps)
					{
						std::transform(ps->constantsBuffersVariables.begin(), ps->constantsBuffersVariables.end(),
							std::back_inserter(selectables), [](auto& psvar) { return psvar.first; });
					}

					auto& j = json.at(0)->at(attribute);
					std::transform(j.begin(), j.end(), std::back_inserter(selectables), [](auto& jvar) { return jvar.at("variable"); });

					return selectables;
				};
			auto drawSelectable = [attribute, &json, append, swap, &removeIndex, updateName, updateType, updateValue, &newType, &newTypeIndex](unsigned int index, auto selectables)
				{
					auto& j = json.at(0)->at(attribute);
					size_t size = j.size();

					auto& jvar = j.at(index);
					std::string name = jvar.at("variable");
					std::string type = jvar.at("variableType");

					std::string tableName = "tables-" + attribute + "-" + std::to_string(index) + "-table";
					if (ImGui::BeginTable(tableName.c_str(), 3, defaultTableFlags))
					{
						ImGui::TableSetupColumn("actions", ImGuiTableColumnFlags_WidthFixed);
						ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthStretch);
						ImGui::TableSetupColumn("type", ImGuiTableColumnFlags_WidthStretch);

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::PushID((std::string("PlusLeft-") + std::to_string(index)).c_str());
						if (ImGui::Button(ICON_FA_PLUS))
						{
							append(index);
						}
						ImGui::PopID();

						ImGui::SameLine();
						ImGui::PushID((std::string("DeleteLeft-") + std::to_string(index)).c_str());
						if (ImGui::Button(ICON_FA_TIMES))
						{
							removeIndex = index;
						}
						ImGui::PopID();

						if (size > 1U)
						{
							ImGui::SameLine();
							ImGui::PushID((std::string("UpLeft-") + std::to_string(index)).c_str());
							ImGui::DrawItemWithEnabledState([index, swap]()
								{
									if (ImGui::Button(ICON_FA_ARROW_UP) && (index != 0))
									{
										swap(index, index - 1);
									}
								}
							, index != 0);
							ImGui::PopID();

							ImGui::SameLine();
							ImGui::PushID((std::string("DownLeft-") + std::to_string(index)).c_str());
							ImGui::DrawItemWithEnabledState([index, size, swap]()
								{
									if (ImGui::Button(ICON_FA_ARROW_DOWN) && (index != (size - 1)))
									{
										swap(index, index + 1);
									}
								}
							, index != (size - 1));
							ImGui::PopID();
						}

						ImGui::TableSetColumnIndex(1);
						ImGui::PushID(std::string("name-" + std::to_string(index)).c_str());
						ImGui::PushItemWidth(ImGui::GetWindowWidth());
						if (ImGui::InputText("##", &name))
						{
							updateName(index, name);
						}
						ImGui::PopItemWidth();
						ImGui::PopID();

						ImGui::TableSetColumnIndex(2);
						ImGui::PushID(std::string("type-" + std::to_string(index)).c_str());
						ImGui::DrawComboSelection(type, nostd::GetKeysFromMap(StringToMaterialVariablesTypes), [index, &newType, &newTypeIndex](std::string nType)
							{
								newType = StringToMaterialVariablesTypes.at(nType);
								newTypeIndex = index;
							}
						);
						ImGui::PopID();

						ImGui::EndTable();
					}

					if (MaterialVariablesTypesDrawers.at(StringToMaterialVariablesTypes.at(type))(index, jvar))
					{
						updateValue(index);
					}
				};
			bool isEq = allEqual();
			bool isEmpty = allEmpty();

			if (isEq)
			{
				auto& j = json.at(0)->at(attribute);
				unsigned int sz = static_cast<unsigned int>(j.size());
				auto selectables = allSelectables();
				ImGui::Separator();
				ImGui::Text(attribute.c_str());
				for (unsigned int i = 0; i < sz; i++)
				{
					drawSelectable(i, selectables);
				}
			}

			if (removeIndex != -1)
			{
				remove(removeIndex);
			}
			if (newType != MAT_VAR_NONE)
			{
				updateType(newTypeIndex, newType);
			}
		};
}

template<>
inline JEdvDrawerFunction DrawVector<DXGI_FORMAT, jedv_t_vector>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			std::vector<std::string> formats = nostd::GetKeysFromMap(StringToDXGI_FORMAT);
			std::vector<std::string> selectables = { "" };
			selectables.insert(selectables.end(), formats.begin(), formats.end());

			int removeIndex = -1;
			auto allEmpty = [attribute, &json]()
				{
					for (auto& j : json)
					{
						if (j->contains(attribute) && j->at(attribute).size() > 0ULL) return false;
					}
					return true;
				};
			auto allEqual = [attribute, &json]()
				{
					std::set<size_t> sizes;
					for (auto& j : json)
					{
						if (j->contains(attribute))
						{
							sizes.insert(j->at(attribute).size());
						}
						else
						{
							sizes.insert(0ULL);
						}
						if (sizes.size() > 1ULL) return false;
					}

					bool first = true;
					std::map<unsigned int, std::string> values;
					unsigned int sz = static_cast<unsigned int>(json.at(0)->at(attribute).size());
					for (auto& j : json)
					{
						for (unsigned int i = 0; i < sz; i++)
						{
							std::string format = std::string(j->at(attribute).at(i));
							if (first)
							{
								values.insert_or_assign(1U, format);
							}
							else
							{
								if (values.at(i) != format) return false;
							}
						}
						first = false;
					}
					return true;
				};
			auto append = [attribute, &json](unsigned int index)
				{
					auto push_back = [attribute, &json](std::string value)
						{
							for (auto& j : json)
							{
								nlohmann::json cpy = (*j)[attribute];
								cpy.push_back(value);
								nlohmann::json patch = { {attribute,cpy} };
								j->JUpdate(patch);
								//(*j)[attribute].push_back(value);
							}
						};
					auto fit = [attribute, &json](unsigned int index, std::string value)
						{
							auto& v0 = json.at(0)->at(attribute);
							std::vector<std::string> v1;
							v1.insert(v1.begin(), v0.begin(), std::next(v0.begin(), index));
							v1.push_back(value);
							v1.insert(v1.end(), std::next(v0.begin(), index), v0.end());

							for (auto& j : json)
							{
								nlohmann::json patch = { { attribute, v1} };
								j->JUpdate(patch);
								//j->at(attribute) = v1;
							}
						};

					unsigned int size = static_cast<unsigned int>(json.at(0)->at(attribute).size());
					//only push_back if the vector is empty or if we are gonna push after the last item, otherwise we need to fit the values between
					if (size == 0U || index == size)
					{
						push_back("");
					}
					else
					{
						fit(index, "");
					}
				};
			auto remove = [attribute, &json](unsigned int index)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.erase(index);
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).erase(index);
					}
				};
			auto swap = [attribute, &json](unsigned int from, unsigned int to)
				{
					std::string formatFrom = json.at(0)->at(attribute).at(from);
					std::string formatTo = json.at(0)->at(attribute).at(to);
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.at(from) = formatTo;
						cpy.at(to) = formatFrom;
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).at(from) = formatTo;
						//j->at(attribute).at(to) = formatFrom;
					}
				};
			auto setFormat = [attribute, &json](unsigned int index, std::string format)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.at(index) = format;
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).at(index) = format;
					}
				};
			auto reset = [attribute, &json](std::string format)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = nlohmann::json::array({ format });
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//(*j)[attribute] = nlohmann::json::array({ format });
					}
				};
			auto drawRow = [attribute, append, swap, &removeIndex, selectables, setFormat](auto& json, unsigned int index, unsigned int size)
				{
					std::string format = std::string(json->at(attribute).at(index));

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::PushID((std::string("PlusLeft-") + std::to_string(index)).c_str());
					if (ImGui::Button(ICON_FA_PLUS))
					{
						append(index);
					}
					ImGui::PopID();

					ImGui::SameLine();
					ImGui::PushID((std::string("DeleteLeft-") + std::to_string(index)).c_str());
					if (ImGui::Button(ICON_FA_TIMES))
					{
						removeIndex = index;
					}
					ImGui::PopID();

					if (size > 1U)
					{
						ImGui::SameLine();
						ImGui::PushID((std::string("UpLeft-") + std::to_string(index)).c_str());
						ImGui::DrawItemWithEnabledState([index, swap]()
							{
								if (ImGui::Button(ICON_FA_ARROW_UP) && (index != 0))
								{
									swap(index, index - 1);
								}
							}
						, index != 0);
						ImGui::PopID();

						ImGui::SameLine();
						ImGui::PushID((std::string("DownLeft-") + std::to_string(index)).c_str());
						ImGui::DrawItemWithEnabledState([index, size, swap]()
							{
								if (ImGui::Button(ICON_FA_ARROW_DOWN) && (index != (size - 1)))
								{
									swap(index, index + 1);
								}
							}
						, index != (size - 1));
						ImGui::PopID();
					}

					ImGui::TableSetColumnIndex(1);
					ImGui::PushID((std::string("formats-") + std::to_string(index)).c_str());
					ImGui::DrawComboSelection(format, selectables, [index, setFormat](std::string option)
						{
							setFormat(index, option);
						}
					);
					ImGui::PopID();
				};
			auto drawPlaceholderRow = [attribute, &json, selectables, reset]()
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(1);

					ImGui::PushID(std::string("format-ph").c_str());
					ImGui::DrawComboSelection("", selectables, [reset](std::string format)
						{
							reset(format);
						}
					);
					ImGui::PopID();
				};

			bool isEmpty = allEmpty();
			if (isEmpty)
			{
				std::string tableName = "empty-dxgi-formats";
				if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Formats");
					ImGui::TableSetColumnIndex(1);
					if (ImGui::Button(ICON_FA_PLUS, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f)))
					{
						append(0);
					}
					ImGui::EndTable();
				}
				return;
			}

			bool allEq = allEqual();

			std::string tableName = "dxgi-formats";
			if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableSetupColumn("actions", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("formats", ImGuiTableColumnFlags_WidthStretch);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TableHeader("actions");
				ImGui::TableSetColumnIndex(1);
				ImGui::TableHeader("formats");

				//this is the all good case
				if (allEq)
				{
					auto j = json.at(0);
					unsigned int sz = static_cast<unsigned int>(j->at(attribute).size());
					for (unsigned int i = 0; i < sz; i++)
					{
						drawRow(j, i, sz);
					}

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(1);
					ImGui::DrawItemWithEnabledState([sz, append]()
						{
							ImGui::PushID("PlusBottom");
							if (ImGui::Button(ICON_FA_PLUS, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f)))
							{
								append(sz);
							}
							ImGui::PopID();
						}
					, allEq);
				}
				else
				{
					drawPlaceholderRow();
				}

				ImGui::EndTable();
			}

			if (removeIndex != -1)
			{
				remove(removeIndex);
			}
		};
}

struct RasterizerDesc;
template<>
inline JEdvDrawerFunction DrawValue<RasterizerDesc, jedv_t_object>()
{
	return [](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			ImGui::Separator();
			ImGui::Text("Rasterizer Desc");

			auto valueEqual = [attribute, &json](std::string key)
				{
					std::set<size_t> hashes;
					for (auto& j : json)
					{
						auto& att = j->at(attribute).at(key);
						hashes.insert(std::hash<nlohmann::json>{}(att));
						if (hashes.size() > 1ULL) return false;
					}
					return true;
				};
			auto update = [attribute, &json](std::string key, auto value)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.at(key) = value;
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).at(key) = value;
					};
				};
			auto drawFromSelectables = [valueEqual, update, attribute, &json](std::string key, auto& strKeysMap)
				{
					std::vector<std::string> selectables = {};
					std::string selected = " ";

					if (!valueEqual(key)) selectables.push_back(" ");
					else selected = json[0]->at(attribute).at(key);

					std::vector<std::string> keysStr = nostd::GetKeysFromMap(strKeysMap);
					nostd::AppendToVector(selectables, keysStr);

					ImGui::DrawComboSelection(selected, selectables, [update, key](std::string value)
						{
							if (value != " ") update(key, value);
						}
					);
				};
			auto drawFromCheckBox = [valueEqual, update, attribute, &json](std::string key)
				{
					bool value = (valueEqual(key)) ? (static_cast<bool>(json[0]->at(attribute).at(key))) : false;
					if (ImGui::Checkbox("##", &value))
					{
						update(key, value);
					}
				};
			auto drawFromInt = [valueEqual, update, attribute, &json](std::string key)
				{
					int value = (valueEqual(key)) ? (static_cast<int>(json[0]->at(attribute).at(key))) : 0;
					if (ImGui::InputInt("##", &value))
					{
						update(key, value);
					}
				};
			auto drawFromUInt = [valueEqual, update, attribute, &json](std::string key)
				{
					int value = (valueEqual(key)) ? (static_cast<int>(json[0]->at(attribute).at(key))) : 0;
					if (ImGui::InputInt("##", &value))
					{
						update(key, max(0, value));
					}
				};
			auto drawFromFloat = [valueEqual, update, attribute, &json](std::string key)
				{
					float value = (valueEqual(key)) ? (static_cast<float>(json[0]->at(attribute).at(key))) : 0;
					if (ImGui::InputFloat("##", &value))
					{
						update(key, value);
					}
				};

			std::string tableName = attribute;
			if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableSetupColumn("attribute", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TableHeader("attribute");
				ImGui::TableSetColumnIndex(1);
				ImGui::TableHeader("value");

				std::map<std::string, std::function<void()>> drawRasterizerState = {
					{ "FillMode", [drawFromSelectables] { drawFromSelectables("FillMode", StringToD3D12_FILL_MODE); }},
					{ "CullMode", [drawFromSelectables] { drawFromSelectables("CullMode", StringToD3D12_CULL_MODE); }},
					{ "FrontCounterClockwise", [drawFromCheckBox] { drawFromCheckBox("FrontCounterClockwise"); }},
					{ "DepthBias", [drawFromInt] { drawFromInt("DepthBias"); }},
					{ "DepthBiasClamp", [drawFromFloat] {drawFromFloat("DepthBiasClamp"); }},
					{ "SlopeScaledDepthBias", [drawFromFloat] {drawFromFloat("SlopeScaledDepthBias"); }},
					{ "DepthClipEnable", [drawFromCheckBox] { drawFromCheckBox("DepthClipEnable"); }},
					{ "MultisampleEnable", [drawFromCheckBox] { drawFromCheckBox("MultisampleEnable"); }},
					{ "AntialiasedLineEnable", [drawFromCheckBox] { drawFromCheckBox("AntialiasedLineEnable"); }},
					{ "ForcedSampleCount",[drawFromUInt] { drawFromUInt("ForcedSampleCount"); }},
					{ "ConservativeRaster", [drawFromSelectables] { drawFromSelectables("ConservativeRaster", StringToD3D12_CONSERVATIVE_RASTERIZATION_MODE); } },
				};

				for (auto& [att, drawer] : drawRasterizerState)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text(att.c_str());
					if (!drawer) continue;

					ImGui::TableSetColumnIndex(1);
					ImGui::PushID(att.c_str());
					drawer();
					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		};
}

struct RenderTargetBlendDesc;
struct BlendDesc;
template<>
inline JEdvDrawerFunction DrawValue<BlendDesc, jedv_t_object>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			ImGui::Separator();
			ImGui::Text("Blending");

			auto valueEqual = [attribute, &json](std::string key)
				{
					std::set<size_t> hashes;
					for (auto& j : json)
					{
						auto& att = j->at(attribute).at(key);
						hashes.insert(std::hash<nlohmann::json>{}(att));
						if (hashes.size() > 1ULL) return false;
					}
					return true;
				};
			auto update = [attribute, &json](std::string key, auto value)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.at(key) = value;
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).at(key) = value;
					};
				};
			auto drawFromCheckBox = [valueEqual, update, attribute, &json](std::string key)
				{
					bool value = (valueEqual(key)) ? (!!static_cast<int>(json[0]->at(attribute).at(key))) : false;
					if (ImGui::Checkbox("##", &value))
					{
						update(key, value);
					}
				};

			auto rtValueEqual = [attribute, &json](unsigned int index, std::string key)
				{
					std::set<size_t> hashes;
					for (auto& j : json)
					{
						if (
							!j->contains(attribute) ||
							!j->at(attribute).contains("RenderTarget") ||
							(j->at(attribute).at("RenderTarget").size() < index) ||
							!j->at(attribute).at("RenderTarget").contains(key))
						{
							hashes.insert(0ULL); continue;
						}

						auto& att = j->at(attribute).at("RenderTarget").at(index).at(key);
						hashes.insert(std::hash<nlohmann::json>{}(att));
						if (hashes.size() > 1ULL) return false;
					}
					return true;
				};
			auto rtUpdate = [attribute, &json](unsigned int index, std::string key, auto value)
				{
					for (auto& j : json)
					{
						nlohmann::json cpy = j->at(attribute);
						cpy.at("RenderTarget").at(index).at(key) = value;
						nlohmann::json patch = { {attribute,cpy} };
						j->JUpdate(patch);
						//j->at(attribute).at("RenderTarget").at(index).at(key) = value;
					};
				};
			auto rtDrawFromCheckBox = [rtValueEqual, rtUpdate, attribute, &json](unsigned int index, std::string key)
				{
					bool value = (rtValueEqual(index, key)) ? (!!static_cast<int>(json[0]->at(attribute).at("RenderTarget").at(index).at(key))) : false;
					if (ImGui::Checkbox("##", &value))
					{
						rtUpdate(index, key, value);
					}
				};
			auto rtDrawComboSelection = [rtValueEqual, rtUpdate, attribute, &json](unsigned int index, std::string key, std::vector<std::string> options)
				{
					std::vector<std::string> selectables;
					std::string selected = " ";
					if (!rtValueEqual(index, key)) { selectables.push_back(" "); }
					else selected = json[0]->at(attribute).at("RenderTarget").at(index).at(key);
					nostd::AppendToVector(selectables, options);
					ImGui::DrawComboSelection(selected, selectables, [index, key, rtUpdate](std::string option)
						{
							if (option != " ") rtUpdate(index, key, option);
						}
					);
				};

			auto allRenderTargetEmpty = [attribute, &json]()
				{
					for (auto& j : json)
					{
						if (j->contains(attribute) && j->at(attribute).contains("RenderTarget") && j->at(attribute).at("RenderTarget").size() > 0ULL) return false;
					}
					return true;
				};
			auto allRenderTargetEqual = [attribute, &json]()
				{
					std::set<size_t> sizes;
					for (auto& j : json)
					{
						sizes.insert((j->contains(attribute) && j->at(attribute).contains("RenderTarget")) ? j->at(attribute).at("RenderTarget").size() : 0ULL);
						if (sizes.size() > 1ULL) return false;
					}

					std::set<size_t> hashes;
					for (auto& j : json)
					{
						if (!(j->contains(attribute) && j->at(attribute).contains("RenderTarget"))) { hashes.insert(0ULL); continue; }
						size_t h = std::hash<std::vector<RenderTargetBlendDesc>>()(j->at(attribute), "RenderTarget");

						hashes.insert(h);
						if (hashes.size() > 1) return false;
					}
					return true;
				};

			std::string tableName = attribute;
			if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableSetupColumn("attribute", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TableHeader("attribute");
				ImGui::TableSetColumnIndex(1);
				ImGui::TableHeader("value");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("AlphaToCoverageEnable");
				ImGui::TableSetColumnIndex(1);
				ImGui::PushID("AlphaToCoverageEnable");
				drawFromCheckBox("AlphaToCoverageEnable");
				ImGui::PopID();

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("IndependentBlendEnable");
				ImGui::TableSetColumnIndex(1);
				ImGui::PushID("IndependentBlendEnable");
				drawFromCheckBox("IndependentBlendEnable");
				ImGui::PopID();

				ImGui::EndTable();
			}

			bool allRTEmpty = allRenderTargetEmpty();
			bool allRTEqual = allRenderTargetEqual();

			if (!allRTEmpty && allRTEqual)
			{
				size_t size = json[0]->at(attribute).at("RenderTarget").size();
				for (unsigned int i = 0; i < size; i++)
				{
					std::string headerName = (std::string("RenderTarget") + std::to_string(i));
					if (ImGui::CollapsingHeader(headerName.c_str()))
					{
						std::string tableName = headerName + "-table";
						if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_NoSavedSettings))
						{
							ImGui::TableSetupColumn("attribute", ImGuiTableColumnFlags_WidthFixed);
							ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::TableHeader("attribute");
							ImGui::TableSetColumnIndex(1);
							ImGui::TableHeader("value");

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("BlendEnable");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("BlendEnable");
							rtDrawFromCheckBox(i, "BlendEnable");
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("LogicOpEnable");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("LogicOpEnable");
							rtDrawFromCheckBox(i, "LogicOpEnable");
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("SrcBlend");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("SrcBlend");
							rtDrawComboSelection(i, "SrcBlend", nostd::GetKeysFromMap(StringToD3D12_BLEND));
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("DestBlend");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("DestBlend");
							rtDrawComboSelection(i, "DestBlend", nostd::GetKeysFromMap(StringToD3D12_BLEND));
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("BlendOp");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("BlendOp");
							rtDrawComboSelection(i, "BlendOp", nostd::GetKeysFromMap(StringToD3D12_BLEND_OP));
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("SrcBlendAlpha");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("SrcBlendAlpha");
							rtDrawComboSelection(i, "SrcBlendAlpha", nostd::GetKeysFromMap(StringToD3D12_BLEND));
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("DestBlendAlpha");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("DestBlendAlpha");
							rtDrawComboSelection(i, "DestBlendAlpha", nostd::GetKeysFromMap(StringToD3D12_BLEND));
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("BlendOpAlpha");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("BlendOpAlpha");
							rtDrawComboSelection(i, "BlendOpAlpha", nostd::GetKeysFromMap(StringToD3D12_BLEND_OP));
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("LogicOp");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("LogicOp");
							rtDrawComboSelection(i, "LogicOp", nostd::GetKeysFromMap(StringToD3D12_LOGIC_OP));
							ImGui::PopID();

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("RenderTargetWriteMask");
							ImGui::TableSetColumnIndex(1);
							ImGui::PushID("RenderTargetWriteMask");
							int RenderTargetWriteMask = json[0]->at(attribute).at("RenderTarget").at(i).at("RenderTargetWriteMask");
							if (ImGui::InputInt("##", &RenderTargetWriteMask))
							{
								rtUpdate(i, "RenderTargetWriteMask", RenderTargetWriteMask);
							}
							ImGui::PopID();

							ImGui::EndTable();
						}
					}
				}
			}
		};
}

template<>
inline JEdvDrawerFunction DrawMap<TextureShaderUsage, std::string>() {
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			ImGui::Separator();
			ImGui::Text("Textures");

			std::vector<UUIDName> texturesUUIDs = Templates::GetTexturesUUIDsNames();

			TextureShaderUsage removeUsage = TextureShaderUsage_None;

			auto allHas = [&json](TextureShaderUsage usage)
				{
					for (auto& j : json)
					{
						if (!j->contains("textures") || !j->at("textures").contains(TextureShaderUsageToString.at(usage)))
							return false;
					}
					return true;
				};
			auto allEqual = [&json](TextureShaderUsage usage)
				{
					std::set<std::string> values;
					for (auto& j : json)
					{
						if (j->contains("textures") && !j->at("textures").at(TextureShaderUsageToString.at(usage)).empty())
						{
							values.insert(j->at("textures").at(TextureShaderUsageToString.at(usage)));
						}
						if (values.size() > 1ULL) return false;
					}
					return true;
				};
			auto allEmpty = [&json, allHas]()
				{
					for (auto& usage : materialTexturesShaderUsage)
					{
						if (allHas(usage)) return false;
					}
					return true;
				};
			auto setTexture = [&json](TextureShaderUsage usage, std::string uuid)
				{
					std::string texUsage = TextureShaderUsageToString.at(usage);
					for (auto& j : json)
					{
						nlohmann::json cpy = (*j)["textures"];
						cpy[texUsage] = uuid;
						nlohmann::json patch = { {"textures",cpy} };
						j->JUpdate(patch);
						//(*j)["textures"][texUsage] = uuid;
					}
				};
			auto remove = [&json](TextureShaderUsage usage)
				{
					std::string strUsage = TextureShaderUsageToString.at(usage);
					for (auto& j : json)
					{
						nlohmann::json cpy = (*j)["textures"];
						cpy.erase(strUsage);
						nlohmann::json patch = { {"textures",cpy} };
						j->JUpdate(patch);
						//j->at("textures").erase(strUsage);
					}
				};
			auto add = [&json](TextureShaderUsage usage)
				{
					std::string strUsage = TextureShaderUsageToString.at(usage);
					for (auto& j : json)
					{
						nlohmann::json cpy = (*j)["textures"];
						cpy[strUsage] = "";
						nlohmann::json patch = { {"textures",cpy} };
						j->JUpdate(patch);
						//j->at("textures")[strUsage] = "";
					}
				};

			std::set<TextureShaderUsage> notSet = materialTexturesShaderUsage;
			if (!allEmpty()) {

				std::string tableName = "dxgi-formats";
				if (ImGui::BeginTable(tableName.c_str(), 3, ImGuiTableFlags_NoSavedSettings))
				{
					ImGui::TableSetupColumn("actions", ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn("usage", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn("texture", ImGuiTableColumnFlags_WidthStretch);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TableHeader("actions");
					ImGui::TableSetColumnIndex(1);
					ImGui::TableHeader("usage");
					ImGui::TableSetColumnIndex(2);
					ImGui::TableHeader("texture");

					for (auto& usage : materialTexturesShaderUsage)
					{
						bool available = allHas(usage);
						if (available) notSet.erase(usage);
						else continue;

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::PushID((std::string("DeleteLeft-") + TextureShaderUsageToString.at(usage)).c_str());
						if (ImGui::Button(ICON_FA_TIMES))
						{
							removeUsage = usage;
						}
						ImGui::PopID();

						ImGui::TableSetColumnIndex(1);
						ImGui::Text(TextureShaderUsageToString.at(usage).c_str());

						ImGui::TableSetColumnIndex(2);
						UUIDName selected = std::make_tuple("", "");

						if (allEqual(usage))
						{
							std::string& selectedUUID = std::get<0>(selected);
							std::string& selectedName = std::get<1>(selected);

							selectedUUID = json.at(0)->at("textures").at(TextureShaderUsageToString.at(usage));
							if (selectedUUID != "")
							{
								selectedName = Templates::GetTextureName(selectedUUID);
							}
						}

						ImGui::PushID(TextureShaderUsageToString.at(usage).c_str());
						ImGui::DrawComboSelection(selected, texturesUUIDs, [setTexture, usage](UUIDName selection)
							{
								setTexture(usage, std::get<0>(selection));
							}
						);
						ImGui::PopID();
					}

					ImGui::EndTable();
				}
			}

			if (notSet.size() > 0ULL)
			{
				std::vector<std::string> selectables = { "" };
				for (auto& usage : notSet)
				{
					selectables.push_back(TextureShaderUsageToString.at(usage));
				}

				ImGui::Text("Add New");
				ImGui::SameLine();
				ImGui::DrawComboSelection("", selectables, [add](auto selected)
					{
						add(StringToTextureShaderUsage.at(selected));
					}
				, "##");
			}

			if (removeUsage != TextureShaderUsage_None)
			{
				remove(removeUsage);
			}
		};
}

template<>
inline JEdvDrawerFunction DrawValue<Perspective, jedv_t_object>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			using namespace Scene::CameraProjections;

			std::set<std::string> projectionTypes;
			std::set<bool> fitWindow;
			for (auto& j : json) {
				projectionTypes.insert(j->at("projectionType"));
				fitWindow.insert(bool(j->at("fitWindow")));
			}
			if (projectionTypes.size() > 1ULL || StringToProjectionsTypes.at(*projectionTypes.begin()) != PROJ_Perspective) return;

			std::vector<std::tuple<std::string, std::string, float>> perspAtts = {
				std::make_tuple("nearZ", "%0.5f", Perspective::defaultNearZ),
				std::make_tuple("farZ", "%.2f", Perspective::defaultFarZ),
				std::make_tuple("fovAngleY", "%.2f", Perspective::defaultFovAngleY),
			};

			if (fitWindow.size() <= 1ULL && (*fitWindow.begin()) == false)
			{
				perspAtts.push_back(std::make_tuple("width", "%.0f", Perspective::defaultWidth));
				perspAtts.push_back(std::make_tuple("height", "%.0f", Perspective::defaultHeight));
			}

			auto updateValue = [attribute, &json](std::string att, float value)
				{
					using namespace Scene::CameraProjections;
					for (auto& j : json)
					{
						nlohmann::json patch =
						{
							{ attribute,
								{
									{ att, value }
								}
							}
						};
						j->JUpdate(patch);
					}
				};

			std::string tableName = "tables-" + attribute + "-table";
			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
			{
				for (auto tup : perspAtts)
				{
					auto att = std::get<0>(tup);
					auto format = std::get<1>(tup);
					auto defaultV = std::get<2>(tup);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(att.c_str());
					ImGui::TableSetColumnIndex(1);
					ImGui::PushID(att.c_str());

					std::set<float> fs;
					for (auto& j : json)
					{
						float v = j->at(attribute).contains(att) ? static_cast<float>(j->at(attribute).at(att)) : defaultV;
						fs.insert(v);
					}

					if (fs.size() > 1)
					{
						std::string value = "";
						if (ImGui::InputText("##", &value, ImGuiInputTextFlags_CharsDecimal))
						{
							value = std::regex_replace(value, std::regex(".+-*/"), "");
							if (value.size() > 0ULL)
							{
								updateValue(att, (value.size() > 0ULL) ? std::stof(value.c_str()) : 0.0f);
							}
						}
					}
					else
					{
						float value = json.at(0)->at(attribute).contains(att) ? static_cast<float>(json.at(0)->at(attribute).at(att)) : defaultV;
						if (ImGui::InputFloat("##", &value, 0.0f, 0.0f, format.c_str()))
						{
							updateValue(att, value);
						}
					}
					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		};
}

template<>
inline JEdvDrawerFunction DrawValue<Orthographic, jedv_t_object>()
{
	return[](std::string attribute, std::vector<std::shared_ptr<JObject>>& json)
		{
			using namespace Scene::CameraProjections;

			std::set<std::string> projectionTypes;
			std::set<bool> fitWindow;
			for (auto& j : json) {
				projectionTypes.insert(j->at("projectionType"));
				fitWindow.insert(bool(j->at("fitWindow")));
			}
			if (projectionTypes.size() > 1ULL || StringToProjectionsTypes.at(*projectionTypes.begin()) != PROJ_Orthographic) return;

			std::vector<std::tuple<std::string, std::string, float>> perspAtts = {
				std::make_tuple("nearZ", "%0.5f", Orthographic::defaultNearZ),
				std::make_tuple("farZ", "%.2f", Orthographic::defaultFarZ),
				std::make_tuple("width", "%.0f", Orthographic::defaultWidth),
				std::make_tuple("height", "%.0f", Orthographic::defaultHeight),
			};

			auto updateValue = [attribute, &json](std::string att, float value)
				{
					using namespace Scene::CameraProjections;
					for (auto& j : json)
					{
						nlohmann::json patch =
						{
							{ attribute,
								{
									{ att, value }
								}
							}
						};
						j->JUpdate(patch);
					}
				};

			std::string tableName = "tables-" + attribute + "-table";
			if (ImGui::BeginTable(tableName.c_str(), 2, defaultTableFlags))
			{
				for (auto tup : perspAtts)
				{
					auto att = std::get<0>(tup);
					auto format = std::get<1>(tup);
					auto defaultV = std::get<2>(tup);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(att.c_str());
					ImGui::TableSetColumnIndex(1);
					ImGui::PushID(att.c_str());

					std::set<float> fs;
					for (auto& j : json)
					{
						float v = j->at(attribute).contains(att) ? static_cast<float>(j->at(attribute).at(att)) : defaultV;
						fs.insert(v);
					}

					if (fs.size() > 1)
					{
						std::string value = "";
						if (ImGui::InputText("##", &value, ImGuiInputTextFlags_CharsDecimal))
						{
							value = std::regex_replace(value, std::regex(".+-*/"), "");
							if (value.size() > 0ULL)
							{
								updateValue(att, (value.size() > 0ULL) ? std::stof(value.c_str()) : 0.0f);
							}
						}
					}
					else
					{
						float value = json.at(0)->at(attribute).contains(att) ? static_cast<float>(json.at(0)->at(attribute).at(att)) : defaultV;
						if (ImGui::InputFloat("##", &value, 0.0f, 0.0f, format.c_str()))
						{
							updateValue(att, value);
						}
					}
					ImGui::PopID();
				}

				ImGui::EndTable();
			}
		};

}