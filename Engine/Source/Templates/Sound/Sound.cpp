#include "pch.h"
#include "Sound.h"
#include "../../Audio/Audio.h"

std::map<std::string, SoundPtr> soundTemplates;

using namespace Audio;
namespace Templates::Sound {

	std::mutex soundCreateMutex;

	std::shared_ptr<Sound>& GetSoundTemplate(std::string soundName) {
		assert(soundTemplates.contains(soundName));

		return soundTemplates[soundName];
	}

	std::map<std::string, std::shared_ptr<Sound>> GetNamedSounds() {
		return soundTemplates;
	}

	std::vector<std::string> GetSoundsNames() {
		std::vector<std::string> names;
		std::transform(soundTemplates.begin(), soundTemplates.end(), std::back_inserter(names), [](std::pair<std::string, std::shared_ptr<Sound>> pair) { return pair.first; });
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
	void SelectSound(std::string soundName, void*& ptr) {
		ptr = soundTemplates.at(soundName).get();
	}

	void DrawSoundPanel(void*& ptr, ImVec2 pos, ImVec2 size)
	{
	}

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
#endif

	Concurrency::task<void> json(std::string name, nlohmann::json soundj)
	{
		assert(!soundTemplates.contains(name));

		std::string assetPath = soundj["assetPath"];

		return concurrency::create_task([name, assetPath] {
			std::lock_guard<std::mutex> lock(soundCreateMutex);

			SoundPtr sound = std::make_shared<SoundT>();
			sound->name = name;
			sound->assetPath = assetPath;
			sound->sound = std::make_unique<SoundEffect>(GetAudio().get(), StringToWString(assetPath).c_str());

			soundTemplates.insert_or_assign(name, sound);
			assert(soundTemplates.contains(name));
		});
	}
}
