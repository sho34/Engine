#pragma once

#include "DecalLoop.h"
#include "LightOscilation.h"

namespace Scene { struct Renderable; };
namespace Scene { struct Light; };

namespace Effects {

	using namespace Scene;

	//creation
	static std::map<std::string, std::function<void(const std::shared_ptr<Renderable>&, void*)>> CreateRenderableEffects = {
		{ DecalLoopEffect, CreateDecalLoop },
	};
	static std::map<std::string, std::function<void(const std::shared_ptr<Light>&, void*)>> CreateLightEffects = {
		{ LightOscilationEffect, CreateLightOscilation },
	};

	//step
	static std::map<std::string, std::function<void(float)>> StepEffects = {
		{ DecalLoopEffect, StepDecalLoop },
		{ LightOscilationEffect, StepLightOscilation },
	};
	void EffectsStep(float delta);

	//write cbv
	static std::map<std::string, std::function<void(unsigned int)>> WriteEffectsConstantsBuffers = {
		{ DecalLoopEffect, WriteDecalLoopConstantsBuffers },
	};
	void WriteEffectsToConstantsBuffer(unsigned int backbufferIndex);

	//destroy
	static std::map<std::string, std::function<void()>> DestroyEffects = {
		{ DecalLoopEffect, DestroyDecalLoops },
		{ LightOscilationEffect, DestroyLightOscilations }
	};
	void EffectsDestroy();

#if defined(_EDITOR)
	//save level

	static std::map<std::string, std::function<void(const std::shared_ptr<Renderable>&, nlohmann::json&)>> RenderableEffectsJson = {
		{ DecalLoopEffect, DecalLoopJson }
	};
	nlohmann::json GetRenderableEffects(const std::shared_ptr<Renderable>& renderable);

	static std::map<std::string, std::function<void(const std::shared_ptr<Light>&, nlohmann::json&)>> LightEffectsJson = {
		{ LightOscilationEffect, LightOscilationJson }
	};
	nlohmann::json GetLightEffects(const std::shared_ptr<Light>& light);

#endif

}