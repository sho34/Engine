#pragma once

#include "DecalLoop.h"
#include "LightOscilation.h"

namespace Scene::Renderable { struct Renderable; };
namespace Scene::Lights { struct Light; };

typedef std::shared_ptr<Scene::Renderable::Renderable> RenderablePtr;
typedef std::shared_ptr<Scene::Lights::Light> LightPtr;

namespace Animation::Effects {

	//creation
	typedef void (*CreateRenderableEffectPtr)(RenderablePtr& renderable, void* constructionData);
	typedef void (*CreateLightEffectPtr)(LightPtr& light, void* constructionData);

	static std::map<std::string, CreateRenderableEffectPtr> CreateRenderableEffects = {
		{ DecalLoopEffect, CreateDecalLoop },
	};
	static std::map<std::string, CreateLightEffectPtr> CreateLightEffects = {
		{ LightOscilationEffect, CreateLightOscilation },
	};

	//step
	typedef void (*StepEffectPtr)(FLOAT delta);

	static std::map<std::string, StepEffectPtr> StepEffects = {
		{ DecalLoopEffect, StepDecalLoop },
		{ LightOscilationEffect, StepLightOscilation },
	};

	void EffectsStep(FLOAT delta);

	//write cbv
	typedef void (*WriteEffectsConstantsBuffersPtr)(UINT backbufferIndex);

	static std::map<std::string, WriteEffectsConstantsBuffersPtr> WriteEffectsConstantsBuffers = {
		{ DecalLoopEffect, WriteDecalLoopConstantsBuffers },
	};

	void WriteEffectsConstantsBuffer(UINT backbufferIndex);

	//destroy
	typedef void (*DestroyEffectPtr)();

	static std::map<std::string, DestroyEffectPtr> DestroyEffects = {
		{ DecalLoopEffect, DestroyDecalLoops },
		{ LightOscilationEffect, DestroyLightOscilations }
	};

	void EffectsDestroy();

#if defined(_EDITOR)
	//save level
	
	typedef void(*GetRenderableEffectsJson)(RenderablePtr& renderable, nlohmann::json& j);
	typedef void(*GetLightEffectsJson)(LightPtr& light, nlohmann::json& j);
	
	static std::map<std::string, GetRenderableEffectsJson> RenderableEffectsJson = {
		{ DecalLoopEffect, DecalLoopJson }
	};

	static std::map<std::string, GetLightEffectsJson> LightEffectsJson = {
		{ LightOscilationEffect, LightOscilationJson }
	};

	nlohmann::json GetRenderableEffects(RenderablePtr& renderable);
	nlohmann::json GetLightEffects(LightPtr& light);

#endif

}