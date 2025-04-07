#include "pch.h"
#include "Sound.h"
#include <map>
#include <Audio.h>
#include <nlohmann/json.hpp>
#include "../../Audio/AudioSystem.h"
#include "../../Scene/Sound/SoundEffect.h"
#include <NoStd.h>
#include <Application.h>
#if defined(_EDITOR)
#include "../../Editor/Editor.h"
#include <Editor.h>
#endif

using namespace AudioSystem;
using namespace DirectX;

#if defined(_EDITOR)
namespace Editor {
	extern _Templates tempTab;
	extern std::string selTemp;
}
#endif

namespace Templates
{
	std::map<std::string, SoundTemplate> sounds;
#if defined(_EDITOR)

	namespace Sound
	{
		nlohmann::json creationJson;
		unsigned int popupModalId = 0U;
	};
#endif

	//CREATE
	void CreateSound(nlohmann::json json)
	{
		std::string uuid = json.at("uuid");

		if (sounds.contains(uuid))
		{
			assert(!!!"sounds creation collision");
		}

		SoundTemplate t;
		std::string& name = std::get<0>(t);
		name = json.at("name");

		nlohmann::json& data = std::get<1>(t);
		data = json;
		data.erase("name");
		data.erase("uuid");

		sounds.insert_or_assign(uuid, t);
	}

	std::unique_ptr<DirectX::SoundEffectInstance> GetSoundEffectInstance(std::string uuid, SOUND_EFFECT_INSTANCE_FLAGS flags)
	{
		std::shared_ptr<DirectX::SoundEffect>& soundEffect = std::get<2>(sounds.at(uuid));

		if (soundEffect == nullptr)
		{
			nlohmann::json json = std::get<1>(sounds.at(uuid));
			soundEffect = std::make_shared<DirectX::SoundEffect>(GetAudioEngine().get(), nostd::StringToWString(json.at("path")).c_str());
		}

		return soundEffect->CreateInstance(flags);
	}

	//READ&GET
	std::vector<std::string> GetSoundsNames()
	{
		return GetNames(sounds);
	}

	std::string GetSoundName(std::string uuid)
	{
		return std::get<0>(sounds.at(uuid));
	}

	std::vector<UUIDName> GetSoundsUUIDsNames()
	{
		return GetUUIDsNames(sounds);
	}

	//UPDATE

	//DESTROY
	void Sound::ReleaseSoundEffectsInstances()
	{
		for (auto& [uuid, t] : sounds)
		{
			auto& instances = std::get<3>(t);
			instances.clear();
		}
		//soundInstances.clear();
	}

	void ReleaseSoundTemplates()
	{
		sounds.clear();
	}

	//EDITOR
#if defined(_EDITOR)

	void BindNotifications(std::string uuid, std::shared_ptr<Scene::SoundEffect> soundEffect)
	{
		auto& instances = std::get<3>(sounds.at(uuid));
		instances.push_back(soundEffect);
	}

	void UnbindNotifications(std::string uuid, std::shared_ptr<Scene::SoundEffect> soundEffect)
	{
		auto& instances = std::get<3>(sounds.at(uuid));
		for (auto it = instances.begin(); it != instances.end(); )
		{
			if (*it = soundEffect)
			{
				it = instances.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	void DetachSoundsEffectsTemplate(std::string uuid)
	{
		auto& instances = std::get<3>(sounds.at(uuid));
		for (auto& it : instances)
		{
			it->DetachSoundEffectTemplate();
		}
	}

	void DestroySoundEffectInstances(std::string uuid)
	{
		auto& instances = std::get<3>(sounds.at(uuid));
		for (auto& it : instances)
		{
			it->DestroySoundEffectInstance();
		}
	}

	void CreateSoundEffectInstances(std::string uuid)
	{
		auto& instances = std::get<3>(sounds.at(uuid));
		for (auto& it : instances)
		{
			it->CreateSoundEffectInstance();
		}
	}

	void EraseSoundEffect(std::string uuid)
	{
		sounds.erase(uuid);
	}

	void DrawSoundPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop)
	{
		std::string tableName = "sound-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			Sound::DrawEditorInformationAttributes(uuid);
			Sound::DrawEditorAssetAttributes(uuid);
			ImGui::EndTable();
		}
	}

	void CreateNewSound()
	{
		Sound::popupModalId = SoundPopupModal_CreateNew;
		Sound::creationJson = nlohmann::json(
			{
				{ "name", "" },
				{ "path", "" },
				{ "uuid", getUUID() }
			}
		);
	}

	void Sound::DrawEditorInformationAttributes(std::string uuid)
	{
		SoundTemplate& t = sounds.at(uuid);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "sound-information-atts";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string& currentName = std::get<0>(t);
			ImGui::InputText("name", &currentName);
			ImGui::EndTable();
		}
	}

