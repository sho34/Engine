#include "pch.h"
#include "Audio.h"
#include "../Templates/Sound/SoundImpl.h"
#include "../Scene/Sound/SoundEffectImpl.h"

namespace Audio {

  std::unique_ptr<AudioEngine> audio;
  AudioListener listener;

	void InitAudio()
	{
    //initialize DirectXTK Audio Engine
    //inicializar el Audio Engine de DirectXTK
    AUDIO_ENGINE_FLAGS audioFlags = AudioEngine_Default;
#ifdef _DEBUG
    audioFlags = audioFlags | AudioEngine_Debug;
#endif
    audio = std::make_unique<AudioEngine>(audioFlags);

	}

  void ShutdownAudio()
  {
    audio = nullptr;
  }

  void UpdateAudio() { GetAudio()->Update(); }

  void UpdateListener(XMFLOAT3 position, XMFLOAT3 forward, XMFLOAT3 up) {
    listener.SetPosition(position);
    listener.SetOrientation(forward, up);
    for (auto& fx : Get3DSoundsEffects()) {
      fx->soundInstance->Apply3D(listener, fx->audioEmitter);
    }
  }

  std::unique_ptr<AudioEngine>& GetAudio() {
    return audio;
  }

}
