#pragma once

namespace Scene {

	struct Renderable;

	struct SoundEffect {

		std::shared_ptr<SoundEffect> this_ptr = nullptr;

		std::string name = "";
		std::string sound = "";
		float volume = 1.0f;
		bool autoPlay = false;
		XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		SOUND_EFFECT_INSTANCE_FLAGS instanceFlags = SoundEffectInstance_Default;

		std::unique_ptr<DirectX::SoundEffectInstance> soundEffectInstance;

		void CreateSoundEffectInstance();
		void DestroySoundEffectInstance();

		//3D
		AudioEmitter audioEmitter;

#if defined(_EDITOR)
		void DrawEditorInformationAttributes();
		void DrawEditorWorldAttributes();
		void DrawEditorSoundAttributes();
		nlohmann::json json();
#endif
		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);
	};

	//CREATE
	std::shared_ptr<SoundEffect> CreateSoundEffect(nlohmann::json soundj);

	//READ&GET
	std::map<std::string, std::shared_ptr<SoundEffect>> GetSoundsEffects();
	std::vector<std::shared_ptr<SoundEffect>> Get3DSoundsEffects();
	std::vector<std::string> GetSoundEffectsNames();

	//UPDATE

	//DESTROY
	void DestroySoundEffects();

#if defined(_EDITOR)
	void SelectSoundEffect(std::string soundEffectName, void*& ptr);
	void DeSelectSoundEffect(void*& ptr);
	void DrawSoundEffectPanel(void*& ptr, ImVec2 pos, ImVec2 size, bool pop);
	std::string GetSoundEffectName(void* ptr);
#endif

}
