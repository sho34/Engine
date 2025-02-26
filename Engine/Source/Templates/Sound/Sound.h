#pragma once
#include <memory>
#include <string>

using namespace DirectX;

namespace Templates {

	namespace Sound
	{
		inline static const std::string templateName = "audio.json";
	}
	/*
	struct Sound
	{
		inline static const std::string templateName = "audio.json";

		std::string name;
		std::string path;

		std::unique_ptr<DirectX::SoundEffectInstance> GetSoundEffectInstance(SOUND_EFFECT_INSTANCE_FLAGS flags);
	};
	*/

	//CREATE
	void CreateSound(std::string name, nlohmann::json json);
	std::unique_ptr<DirectX::SoundEffectInstance> GetSoundEffectInstance(std::string name, SOUND_EFFECT_INSTANCE_FLAGS flags);
	//std::shared_ptr<DirectX::SoundEffect> GetSoundEffect(std::string name);

	//READ&GET
	//std::shared_ptr<Sound> GetSoundTemplate(std::string name);
	std::vector<std::string> GetSoundsNames();

	//UPDATE

	//DESTROY
	void ReleaseSoundTemplates();
	//void DestroySoundEffect(std::shared_ptr<DirectX::SoundEffect>& soundEffect);

	//EDITOR
#if defined(_EDITOR)
	/*
	void SelectSound(std::string soundName, void*& ptr);
	*/
	void DrawSoundPanel(std::string& sound, ImVec2 pos, ImVec2 size, bool pop);
	/*
	std::string GetSoundName(void* ptr);
	*/
	//nlohmann::json json();
#endif

};

