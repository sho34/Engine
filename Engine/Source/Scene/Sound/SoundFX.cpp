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
		CreateSoundFXBillboard();
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
		BindSoundFXBillboardToScene();
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
				sfx->UpdateSoundFXBillboard();
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

	void SoundFX::CreateSoundFXBillboard()
	{
		if (!(instanceFlags() & SoundEffectInstance_Use3D)) return;

		nlohmann::json jbillboard = nlohmann::json(
			{
				{ "meshMaterials",
					{
						{
							{ "material", FindMaterialUUIDByName("SoundEffect") },
							{ "mesh", "7dec1229-075f-4599-95e1-9ccfad0d48b1" }
						}
					}
				},
				{ "castShadows", false },
				{ "shadowed", false },
				{ "name" , uuid() + "-billboard" },
				{ "uuid" , getUUID() },
				{ "position" , { 0.0f, 0.0f, 0.0f} },
				{ "topology", "TRIANGLELIST"},
				{ "rotation" , { 0.0, 0.0, 0.0 } },
				{ "scale" , { 1.0f, 1.0f, 1.0f } },
				{ "skipMeshes" , {}},
				{ "visible" , true},
				{ "hidden" , true},
				{ "cameras", { GetMouseCameras().at(0)->uuid()}},
				{ "passMaterialOverrides",
					{
						{
							{ "meshIndex", 0 },
							{ "renderPass", FindRenderPassUUIDByName("PickingPass") },
							{ "material", FindMaterialUUIDByName("SoundEffectPicking") }
						}
					}
				}
			}
		);
		soundFXBillboard = CreateSceneObjectFromJson<Renderable>(jbillboard);
		soundFXBillboard->OnPick = [this] {Editor::SelectSoundEffect(this_ptr); };
	}

	void SoundFX::BindSoundFXBillboardToScene()
	{
		if (!soundFXBillboard) return;
		soundFXBillboard->BindToScene();
	}

	void SoundFX::DestroySoundFXBillboard()
	{
	}

	void SoundFX::UpdateSoundFXBillboard()
	{
		if (!soundFXBillboard) return;
		soundFXBillboard->position(position());
		XMFLOAT3 baseColor = { 1.0f,1.0f,1.0f };
		soundFXBillboard->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, renderer->backBufferIndex);
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
		if (sfx->soundFXBillboard)
		{
			DeleteSceneObject(sfx->soundFXBillboard->uuid());
			sfx->soundFXBillboard->OnPick = [] {};
			sfx->soundFXBillboard = nullptr;
		}
#endif
		sfx->markedForDelete = true;
	}
}