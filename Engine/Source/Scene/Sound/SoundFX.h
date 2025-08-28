#pragma once

#include <SceneObjectDecl.h>
#include <Json.h>
#include <SceneObject.h>
#include <JExposeTypes.h>

namespace Scene { struct Renderable; };

namespace Scene {
	struct SoundFX;

#include <JExposeTrackUUIDDecl.h>
#include <SoundFXAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttOrder.h>
#include <SoundFXAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttDrawersDecl.h>
#include <SoundFXAtt.h>
#include <JExposeEnd.h>

	struct SoundFX : SceneObject
	{
		SCENEOBJECT_DECL(SoundFX);

#include <JExposeAttFlags.h>
#include <SoundFXAtt.h>
#include <JExposeEnd.h>

#include <JExposeDecl.h>
#include <SoundFXAtt.h>
#include <JExposeEnd.h>

		void Destroy();

		XMVECTOR rotationQ();
		XMMATRIX world();
		XMVECTOR fw();

		std::tuple<std::unique_ptr<DirectX::SoundEffect>, std::unique_ptr<DirectX::SoundEffectInstance>> soundEffectInstance;

		std::unique_ptr<DirectX::SoundEffect>& GetEffect() { return std::get<0>(soundEffectInstance); }
		std::unique_ptr<DirectX::SoundEffectInstance>& GetInstance() { return std::get<1>(soundEffectInstance); }
		bool Play();
		bool Stop();
		bool Pause();
		bool Resume();
		bool IsPlaying() { return GetInstance()->GetState() == DirectX::SoundState::PLAYING; }
		bool IsPaused() { return GetInstance()->GetState() == DirectX::SoundState::PAUSED; }
		float Duration() { return (GetEffect()->GetSampleDurationMS() / 1000.0f); }
		float time = 0.0f;
		void Step(float step);
		float Time() { return time; }

		//3D
		AudioEmitter audioEmitter;
		void UpdateEmmiter();

		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);
	};

	//DESTROY
	void DestroySoundEffect(std::shared_ptr<SoundFX>& sfx);
	void DestroySoundEffects();

#if defined(_EDITOR)
	void WriteSoundEffectsJson(nlohmann::json& json);
#endif

	void SoundEffectsStep(float step);
}