	void Sound::DrawEditorAssetAttributes(std::string uuid)
	{
		nlohmann::json& json = std::get<1>(sounds.at(uuid));

		std::string parentFolder = defaultAssetsFolder;
		std::string fileName = "";
		if (json.contains("path") && !json.at("path").empty())
		{
			fileName = json.at("path");
			std::filesystem::path rootFolder = fileName;
			parentFolder = rootFolder.parent_path().string();
		}

		ImDrawFileSelector("##", fileName, [&json, uuid](std::filesystem::path path)
			{
				std::filesystem::path curPath = std::filesystem::current_path();
				std::filesystem::path relPath = std::filesystem::relative(path, curPath);
				json.at("path") = relPath.string();
				DestroySoundEffectInstances(uuid);
				EraseSoundEffect(uuid);
				CreateSoundEffectInstances(uuid);
			},
			parentFolder, "Sound files. (*.wav)", "*.wav"
		);
	}

	void DeleteSound(std::string uuid)
	{
		nlohmann::json& json = std::get<1>(sounds.at(uuid));
		if (json.contains("systemCreated") && json.at("systemCreated") == true)
		{
			Sound::popupModalId = SoundPopupModal_CannotDelete;
			return;
		}

		DetachSoundsEffectsTemplate(uuid);
		DestroySoundEffectInstances(uuid);
		EraseSoundEffect(uuid);
		sounds.erase(uuid);
	}

	void DrawSoundsPopups()
	{
		Editor::DrawOkPopup(Sound::popupModalId, SoundPopupModal_CannotDelete, "Cannot delete sound", []
			{
				ImGui::Text("Cannot delete a system created sound");
			}
		);

		Editor::DrawCreateWindow(Sound::popupModalId, SoundPopupModal_CreateNew, "Create new sound", [](auto OnCancel)
			{
				nlohmann::json& json = Sound::creationJson;

				ImGui::PushID("sound-name");
				{
					ImGui::Text("Name");
					ImDrawJsonInputText(json, "name");
				}
				ImGui::PopID();

				std::string parentFolder = defaultAssetsFolder;

				ImGui::PushID("sound-path");
				{
					ImDrawJsonFilePicker(json, "path", parentFolder, "Sound files. (*.wav)", "*.wav");
				}
				ImGui::PopID();

				if (ImGui::Button("Cancel")) { OnCancel(); }
				ImGui::SameLine();

				std::vector<std::string> soundNames = GetSoundsNames();
				bool disabledCreate = json.at("path") == "" || json.at("name") == "" || std::find(soundNames.begin(), soundNames.end(), std::string(json.at("name"))) != soundNames.end();

				if (disabledCreate)
				{
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}

				if (ImGui::Button("Create"))
				{
					Sound::popupModalId = 0;
					CreateSound(json);
				}

				if (disabledCreate)
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}
			}
		);
	}

	void WriteSoundsJson(nlohmann::json& json)
	{
		WriteTemplateJson(json, sounds);
	}

#endif

}
