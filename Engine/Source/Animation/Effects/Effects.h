#pragma once

#include "DecalLoop.h"
#include "LightOscilation.h"

namespace Scene::Renderable { struct Renderable; };
typedef std::shared_ptr<Scene::Renderable::Renderable> RenderablePtr;
namespace Scene::Lights { struct Light; };
typedef std::shared_ptr<Scene::Lights::Light> LightPtr;
namespace Animation::Effects {

	//creation
	typedef void (*CreateRenderableEffectPtr)(RenderablePtr& renderable, void* constructionData);
	typedef void (*CreateLightEffectPtr)(LightPtr& light, void* constructionData);

	//step
	typedef void (*StepEffectPtr)(FLOAT delta);

	//write cbv
	typedef void (*WriteEffectsConstantsBuffersPtr)(UINT backbufferIndex);

	//destroy
	typedef void (*DestroyEffectPtr)();

	static std::map<std::wstring, CreateRenderableEffectPtr> CreateRenderableEffects = {
		{ DecalLoopEffect, CreateDecalLoop },
	};
	static std::map<std::wstring, CreateLightEffectPtr> CreateLightEffects = {
		{ LightOscilationEffect, CreateLightOscilation },
	};

	static std::map<std::wstring, StepEffectPtr> StepEffects = {
		{ DecalLoopEffect, StepDecalLoop },
		{ LightOscilationEffect, StepLightOscilation },
	};

	static std::map<std::wstring, WriteEffectsConstantsBuffersPtr> WriteEffectsConstantsBuffers = {
		{ DecalLoopEffect, WriteDecalLoopConstantsBuffers },
	};

	static std::map<std::wstring, DestroyEffectPtr> DestroyEffects = {
		{ DecalLoopEffect, DestroyDecalLoops },
		{ LightOscilationEffect, DestroyLightOscilations }
	};

	void EffectsStep(FLOAT delta);
	void WriteEffectsConstantsBuffer(UINT backbufferIndex);
	void EffectsDestroy();
}