#include "pch.h"
#include "SoundFX.h"
#include <Scene.h>
#include <Templates.h>
#include <NoStd.h>
#include <AudioSystem.h>
#include <Renderable/Renderable.h>
#include <Sound/Sound.h>

using namespace Templates;
namespace Scene {

#include <JExposeAttDrawersDef.h>
#include <SoundFXAtt.h>
#include <JExposeEnd.h>

#include <JExposeTrackUUIDDef.h>
#include <SoundFXAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttJsonDef.h>
#include <SoundFXAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttCreatorDrawersDef.h>
#include <SoundFXAtt.h>
#include <JExposeEnd.h>

	SoundFX::SoundFX(nlohmann::json json) : SceneObject(json)
	{
#include <JExposeInit.h>
#include <SoundFXAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttUpdate.h>
#include <SoundFXAtt.h>
#include <JExposeEnd.h>
	}

	void SoundFX::BindToScene()
	{
		SoundEffects.insert_or_assign(uuid(), this_ptr);

		if (!sound().empty())
		{
			auto OnSoundChange = [this](std::shared_ptr<JObject> sound)
				{
					UnbindFromScene();
					BindToScene();
				};
			soundEffectInstance = GetSoundEffectInstance(sound(), instanceFlags(), uuid(), OnSoundChange);

			if (nostd::bytesHas(instanceFlags(), SoundEffectInstance_Use3D))
			{
				audioEmitter.SetPosition(position());
				audioEmitter.SetOrientationFromQuaternion(rotationQ());
				Sound3DEffects.insert_or_assign(uuid(), this_ptr);
			}

			if (autoPlay())
			{
				Play();
			}
		}
	}

	void SoundFX::UnbindFromScene()
	{
		SoundEffects.erase(uuid());
		Sound3DEffects.erase(uuid());

		if (GetEffect() != nullptr)
		{
			if (dirty(SoundFX::Update_sound))
			{
				std::string prevSoundUUID = UpdatePrevValues.at("sound");
				DestroySoundEffectInstance(prevSoundUUID, soundEffectInstance);
			}
			else
			{
				DestroySoundEffectInstance(sound(), soundEffectInstance);
			}
		}
	}

	bool SoundFX::Play()
	{
		auto& sfxI = GetInstance();
		if (sfxI->GetState() == DirectX::SoundState::PLAYING) return false;
		sfxI->SetVolume(volume());
		sfxI->Play(loop());
		time = 0.0f;
		return true;
	}

	bool SoundFX::Stop()
	{
		auto& sfxI = GetInstance();
		if (sfxI->GetState() == DirectX::SoundState::STOPPED) return false;
		sfxI->Stop();
		time = 0.0f;
		return true;
	}

	bool SoundFX::Pause()
	{
		auto& sfxI = GetInstance();
		if (sfxI->GetState() == DirectX::SoundState::PAUSED) return false;
		sfxI->Pause();
		return true;
	}

	bool SoundFX::Resume()
	{
		auto& sfxI = GetInstance();
		if (sfxI->GetState() != DirectX::SoundState::PAUSED) return false;
		sfxI->Resume();
		return true;
	}

	void SoundFX::Step(float step)
	{
		if (!IsPlaying()) return;

		time += step;
		float duration = Duration();
		if (!loop())
		{
			time = std::min(time, duration);
		}
		else if (time > duration)
		{
			time = fmodf(time, duration);
		}
	}

	void SoundFX::Destroy()
	{
		soundEffectInstance = std::make_tuple(nullptr, nullptr);
	}

	XMVECTOR SoundFX::rotationQ()
	{
		XMFLOAT3 rotV = rotation();
		float roll, pitch, yaw;
		pitch = rotV.x; yaw = rotV.y; roll = rotV.z;
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
		return rotQ;
	}

	XMMATRIX SoundFX::world()
	{
		XMFLOAT3 posV = position();
		XMMATRIX rotationM = XMMatrixRotationQuaternion(rotationQ());
		XMMATRIX positionM = XMMatrixTranslationFromVector({ posV.x, posV.y, posV.z });
		return XMMatrixMultiply(rotationM, positionM);
	}

