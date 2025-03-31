#pragma once

namespace Scene {

	enum SoundEffect_UpdateFlags
	{
		SoundEffectFlags_Destroy = 0x1
	};

	struct Renderable;

	struct SoundEffect {

		std::shared_ptr<SoundEffect> this_ptr = nullptr;

		nlohmann::json json;

		std::string uuid();
		void uuid(std::string uuid);

		std::string name();
		void name(std::string name);

		std::string sound();
		void sound(std::string sound);

		float volume();
		void volume(float volume);

		bool autoPlay();
		void autoPlay(bool autoPlay);

		bool hidden() { return false; }

		XMFLOAT3 position();
		void position(XMFLOAT3 f3);
		void position(nlohmann::json f3);

		SOUND_EFFECT_INSTANCE_FLAGS instanceFlags();
		void instanceFlags(SOUND_EFFECT_INSTANCE_FLAGS instanceFlags);

		std::unique_ptr<DirectX::SoundEffectInstance> soundEffectInstance;

		void DetachSoundEffectTemplate();
		void CreateSoundEffectInstance();
		void DestroySoundEffectInstance();

		//3D
		AudioEmitter audioEmitter;

#if defined(_EDITOR)
		unsigned int soundEffectUpdateFlags = 0U;
		void DrawEditorInformationAttributes();
		void DrawEditorWorldAttributes();
		void DrawEditorSoundAttributes();
#endif
		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);
	};

	//CREATE
	std::shared_ptr<SoundEffect> CreateSoundEffect(nlohmann::json soundj);

	//READ&GET
	std::shared_ptr<SoundEffect> GetSoundEffect(std::string uuid);
	std::map<std::string, std::shared_ptr<SoundEffect>> GetSoundsEffects();
	std::vector<std::shared_ptr<SoundEffect>> Get3DSoundsEffects();
	std::vector<std::string> GetSoundEffectsNames();
	std::vector<UUIDName> GetSoundEffectsUUIDNames();

	//UPDATE

	//DESTROY
	void DestroySoundEffects();

#if defined(_EDITOR)
	void SelectSoundEffect(std::string uuid, std::string& edSO);
	void DeSelectSoundEffect(std::string& edSO);
	void DrawSoundEffectPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop);
	std::string GetSoundEffectName(std::string uuid);
	void DeleteSoundEffect(std::string uuid);
	void DrawSoundEffectsPopups();
#endif

	void SoundEffectsStep();
}
