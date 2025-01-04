#include "pch.h"
#include "SoundEffect.h"
#include "../../Templates/Sound/SoundImpl.h"

namespace Scene::SoundEffect {
  
  std::map<std::string, SoundEffectPtr> soundsEffects;
  std::vector<SoundEffectPtr> sounds3DEffects;

  std::map<std::string, std::shared_ptr<SoundEffect>> GetSoundsEffects() { return soundsEffects; }
  std::vector<SoundEffectPtr> Get3DSoundsEffects() { return sounds3DEffects; }

  std::vector<std::string> GetSoundEffectsNames() {
    std::vector<std::string> names;
    std::transform(soundsEffects.begin(), soundsEffects.end(), std::back_inserter(names), [](const std::pair<std::string, std::shared_ptr<SoundEffect>> pair) { return pair.second->name; });
    return names;
  }

#if defined(_EDITOR)
  void SelectSoundEffect(std::string soundEffectName, void*& ptr) {
    ptr = soundsEffects.at(soundEffectName).get();
  }

  void DrawSoundEffectPanel(void*& ptr, ImVec2 pos, ImVec2 size)
  {
  }

  std::string GetSoundEffectName(void* ptr)
  {
    SoundEffect* soundEffect = (SoundEffect*)ptr;
    return soundEffect->name;
  }
#endif

  std::shared_ptr<SoundEffect> CreateSoundEffect(nlohmann::json soundj)
  {
    SoundEffectPtr fx = std::make_shared<SoundEffect>();

    std::string name = soundj["name"];
    std::string templateName = soundj["sound"];
    fx->name = name;
    fx->soundTemplate = GetSoundTemplate(templateName);

    SOUND_EFFECT_INSTANCE_FLAGS instanceCreationFlags = SoundEffectInstance_Default;
    if (soundj.contains("instanceFlags")) {
      instanceCreationFlags = stringToSoundEffectInstanceFlag[soundj["instanceFlags"]];
    }
    fx->instanceFlags = instanceCreationFlags;
    fx->soundInstance = fx->soundTemplate->sound->CreateInstance(instanceCreationFlags);

    if(soundj.contains("volume")) {
      fx->volume = soundj["volume"];
      fx->soundInstance->SetVolume(fx->volume);
    }
    if (soundj.contains("autoPlay") && soundj["autoPlay"]==true) {
      fx->autoPlay = true;
      fx->soundInstance->Play(true);
    }

    soundsEffects[name] = fx;

    if (bytesHas(instanceCreationFlags,SoundEffectInstance_Use3D)) {
      fx->position = JsonToFloat3(soundj["position"]);
      fx->audioEmitter.SetPosition(fx->position);
      sounds3DEffects.push_back(fx);
    }

    return fx;
  }

#if defined(_EDITOR)
  nlohmann::json SoundEffect::json() {
    nlohmann::json soundj = {
      { "name", name },
      { "sound", soundTemplate->name },
      { "volume", volume },
      { "autoPlay", autoPlay },
      { "position", { position.x, position.y, position.z } },
      { "instanceFlags", soundEffectInstanceFlagToString[instanceFlags] }
    };
    return soundj;
  }
#endif

  void DestroySoundEffects()
  {
    for (auto& [name, fx] : soundsEffects) {
      fx->soundInstance = nullptr;
    }

    soundsEffects.clear();
    sounds3DEffects.clear();
  }

}