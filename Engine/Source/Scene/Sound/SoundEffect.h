#pragma once

namespace Templates::Sound { struct Sound; }
namespace Scene::SoundEffect {

	struct SoundEffect {
		std::shared_ptr<Templates::Sound::Sound> soundTemplate;
		std::unique_ptr<SoundEffectInstance> soundInstance;

		std::string name = "";
		float volume = 1.0f;
		bool autoPlay = false;
		XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		SOUND_EFFECT_INSTANCE_FLAGS instanceFlags = SoundEffectInstance_Default;

		//3D
		AudioEmitter audioEmitter;

#if defined(_EDITOR)
		nlohmann::json json();
#endif
	};

	std::map<std::string, std::shared_ptr<SoundEffect>> GetSoundsEffects();
	std::vector<std::shared_ptr<SoundEffect>> Get3DSoundsEffects();
	std::vector<std::string> GetSoundEffectsNames();
#if defined(_EDITOR)
	void SelectSoundEffect(std::string soundEffectName, void*& ptr);
	void DrawSoundEffectPanel(void*& ptr, ImVec2 pos, ImVec2 size);
	std::string GetSoundEffectName(void* ptr);
#endif
	std::shared_ptr<SoundEffect> CreateSoundEffect(nlohmann::json soundj);
	void DestroySoundEffects();
}


typedef Scene::SoundEffect::SoundEffect SoundEffectT;
typedef std::shared_ptr<SoundEffectT> SoundEffectPtr;