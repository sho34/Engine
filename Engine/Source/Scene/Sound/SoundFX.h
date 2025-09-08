#pragma once

#include <SceneObjectDecl.h>
#include <Json.h>
#include <SceneObject.h>
#include <JTypes.h>

namespace Scene { struct Renderable; };

namespace Scene {
	struct SoundFX;

#include <TrackUUID/JDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Attributes/JOrder.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

	struct SoundFX : SceneObject
	{
		SCENEOBJECT_DECL(SoundFX);

#include <Attributes/JFlags.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <SoundFXAtt.h>
#include <JEnd.h>

		virtual void Initialize();
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
		float Time() const { return time; }

		//3D
		AudioEmitter audioEmitter;
		void UpdateEmmiter();

		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);
	};

	//DESTROY
	void DestroySoundEffects();

#if defined(_EDITOR)
	void WriteSoundEffectsJson(nlohmann::json& json);
#endif

	void SoundEffectsStep(float step);
}
