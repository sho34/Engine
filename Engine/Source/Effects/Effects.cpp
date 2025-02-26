#include "pch.h"
#include "Effects.h"

namespace Effects {

	void EffectsStep(float delta)
	{
		for (auto& [fx, StepFn] : StepEffects)
		{
			StepFn(delta);
		}
	}

	void WriteEffectsToConstantsBuffer(unsigned int backbufferIndex)
	{
		for (auto& [fx, WriteCbFn] : WriteEffectsConstantsBuffers)
		{
			WriteCbFn(backbufferIndex);
		}
	}

	void EffectsDestroy()
	{
		for (auto& [fx, DestroyCbFn] : DestroyEffects)
		{
			DestroyCbFn();
		}
	}

#if defined(_EDITOR)

	nlohmann::json GetRenderableEffects(const std::shared_ptr<Renderable>& renderable) {
		nlohmann::json j;

		for (auto& [fx, JsonCbFn] : RenderableEffectsJson) {
			JsonCbFn(renderable, j);
		}

		return j;
	}

	nlohmann::json GetLightEffects(const std::shared_ptr<Light>& light)
	{
		nlohmann::json j;

		for (auto& [fx, JsonCbFn] : LightEffectsJson) {
			JsonCbFn(light, j);
		}

		return j;
	}

#endif

}