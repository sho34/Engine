#include "pch.h"
#include "SoundFX.h"
#include <Scene.h>
#include <Templates.h>
#include <NoStd.h>
#include <AudioSystem.h>
#include <Renderable/Renderable.h>
#include <Sound/Sound.h>
#include <Renderer.h>

extern std::shared_ptr<Renderer> renderer;

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectSoundEffect(std::shared_ptr<SoundFX> soundEffect);
	extern std::shared_ptr<Renderable> CreateBillboardFromMaterials(std::string name, std::string material, std::string pickingMaterial);
	extern void RegisterBillboard(std::shared_ptr<SceneObject> sceneObject);
	extern std::shared_ptr<Renderable> GetBillboard(std::shared_ptr<SceneObject> sceneObject);
	extern void DestroyBillboard(std::shared_ptr<SceneObject> sceneObject);
}
#endif

using namespace Templates;
namespace Scene {

#include <Editor/JDrawersDef.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <TrackUUID/JDef.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

	SoundFX::SoundFX(nlohmann::json json) : SceneObject(json)
	{
#include <Attributes/JInit.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}

	void SoundFX::Initialize()
	{
#include <TrackUUID/JInsert.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
		if (instanceFlags() & SoundEffectInstance_Use3D)
			Editor::RegisterBillboard(this_ptr);
#endif
	}

	void SoundFX::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

		if (!sound().empty())
		{
			auto OnSoundChange = [this](std::shared_ptr<JObject> sound)
				{
					UnbindFromScene();
					BindToScene();
				};
			soundEffectInstance = GetSoundEffectInstance(sound(), instanceFlags(), uuid(), OnSoundChange);
		}
		if (nostd::bytesHas(instanceFlags(), SoundEffectInstance_Use3D))
		{
			audioEmitter.SetPosition(position());
			audioEmitter.SetOrientationFromQuaternion(rotationQ());
		}
		if (std::get<0>(soundEffectInstance) != nullptr && autoPlay())
		{
			Play();
		}
#if defined(_EDITOR)
		SceneObject::BindToScene();
#endif
	}

	void SoundFX::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

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
#include <Editor/JSaveFile.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}
#endif

	void SoundEffectsStep(float step)
	{
		std::set<std::shared_ptr<SoundFX>> sfxs;
		std::transform(SoundEffects.begin(), SoundEffects.end(), std::inserter(sfxs, sfxs.end()), [](auto& pair) { return pair.second; });

		std::for_each(sfxs.begin(), sfxs.end(), [step](auto& sfx)
			{
				sfx->Step(step);
#if defined(_EDITOR)
				sfx->UpdateBillboard(Editor::GetBillboard(sfx));
#endif
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

		std::set<std::shared_ptr<SoundFX>> sfxsDelete;
		std::copy_if(sfxs.begin(), sfxs.end(), std::inserter(sfxsDelete, sfxsDelete.end()), [](auto& sfx)
			{
				return sfx->markedForDelete;
			}
		);

		for (auto& sfx : sfxsDelete)
		{
			EraseSoundFXFromSoundEffects(sfx);
			EraseSoundFXFromSound3DEffects(sfx);
			std::shared_ptr<SoundFX> soundfx = sfx;
			SafeDeleteSceneObject(soundfx);
		}
	}

	void SoundFX::UpdateEmmiter()
	{
		using namespace AudioSystem;
		GetInstance()->Apply3D(GetAudioListener(), audioEmitter, false);
	}

#if defined(_EDITOR)

	void SoundFX::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox)
	{
		bbox->position(position());
		bbox->scale(XMFLOAT3({ 0.3f, 0.3f, 0.3f }));
		bbox->rotation(XMFLOAT3({ 0.0f, 0.0f, 0.0f }));
	}

	std::shared_ptr<Renderable> SoundFX::CreateBillboard()
	{
		if (!(instanceFlags() & SoundEffectInstance_Use3D)) return nullptr;

		std::shared_ptr<Renderable> billboard = Editor::CreateBillboardFromMaterials(at("name"), "SoundEffect", "SoundEffectPicking");
		billboard->OnPick = [this] {Editor::SelectSoundEffect(this_ptr); };
		UpdateBillboard(billboard);
		return billboard;
	}

	void SoundFX::UpdateBillboard(std::shared_ptr<Renderable> billboard)
	{
		if (!billboard) return;
		billboard->position(position());
		XMFLOAT3 baseColor = { 1.0f,1.0f,1.0f };
		billboard->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, renderer->backBufferIndex);
		billboard->WriteConstantsBuffer(renderer->backBufferIndex);

	}

	BoundingBox SoundFX::GetBoundingBox()
	{
		return BoundingBox(position(), { 0.1f,0.1f,0.1f });
	}
#endif

	void DestroySoundEffects()
	{
		auto tmp = SoundEffects;
		for (auto& [_, sfx] : tmp) {
			SafeDeleteSceneObject(sfx);
		}

#include <TrackUUID/JClear.h>
#include <SoundFXAtt.h>
#include <JEnd.h>
	}
	void DeleteSoundEffect(std::string uuid)
	{
		std::shared_ptr<SoundFX> sfx = FindInSoundEffects(uuid);
#if defined(_EDITOR)
		Editor::DestroyBillboard(sfx);
#endif
		sfx->markedForDelete = true;
	}
}