#pragma once

namespace Templates::Sound {

	static const std::wstring templateName = L"audio.json";

	struct Sound
	{
		bool systemCreated = false;
		std::wstring name = L"";
		std::wstring assetPath;
		std::unique_ptr<SoundEffect> sound;
	};

	std::shared_ptr<Sound>& GetSoundTemplate(std::wstring soundName);
	std::map<std::wstring, std::shared_ptr<Sound>> GetNamedSounds();
	std::vector<std::wstring> GetSoundsNames();
	void ReleaseSoundTemplates();

#if defined(_EDITOR)
	nlohmann::json json();
#endif
	Concurrency::task<void> json(std::wstring, nlohmann::json);

};

typedef Templates::Sound::Sound SoundT;
typedef std::shared_ptr<SoundT> SoundPtr;

