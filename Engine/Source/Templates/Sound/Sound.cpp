#include "pch.h"
#include "Sound.h"
#include <map>
#include <set>
#include <Audio.h>
#include <nlohmann/json.hpp>
#include <AudioSystem.h>
#include <Sound/SoundFX.h>
#include <NoStd.h>
#include <Application.h>
#if defined(_EDITOR)
#include <TemplateDef.h>
#endif

using namespace AudioSystem;
using namespace DirectX;

namespace Templates
{
#include <Editor/JDrawersDef.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <SoundAtt.h>
#include <JEnd.h>

	namespace Sound
	{
		std::map<std::string, std::unique_ptr<DirectX::SoundEffect>> uuidToSoundEffects;
		std::map<std::string, unsigned int> uuidInstanceCount;
	};

	std::tuple<std::unique_ptr<DirectX::SoundEffect>, std::unique_ptr<DirectX::SoundEffectInstance>>
		GetSoundEffectInstance(std::string uuid, unsigned int flags,
			std::string objectUUID, JObjectChangeCallback cb, JObjectChangePostCallback postCb)
	{
		if (objectUUID != "" && (cb != nullptr || postCb != nullptr))
		{
			std::shared_ptr<SoundJson> json = GetSoundTemplate(uuid);
			json->BindChangeCallback(objectUUID, cb, postCb);
		}
		using namespace Sound;
		if (uuidToSoundEffects.contains(uuid))
		{
			uuidInstanceCount[uuid]++;
		}
		else
		{
			std::shared_ptr<SoundJson> json = GetSoundTemplate(uuid);
			std::wstring path = nostd::StringToWString(defaultSoundsFolder + json->path());
			uuidToSoundEffects[uuid] = std::make_unique<DirectX::SoundEffect>(GetAudioEngine().get(), path.c_str());
			uuidInstanceCount[uuid] = 1;
		}
		return std::make_tuple(std::move(uuidToSoundEffects[uuid]), std::move(uuidToSoundEffects[uuid]->CreateInstance(SOUND_EFFECT_INSTANCE_FLAGS(flags))));
	}

	void DestroySoundEffectInstance(std::string uuid, std::tuple<
		std::unique_ptr<DirectX::SoundEffect>,
		std::unique_ptr<DirectX::SoundEffectInstance>
	>& soundEffectInstance)
	{
		using namespace Sound;
		uuidInstanceCount[uuid]--;
		std::unique_ptr<DirectX::SoundEffectInstance>& sfxI = std::get<1>(soundEffectInstance);
		sfxI = nullptr;
		soundEffectInstance = std::make_tuple(nullptr, nullptr);
		if (uuidInstanceCount[uuid] == 0)
		{
			uuidToSoundEffects.erase(uuid);
			uuidInstanceCount.erase(uuid);
		}
	}



	SoundJson::SoundJson(nlohmann::json json) : JTemplate(json)
	{
#include <Attributes/JInit.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <SoundAtt.h>
#include <JEnd.h>
	}

	TEMPDEF_FULL(Sound);

	void SoundJsonStep()
	{
		std::set<std::shared_ptr<SoundJson>> sounds;
		std::transform(Soundtemplates.begin(), Soundtemplates.end(), std::inserter(sounds, sounds.begin()), [](auto& temps)
			{
				auto& matJ = std::get<1>(temps.second);
				return matJ;
			}
		);

		std::set<std::shared_ptr<SoundJson>> rebuildSounds;
		std::copy_if(sounds.begin(), sounds.end(), std::inserter(rebuildSounds, rebuildSounds.begin()), [](auto& sound)
			{
				return sound->dirty(SoundJson::Update_path);
			}
		);

		if (rebuildSounds.size() > 0ULL)
		{
			JObject::RunChangesCallback(rebuildSounds, [](auto sound)
				{
					sound->clean(SoundJson::Update_path);
				}
			);
		}
	}

	void ReleaseSoundEffectsInstances()
	{
		//for (auto& [uuid, t] : sounds)
		//{
		//	auto& instances = std::get<3>(t);
		//	instances.clear();
		//}
	}
}
