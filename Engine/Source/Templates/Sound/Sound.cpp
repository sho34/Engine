#include "pch.h"
#include "Sound.h"
#include <map>
#include <Audio.h>
#include <nlohmann/json.hpp>
#include "../../Audio/AudioSystem.h"
#include <NoStd.h>

using namespace AudioSystem;
using namespace DirectX;
namespace Templates
{
	std::map<std::string, nlohmann::json> soundTemplates;
	std::map<std::string, std::shared_ptr<DirectX::SoundEffect>> soundEffects;

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

	/*
	std::shared_ptr<DirectX::SoundEffect> GetSoundEffect(std::string name)
		std::shared_ptr<Sound> sound = GetSoundTemplate(name);
		std::shared_ptr<DirectX::SoundEffect> soundEffect = std::make_shared<DirectX::SoundEffect>(GetAudioEngine().get(), nostd::StringToWString(sound->path).c_str());
		soundEffects.insert_or_assign(name, soundEffect);
		return soundEffect;
	}
	*/

	//READ&GET
	/*
	std::shared_ptr<Sound> GetSoundTemplate(std::string name) {
		return nostd::GetValueFromMap(soundTemplates, name);
	}
	*/

	std::vector<std::string> GetSoundsNames() {
		return nostd::GetKeysFromMap(soundTemplates);
	}

	//UPDATE

	//DESTROY
	void ReleaseSoundTemplates()
	{
		soundTemplates.clear();
	}

	/*
	void DestroySoundEffect(std::shared_ptr<DirectX::SoundEffect>& soundEffect)
	{
		nostd::EraseByValue(soundEffects, soundEffect);
		soundEffect = nullptr;
	}
	*/

	//EDITOR



	/*
	std::mutex soundCreateMutex;
	*/
#if defined(_EDITOR)
	/*
	void SelectSound(std::string soundName, void*& ptr) {
		ptr = soundTemplates.at(soundName).get();
	}
	*/

	void DrawSoundPanel(std::string& sound, ImVec2 pos, ImVec2 size, bool pop)
	{
	}

	/*
	std::string GetSoundName(void* ptr)
	{
		Sound* sound = (Sound*)ptr;
		return sound->name;
	}

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
