#pragma once

namespace Templates::Sound {

	static const std::string templateName = "audio.json";

	struct Sound
	{
		bool systemCreated = false;
		std::string name = "";
		std::string assetPath;
		std::unique_ptr<SoundEffect> sound;
	};

	std::shared_ptr<Sound>& GetSoundTemplate(std::string soundName);
	std::map<std::string, std::shared_ptr<Sound>> GetNamedSounds();
	std::vector<std::string> GetSoundsNames();
	void ReleaseSoundTemplates();

#if defined(_EDITOR)
	void SelectSound(std::string soundName, void*& ptr);
	void DrawSoundPanel(void*& ptr, ImVec2 pos, ImVec2 size);
	std::string GetSoundName(void* ptr);
	nlohmann::json json();
#endif
	Concurrency::task<void> json(std::string, nlohmann::json);

};

typedef Templates::Sound::Sound SoundT;
typedef std::shared_ptr<SoundT> SoundPtr;

