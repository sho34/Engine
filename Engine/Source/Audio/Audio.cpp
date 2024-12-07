#include "pch.h"
#include "Audio.h"
#include "../Templates/Sound.h"

namespace Audio {

  std::unique_ptr<AudioEngine> audio;
  std::map<std::wstring, SoundInstancePtr> soundsInstances;
  std::vector<SoundInstancePtr> sounds3DInstances;
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

  void UpdateAudio() { GetAudio()->Update(); }

  void UpdateListener(XMFLOAT3 position, XMFLOAT3 forward, XMFLOAT3 up) {
    listener.SetPosition(position);
    listener.SetOrientation(forward, up);
    for (auto& ins : sounds3DInstances) {
      ins->soundInstance->Apply3D(listener, ins->audioEmitter);
    }
  }

  std::unique_ptr<AudioEngine>& GetAudio() {
    return audio;
  }

  std::shared_ptr<SoundInstance> CreateInstance(std::wstring soundTemplateName, std::wstring soundInstanceName, SoundInstanceDefinition def)
  {
    using namespace Templates::Audio;

    auto temp = GetSoundTemplate(soundTemplateName);

    SoundInstancePtr ins = std::make_shared<SoundInstance>();

    ins->soundInstance = temp->sound->CreateInstance(def.instanceFlags);
    auto& sfx = ins->soundInstance;
    if (def.autoPlay) sfx->Play(true);
    sfx->SetVolume(def.volume);
    soundsInstances[soundInstanceName] = ins;

    if ((def.instanceFlags & SoundEffectInstance_Use3D) == SoundEffectInstance_Use3D) {
      ins->audioEmitter.SetPosition(def.position);
      sounds3DInstances.push_back(ins);
    }

    return ins;
  }

}
