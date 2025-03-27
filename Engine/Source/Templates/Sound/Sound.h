#pragma once
#include <memory>
#include <string>
#include <Audio.h>
#include "../../Scene/Sound/SoundEffect.h"

using namespace DirectX;

namespace Templates {

	namespace Sound
	{
		inline static const std::string templateName = "audio.json";
#if defined(_EDITOR)
		void ReleaseSoundEffectsInstances();
		void DrawEditorInformationAttributes(std::string& sound);
		void DrawEditorAssetAttributes(std::string sound);
#endif
	}

	//CREATE
	void CreateSound(std::string name, nlohmann::json json);
	std::unique_ptr<DirectX::SoundEffectInstance> GetSoundEffectInstance(std::string name, SOUND_EFFECT_INSTANCE_FLAGS flags);

	//READ&GET
	std::vector<std::string> GetSoundsNames();

	//UPDATE

	//DESTROY
	void ReleaseSoundTemplates();

	//EDITOR
#if defined(_EDITOR)
	void BindNotifications(std::string sound, std::shared_ptr<Scene::SoundEffect> soundEffect);
	void UnbindNotifications(std::string sound, std::shared_ptr<Scene::SoundEffect> soundEffect);
	void DrawSoundPanel(std::string sound, ImVec2 pos, ImVec2 size, bool pop);
	void DeleteSound(std::string name);
	void DrawSoundsPopups();
	//nlohmann::json json();
#endif

};

