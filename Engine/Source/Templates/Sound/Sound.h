#pragma once
#include <memory>
#include <string>
#include <Audio.h>
#include <Application.h>
#include <nlohmann/json.hpp>
#include "../../Scene/Sound/SoundEffect.h"
#include <UUID.h>

using namespace DirectX;

typedef std::tuple<
	std::string, //name
	nlohmann::json, //data
	std::shared_ptr<DirectX::SoundEffect> //DirectXTK SoundEffect Instance
#if defined(_EDITOR)
	,
	std::vector<std::shared_ptr<Scene::SoundEffect>> //Scene's SoundEffect instances
#endif
> SoundTemplate;

namespace Templates {

#if defined(_EDITOR)
	enum SoundPopupModal
	{
		SoundPopupModal_CannotDelete = 1,
		SoundPopupModal_CreateNew = 2
};
#endif

	namespace Sound
	{
		inline static const std::string templateName = "audio.json";
#if defined(_EDITOR)
		void ReleaseSoundEffectsInstances();
		void DrawEditorInformationAttributes(std::string uuid);
		void DrawEditorAssetAttributes(std::string uuid);
#endif
	}

	//CREATE
	void CreateSound(nlohmann::json json);
	std::unique_ptr<DirectX::SoundEffectInstance> GetSoundEffectInstance(std::string uuid, SOUND_EFFECT_INSTANCE_FLAGS flags);

	//READ&GET
	std::vector<std::string> GetSoundsNames();
	std::string GetSoundName(std::string uuid);
	std::vector<UUIDName> GetSoundsUUIDsNames();

	//UPDATE

	//DESTROY
	void ReleaseSoundTemplates();

	//EDITOR
#if defined(_EDITOR)
	void BindNotifications(std::string uuid, std::shared_ptr<Scene::SoundEffect> soundEffect);
	void UnbindNotifications(std::string uuid, std::shared_ptr<Scene::SoundEffect> soundEffect);
	void DrawSoundPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop);
	void CreateNewSound();
	void DeleteSound(std::string uuid);
	void DrawSoundsPopups();
	void WriteSoundsJson(nlohmann::json& json);
#endif

};

