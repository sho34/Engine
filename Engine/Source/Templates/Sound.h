#pragma once

using namespace DirectX;
namespace Templates::Audio {

	static const std::wstring templateName = L"audio.json";

	struct Sound
	{
		bool systemCreated = false;
		std::wstring assetPath;
		std::unique_ptr<SoundEffect> sound;
	};

	concurrency::task<void> CreateSoundTemplate(std::wstring soundName, std::wstring assetPath);
	std::shared_ptr<Sound>& GetSoundTemplate(std::wstring soundName);
	void ReleaseSoundTemplates();

#if defined(_EDITOR)
	nlohmann::json json();
#endif
	Concurrency::task<void> json(std::wstring, nlohmann::json);

};

typedef Templates::Audio::Sound SoundT;
typedef std::shared_ptr<SoundT> SoundPtr;