	XMVECTOR SoundFX::fw()
	{
		FXMVECTOR dir = { 0.0f, 0.0f, 1.0f,0.0f };
		XMVECTOR fw = XMVector3Normalize(XMVector3Rotate(dir, rotationQ()));
		return fw;
	}

#if defined(_EDITOR)

	void WriteSoundEffectsJson(nlohmann::json& json)
	{
		std::map<std::string, std::shared_ptr<SoundFX>> filtered;
		std::copy_if(SoundEffects.begin(), SoundEffects.end(), std::inserter(filtered, filtered.end()), [](const auto& pair)
			{
				auto& [uuid, sfx] = pair;
				return !sfx->hidden();
			}
		);
		std::transform(filtered.begin(), filtered.end(), std::back_inserter(json), [](const auto& pair)
			{
				auto& [uuid, sfx] = pair;
				return *(static_cast<nlohmann::json*>(sfx.get()));
			}
		);
	}
#endif

	void SoundEffectsStep(float step)
	{
		std::set<std::shared_ptr<SoundFX>> sfxs;
		std::transform(SoundEffects.begin(), SoundEffects.end(), std::inserter(sfxs, sfxs.end()), [](auto& pair) { return pair.second; });

		std::for_each(sfxs.begin(), sfxs.end(), [step](auto& sfx)
			{
				sfx->Step(step);
			}
		);

		std::set<std::shared_ptr<SoundFX>> sfxsDestroyI;
		std::set<std::shared_ptr<SoundFX>> sfxsCreateI;
		std::copy_if(sfxs.begin(), sfxs.end(), std::inserter(sfxsDestroyI, sfxsDestroyI.end()), [](auto& sfx)
			{
				return sfx->dirty(SoundFX::Update_sound) || sfx->dirty(SoundFX::Update_loop) || sfx->dirty(SoundFX::Update_instanceFlags);
			}
		);

		std::copy_if(sfxs.begin(), sfxs.end(), std::inserter(sfxsCreateI, sfxsCreateI.end()), [](auto& sfx)
			{
				if (!sfx->dirty(SoundFX::Update_sound) && (sfx->dirty(SoundFX::Update_loop) || sfx->dirty(SoundFX::Update_instanceFlags)))
					return true;

				return (sfx->dirty(SoundFX::Update_sound) && !sfx->sound().empty());
			}
		);

		std::for_each(sfxsDestroyI.begin(), sfxsDestroyI.end(), [](auto& sfx)
			{
				sfx->UnbindFromScene();
			}
		);

		std::for_each(sfxsCreateI.begin(), sfxsCreateI.end(), [](auto& sfx)
			{
				sfx->BindToScene();
			}
		);

		std::for_each(sfxs.begin(), sfxs.end(), [step](auto& sfx)
			{
				sfx->clear();
			}
		);
	}

	void SoundFX::UpdateEmmiter()
	{
		using namespace AudioSystem;
		GetInstance()->Apply3D(GetAudioListener(), audioEmitter, false);
	}

	void SoundFX::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox)
	{
		bbox->position(position());
		bbox->scale(XMFLOAT3({ 0.3f, 0.3f, 0.3f }));
		bbox->rotation(XMFLOAT3({ 0.0f, 0.0f, 0.0f }));
	}

	void DestroySoundEffect(std::shared_ptr<SoundFX>& sfx)
	{
		if (sfx == nullptr) return;
		DEBUG_PTR_COUNT_JSON(sfx);

		sfx->UnbindFromScene();
		sfx->this_ptr = nullptr;
		sfx = nullptr;
	}

	void DestroySoundEffects()
	{
		auto tmp = SoundEffects;
		for (auto& [_, sfx] : tmp) {
			DestroySoundEffect(sfx);
		}

		SoundEffects.clear();
		Sound3DEffects.clear();
	}
}