#pragma once

using namespace DirectX;
namespace Templates::Audio {

	struct Sound
	{
		std::unique_ptr<SoundEffect> sound;
	};

	concurrency::task<void> CreateSoundTemplate(std::wstring soundName, std::wstring assetPath);
	std::shared_ptr<Sound> GetSoundTemplate(std::wstring soundName);
};

typedef Templates::Audio::Sound SoundT;
typedef std::shared_ptr<SoundT> SoundPtr;

