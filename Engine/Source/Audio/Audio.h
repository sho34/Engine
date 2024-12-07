#pragma once

using namespace DirectX;
namespace Templates::Audio { struct Sound; };
namespace Audio {

	struct SoundInstanceDefinition {
		SOUND_EFFECT_INSTANCE_FLAGS instanceFlags = SoundEffectInstance_Default;

		bool autoPlay = false;
		float volume = 1.0f;
		
		//3D
		XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	};

	struct SoundInstance {
		std::shared_ptr<Templates::Audio::Sound> soundTemplate;

		std::unique_ptr<SoundEffectInstance> soundInstance;

		//3D
		AudioEmitter audioEmitter;
	};

	void InitAudio();
	void ShutdownAudio();
	void UpdateAudio();
	void UpdateListener(XMFLOAT3 position, XMFLOAT3 forward, XMFLOAT3 up);
	
	std::unique_ptr<AudioEngine>& GetAudio();

	std::shared_ptr<SoundInstance> CreateInstance(std::wstring soundName, std::wstring soundInstanceName, SoundInstanceDefinition def = {} );

	void DestroySounds();
	
}

typedef Audio::SoundInstance SoundInstanceT;
typedef std::shared_ptr<SoundInstanceT> SoundInstancePtr;
