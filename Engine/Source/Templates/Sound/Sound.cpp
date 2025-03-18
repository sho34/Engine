#include "pch.h"
#include "Sound.h"
#include <map>
#include <Audio.h>
#include <nlohmann/json.hpp>
#include "../../Audio/AudioSystem.h"
#include "../../Scene/Sound/SoundEffect.h"
#include <NoStd.h>
#include <Application.h>

using namespace AudioSystem;
using namespace DirectX;
namespace Templates
{
	std::map<std::string, nlohmann::json> soundTemplates;
	std::map<std::string, std::shared_ptr<DirectX::SoundEffect>> soundEffects;
#if defined(_EDITOR)
	std::map<std::string, std::vector<std::shared_ptr<Scene::SoundEffect>>> soundInstances;
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

	void DrawSoundPanel(std::string sound, ImVec2 pos, ImVec2 size, bool pop)
	{
		nlohmann::json& snd = soundTemplates.at(sound);

		std::string parentFolder = defaultAssetsFolder;
		std::string fileName = "";
		if (snd.contains("path") && !snd.at("path").empty())
		{
			fileName = snd.at("path");
			std::filesystem::path rootFolder = fileName;
			parentFolder = rootFolder.parent_path().string();
		}

		ImDrawFileSelector("##", fileName, [&snd, sound](std::filesystem::path path)
			{
				std::filesystem::path curPath = std::filesystem::current_path();
				std::filesystem::path relPath = std::filesystem::relative(path, curPath);
				snd.at("path") = relPath.string();
				DestroySoundEffectInstances(sound);
				EraseSoundEffect(sound);
				CreateSoundEffectInstances(sound);
			},
			parentFolder, "Sound files. (*.wav)", "*.wav"
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
