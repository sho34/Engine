#pragma once

#include "DecalLoop.h"
#include "LightOscilation.h"

namespace Renderable { struct Renderable; };
namespace Scene::Lights { struct Light; };
namespace Animation::Effects {

	//creation
	typedef void (*CreateRenderableEffectPtr)(std::shared_ptr<Renderable::Renderable> renderable, void* constructionData);
	typedef void (*CreateLightEffectPtr)(std::shared_ptr<Scene::Lights::Light> light, void* constructionData);

	//step
	typedef void (*StepEffectPtr)(FLOAT delta);

	//write cbv
	typedef void (*WriteEffectsConstantsBuffersPtr)(UINT backbufferIndex);

	static std::map<std::wstring, CreateRenderableEffectPtr> CreateRenderableEffects = {
		{ L"DecalLoop", CreateDecalLoop },
	};
	static std::map<std::wstring, CreateLightEffectPtr> CreateLightEffects = {
		{ L"LightOscilation", CreateLightOscilation },
	};

	static std::map<std::wstring, StepEffectPtr> StepEffects = {
		{ L"DecalLoop", StepDecalLoop },
		{ L"LightOscilation", StepLightOscilation },
	};

	static std::map<std::wstring, WriteEffectsConstantsBuffersPtr> WriteEffectsConstantsBuffers = {
		{ L"DecalLoop", WriteDecalLoopConstantsBuffers },
	};

	void EffectsStep(FLOAT delta);
	void WriteEffectsConstantsBuffer(UINT backbufferIndex);
}