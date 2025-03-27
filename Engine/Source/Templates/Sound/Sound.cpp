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
	std::map<std::string, nlohmann::json> soundTemplates;
	std::map<std::string, std::shared_ptr<DirectX::SoundEffect>> soundEffects;
#if defined(_EDITOR)
	std::map<std::string, std::vector<std::shared_ptr<Scene::SoundEffect>>> soundInstances;
	enum SoundPopupModal
	{
		SoundPopupModal_CannotDelete = 1,
	};

	namespace Sound
	{
		unsigned int popupModalId = 0U;
	};
#endif

	//CREATE
	void CreateSound(std::string name, nlohmann::json json)
	{
		if (soundTemplates.contains(name)) return;
		soundTemplates.insert_or_assign(name, json);
	}

	std::unique_ptr<DirectX::SoundEffectInstance> GetSoundEffectInstance(std::string name, SOUND_EFFECT_INSTANCE_FLAGS flags)
	{
		std::shared_ptr<DirectX::SoundEffect> soundEffect = nullptr;
		if (soundEffects.contains(name))
		{
			soundEffect = soundEffects.at(name);
		}
		else
		{
			nlohmann::json json = soundTemplates.at(name);
			std::shared_ptr<DirectX::SoundEffect> soundEffect = std::make_shared<DirectX::SoundEffect>(GetAudioEngine().get(), nostd::StringToWString(json.at("path")).c_str());
			soundEffects.insert_or_assign(name, soundEffect);
		}

		return soundEffects.at(name)->CreateInstance(flags);
	}

	//READ&GET
	std::vector<std::string> GetSoundsNames() {
		return nostd::GetKeysFromMap(soundTemplates);
	}

	//UPDATE

	//DESTROY
	void Sound::ReleaseSoundEffectsInstances()
	{
		soundInstances.clear();
	}

	void ReleaseSoundTemplates()
	{
		soundTemplates.clear();
	}

	//EDITOR
#if defined(_EDITOR)

	void BindNotifications(std::string sound, std::shared_ptr<Scene::SoundEffect> soundEffect)
	{
		soundInstances[sound].push_back(soundEffect);
	}

	void UnbindNotifications(std::string sound, std::shared_ptr<Scene::SoundEffect> soundEffect)
	{
		auto& instances = soundInstances.at(sound);
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

	void DetachSoundsEffectsTemplate(std::string sound)
	{
		auto& instances = soundInstances.at(sound);
		for (auto& it : instances)
		{
			it->DetachSoundEffectTemplate();
		}
	}

	void DestroySoundEffectInstances(std::string sound)
	{
		auto& instances = soundInstances.at(sound);
		for (auto& it : instances)
		{
			it->DestroySoundEffectInstance();
		}
	}

	void CreateSoundEffectInstances(std::string sound)
	{
		auto& instances = soundInstances.at(sound);
		for (auto& it : instances)
		{
			it->CreateSoundEffectInstance();
		}
	}

	void EraseSoundEffect(std::string sound)
	{
		soundEffects.erase(sound);
	}

	void RenameSound(std::string& from, std::string to)
	{
		if (soundTemplates.contains(to) || to == "") return;
		nostd::RenameKey(soundTemplates, from, to);
		nostd::RenameKey(soundEffects, from, to);
		nostd::RenameKey(soundInstances, from, to);
		Editor::selTemp = to;
		from = to;
	}

	void DrawSoundPanel(std::string sound, ImVec2 pos, ImVec2 size, bool pop)
	{
		std::string tableName = "sound-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			Sound::DrawEditorInformationAttributes(sound);
			Sound::DrawEditorAssetAttributes(sound);
			ImGui::EndTable();
		}
	}

	void Sound::DrawEditorInformationAttributes(std::string& sound)
	{
		nlohmann::json& json = soundTemplates.at(sound);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "sound-information-atts";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string currentName = sound;
			if (ImGui::InputText("name", &currentName))
			{
				RenameSound(sound, currentName);
			}
			ImGui::EndTable();
		}
	}

	void Sound::DrawEditorAssetAttributes(std::string sound)
	{
		nlohmann::json& json = soundTemplates.at(sound);

		std::string parentFolder = defaultAssetsFolder;
		std::string fileName = "";
		if (json.contains("path") && !json.at("path").empty())
		{
			fileName = json.at("path");
			std::filesystem::path rootFolder = fileName;
			parentFolder = rootFolder.parent_path().string();
		}

		ImDrawFileSelector("##", fileName, [&json, sound](std::filesystem::path path)
			{
				std::filesystem::path curPath = std::filesystem::current_path();
				std::filesystem::path relPath = std::filesystem::relative(path, curPath);
				json.at("path") = relPath.string();
				DestroySoundEffectInstances(sound);
				EraseSoundEffect(sound);
				CreateSoundEffectInstances(sound);
			},
			parentFolder, "Sound files. (*.wav)", "*.wav"
		);
	}

	void DeleteSound(std::string name)
	{
		nlohmann::json sound = soundTemplates.at(name);
		if (sound.contains("systemCreated") && sound.at("systemCreated") == true)
		{
			Sound::popupModalId = SoundPopupModal_CannotDelete;
			return;
		}

		DetachSoundsEffectsTemplate(name);
		DestroySoundEffectInstances(name);
		EraseSoundEffect(name);
		soundTemplates.erase(name);
	}

	void DrawSoundsPopups()
	{
		Editor::DrawOkPopup(Sound::popupModalId, SoundPopupModal_CannotDelete, "CannotDeleteSoundPopup", []
			{
				ImGui::Text("Cannot delete a system created sound");
			}
		);
	}

	/*
	nlohmann::json json()
	{
		nlohmann::json j = nlohmann::json({});
		for (auto& [name, sound] : soundTemplates) {
			if (sound->systemCreated) continue;
			j[name] = nlohmann::json({});
			j[name]["assetPath"] = sound->assetPath;
		}
		return j;
	}
	*/
#endif

}
