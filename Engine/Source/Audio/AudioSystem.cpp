#include "pch.h"
#include "AudioSystem.h"
#include "../Templates/Sound/Sound.h"
#include "../Scene/Sound/SoundEffect.h"
#include <Audio.h>

namespace AudioSystem {

	std::unique_ptr<DirectX::AudioEngine> audioEngine;
	DirectX::AudioListener listener;

	void InitAudio()
	{
		//initialize DirectXTK Audio Engine
		//inicializar el Audio Engine de DirectXTK
		AUDIO_ENGINE_FLAGS audioFlags = AudioEngine_Default;
#ifdef _DEBUG
		audioFlags = audioFlags | AudioEngine_Debug;
#endif
		audioEngine = std::make_unique<DirectX::AudioEngine>(audioFlags);
	}

	void ShutdownAudio()
	{
		audioEngine = nullptr;
	}

	void UpdateAudio()
	{
		GetAudioEngine()->Update();
	}

	void UpdateListener(XMFLOAT3 position, XMFLOAT3 forward, XMFLOAT3 up)
	{
		using namespace Scene;

		listener.SetPosition(position);
		listener.SetOrientation(forward, up);
		for (auto& fx : Get3DSoundsEffects())
		{
			fx->soundEffectInstance->Apply3D(listener, fx->audioEmitter);
		}
	}

	std::unique_ptr<DirectX::AudioEngine>& GetAudioEngine()
	{
		return audioEngine;
	}
}
