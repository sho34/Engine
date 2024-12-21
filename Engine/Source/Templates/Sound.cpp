#include "pch.h"
#include "Sound.h"
#include "../Audio/Audio.h"
#include "../Types.h"

std::map<std::wstring, SoundPtr> soundTemplates;

using namespace Audio;
namespace Templates::Audio {

	std::mutex soundCreateMutex;
	concurrency::task<void> CreateSoundTemplate(std::wstring soundName, std::wstring assetPath)
	{
		assert(!soundTemplates.contains(soundName));

		return concurrency::create_task([soundName, assetPath] {
			std::lock_guard<std::mutex> lock(soundCreateMutex);

			SoundPtr sound = std::make_shared<SoundT>();
			sound->assetPath = assetPath;
			sound->sound = std::make_unique<SoundEffect>(GetAudio().get(), assetPath.c_str());

			soundTemplates.insert_or_assign(soundName,sound);
			assert(soundTemplates.contains(soundName));
		});
	}

	std::shared_ptr<Sound>& GetSoundTemplate(std::wstring soundName) {
		assert(soundTemplates.contains(soundName));

		return soundTemplates[soundName];
	}

	void ReleaseSoundTemplates()
	{
		for (auto& [name, sound] : soundTemplates) {
			sound->sound = nullptr;
		}
		soundTemplates.clear();
	}

#if defined(_EDITOR)
	nlohmann::json json()
	{
		nlohmann::json j = nlohmann::json({});
		for (auto& [name, sound] : soundTemplates) {
			if (sound->systemCreated) continue;
			std::string jname = WStringToString(name);
			j[jname] = nlohmann::json({});
			j[jname]["assetPath"] = WStringToString(sound->assetPath);
		}
		return j;
	}
#endif

	Concurrency::task<void> json(std::wstring name, nlohmann::json soundj)
	{
		assert(!soundTemplates.contains(name));

		std::wstring assetPath = StringToWString(soundj["assetPath"]);

		return concurrency::create_task([name, assetPath] {
			std::lock_guard<std::mutex> lock(soundCreateMutex);

			SoundPtr sound = std::make_shared<SoundT>();
			sound->assetPath = assetPath;
			sound->sound = std::make_unique<SoundEffect>(GetAudio().get(), assetPath.c_str());

			soundTemplates.insert_or_assign(name, sound);
			assert(soundTemplates.contains(name));
		});
	}
}
