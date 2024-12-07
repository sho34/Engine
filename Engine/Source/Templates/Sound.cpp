#include "pch.h"
#include "Sound.h"
#include "../Audio/Audio.h"

std::map<std::wstring, SoundPtr> soundTemplates;

using namespace Audio;
namespace Templates::Audio {

	concurrency::task<void> CreateSoundTemplate(std::wstring soundName, std::wstring assetPath)
	{
		assert(!soundTemplates.contains(soundName));

		return concurrency::create_task([soundName, assetPath] {
			SoundPtr sound = std::make_shared<SoundT>();
			sound->sound = std::make_unique<SoundEffect>(GetAudio().get(), assetPath.c_str());

			soundTemplates[soundName] = sound;
		});
	}

	std::shared_ptr<Sound> GetSoundTemplate(std::wstring soundName) {
		return soundTemplates[soundName];
	}

}
