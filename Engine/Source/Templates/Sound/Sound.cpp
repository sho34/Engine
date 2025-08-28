#include "pch.h"
#include "Sound.h"
#include <map>
#include <Audio.h>
#include <nlohmann/json.hpp>
#include <AudioSystem.h>
#include <Sound/SoundFX.h>
#include <NoStd.h>
#include <Application.h>
#if defined(_EDITOR)
#include <TemplateDef.h>
#endif

using namespace AudioSystem;
using namespace DirectX;

namespace Templates
{
#include <JExposeAttDrawersDef.h>
#include <SoundAtt.h>
#include <JExposeEnd.h>

	namespace Sound
	{
		std::map<std::string, std::unique_ptr<DirectX::SoundEffect>> uuidToSoundEffects;
		std::map<std::string, unsigned int> uuidInstanceCount;
	};

	std::tuple<
		std::unique_ptr<DirectX::SoundEffect>,
		std::unique_ptr<DirectX::SoundEffectInstance>
	> GetSoundEffectInstance(std::string uuid, unsigned int flags)
	{
		using namespace Sound;
		if (uuidToSoundEffects.contains(uuid))
		{
			uuidInstanceCount[uuid]++;
		}
		else
		{
			std::shared_ptr<SoundJson> json = GetSoundTemplate(uuid);
			std::wstring path = nostd::StringToWString(json->path());
			uuidToSoundEffects[uuid] = std::make_unique<DirectX::SoundEffect>(GetAudioEngine().get(), path.c_str());
			uuidInstanceCount[uuid] = 1;
		}
		return std::make_tuple(std::move(uuidToSoundEffects[uuid]), std::move(uuidToSoundEffects[uuid]->CreateInstance(SOUND_EFFECT_INSTANCE_FLAGS(flags))));
	}

	void DestroySoundEffectInstance(std::string uuid, std::tuple<
		std::unique_ptr<DirectX::SoundEffect>,
		std::unique_ptr<DirectX::SoundEffectInstance>
	>& soundEffectInstance)
	{
		using namespace Sound;
		uuidInstanceCount[uuid]--;
		std::unique_ptr<DirectX::SoundEffectInstance>& sfxI = std::get<1>(soundEffectInstance);
		sfxI = nullptr;
		soundEffectInstance = std::make_tuple(nullptr, nullptr);
		if (uuidInstanceCount[uuid] == 0)
		{
			uuidToSoundEffects.erase(uuid);
			uuidInstanceCount.erase(uuid);
		}
	}

	void ReleaseSoundEffectsInstances()
	{
		//for (auto& [uuid, t] : sounds)
		//{
		//	auto& instances = std::get<3>(t);
		//	instances.clear();
		//}
	}

	SoundJson::SoundJson(nlohmann::json json) : JTemplate(json)
	{
#include <JExposeInit.h>
#include <SoundAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttUpdate.h>
#include <SoundAtt.h>
#include <JExposeEnd.h>
	}

	TEMPDEF_FULL(Sound);

	//EDITOR
#if defined(_EDITOR)

	void BindNotifications(std::string uuid, std::shared_ptr<Scene::SoundFX> soundEffect)
	{
		//auto& instances = std::get<3>(sounds.at(uuid));
		//instances.push_back(soundEffect);
	}

	void UnbindNotifications(std::string uuid, std::shared_ptr<Scene::SoundFX> soundEffect)
	{
		//auto& instances = std::get<3>(sounds.at(uuid));
		//for (auto it = instances.begin(); it != instances.end(); )
		//{
		//	if (*it = soundEffect)
		//	{
		//		it = instances.erase(it);
		//	}
		//	else
		//	{
		//		it++;
		//	}
		//}
	}

	void DetachSoundsEffectsTemplate(std::string uuid)
	{
		//auto& instances = std::get<3>(sounds.at(uuid));
		//for (auto& it : instances)
		//{
		//	it->DetachSoundEffectTemplate();
		//}
	}

	void DestroySoundEffectInstances(std::string uuid)
	{
		//auto& instances = std::get<3>(sounds.at(uuid));
		//for (auto& it : instances)
		//{
		//	it->DestroySoundEffectInstance();
		//}
	}

