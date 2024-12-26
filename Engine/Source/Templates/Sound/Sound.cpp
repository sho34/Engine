#include "pch.h"
#include "Sound.h"
#include "../../Audio/Audio.h"

std::map<std::wstring, SoundPtr> soundTemplates;

using namespace Audio;
namespace Templates::Sound {

	std::mutex soundCreateMutex;

	std::shared_ptr<Sound>& GetSoundTemplate(std::wstring soundName) {
		assert(soundTemplates.contains(soundName));

		return soundTemplates[soundName];
	}

	std::map<std::wstring, std::shared_ptr<Sound>> GetNamedSounds() {
		return soundTemplates;
	}

	std::vector<std::wstring> GetSoundsNames() {
		std::vector<std::wstring> names;
		std::transform(soundTemplates.begin(), soundTemplates.end(), std::back_inserter(names), [](std::pair<std::wstring, std::shared_ptr<Sound>> pair) { return pair.first; });
		return names;
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
			sound->name = name;
			sound->assetPath = assetPath;
			sound->sound = std::make_unique<SoundEffect>(GetAudio().get(), assetPath.c_str());

			soundTemplates.insert_or_assign(name, sound);
			assert(soundTemplates.contains(name));
		});
	}
}