	void CreateSoundEffectInstances(std::string uuid)
	{
		//auto& instances = std::get<3>(sounds.at(uuid));
		//for (auto& it : instances)
		//{
		//	it->CreateSoundEffectInstance();
		//}
	}

	void EraseSoundEffect(std::string uuid)
	{
		//sounds.erase(uuid);
	}

	void DrawSoundPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop)
	{
		//std::string tableName = "sound-panel";
		//if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		//{
		//	Sound::DrawEditorInformationAttributes(uuid);
		//	Sound::DrawEditorAssetAttributes(uuid);
		//	ImGui::EndTable();
		//}
	}

	void CreateNewSound()
	{
		//Sound::popupModalId = SoundPopupModal_CreateNew;
		//Sound::creationJson = nlohmann::json(
		//	{
		//		{ "name", "" },
		//		{ "path", "" },
		//		{ "uuid", getUUID() }
		//	}
		//);
	}

	void DeleteSound(std::string uuid)
	{
		//nlohmann::json& json = std::get<1>(sounds.at(uuid));
		//if (json.contains("systemCreated") && json.at("systemCreated") == true)
		//{
		//	Sound::popupModalId = SoundPopupModal_CannotDelete;
		//	return;
		//}
		//
		//DetachSoundsEffectsTemplate(uuid);
		//DestroySoundEffectInstances(uuid);
		//EraseSoundEffect(uuid);
		//sounds.erase(uuid);
	}

	void DrawSoundsPopups()
	{
		//Editor::DrawOkPopup(Sound::popupModalId, SoundPopupModal_CannotDelete, "Cannot delete sound", []
		//	{
		//		ImGui::Text("Cannot delete a system created sound");
		//	}
		//);
		//
		//Editor::DrawCreateWindow(Sound::popupModalId, SoundPopupModal_CreateNew, "Create new sound", [](auto OnCancel)
		//	{
		//		nlohmann::json& json = Sound::creationJson;
		//
		//		ImGui::PushID("sound-name");
		//		{
		//			ImGui::Text("Name");
		//			ImDrawJsonInputText(json, "name");
		//		}
		//		ImGui::PopID();
		//
		//		std::string parentFolder = defaultAssetsFolder;
		//
		//		ImGui::PushID("sound-path");
		//		{
		//			ImDrawJsonFilePicker(json, "path", parentFolder, "Sound files. (*.wav)", "*.wav");
		//		}
		//		ImGui::PopID();
		//
		//		if (ImGui::Button("Cancel")) { OnCancel(); }
		//		ImGui::SameLine();
		//
		//		std::vector<std::string> soundNames = GetSoundsNames();
		//		bool disabledCreate = json.at("path") == "" || json.at("name") == "" || std::find(soundNames.begin(), soundNames.end(), std::string(json.at("name"))) != soundNames.end();
		//
		//		if (disabledCreate)
		//		{
		//			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		//			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		//		}
		//
		//		if (ImGui::Button("Create"))
		//		{
		//			Sound::popupModalId = 0;
		//			CreateSound(json);
		//		}
		//
		//		if (disabledCreate)
		//		{
		//			ImGui::PopItemFlag();
		//			ImGui::PopStyleVar();
		//		}
		//	}
		//);
	}

	bool SoundsPopupIsOpen()
	{
		//return !!Sound::popupModalId;
		return false;
	}

#endif

	/*
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
	*/



	/*
	//READ&GET
#if defined(_EDITOR)
	std::vector<std::string> GetSoundsNames()
	{
		return GetNames(sounds);
	}
#endif

	std::string GetSoundName(std::string uuid)
	{
		return std::get<0>(sounds.at(uuid));
	}

#if defined(_EDITOR)
	std::vector<UUIDName> GetSoundsUUIDsNames()
	{
		return GetUUIDsNames(sounds);
	}
#endif

	//UPDATE

	//DESTROY


	void ReleaseSoundTemplates()
	{
		sounds.clear();
	}
	*/

}
