#include "pch.h"
#include "LightOscilation.h"
#include <Lights/Lights.h>

namespace Effects {

	std::map<std::shared_ptr<Light>, std::shared_ptr<LightOscilation>> lightOscilationEffects;

	void CreateLightOscilation(const std::shared_ptr<Light>& light, void* constructionData)
	{
		LightOscilation* lightdata = (LightOscilation*)constructionData;
		lightOscilationEffects[light] = std::make_shared<LightOscilation>(*lightdata);
		lightOscilationEffects[light]->originalValue = *lightOscilationEffects[light]->target;
	}

	void DestroyLightOscilations()
	{
		lightOscilationEffects.clear();
	}

	void StepLightOscilation(float delta) {

		for (auto& [light, oscilation] : lightOscilationEffects) {
			oscilation->currentTime += delta;
			float value = oscilation->offset + oscilation->amplitude * XMScalarCos(oscilation->currentTime * oscilation->angularFrequency);
			XMVECTOR originalVector = { oscilation->originalValue.x, oscilation->originalValue.y, oscilation->originalValue.z, 0.0f };
			XMVECTOR newVector = XMVectorScale(originalVector, value);
			*oscilation->target = *(XMFLOAT3*)newVector.m128_f32;
		}
	}

#if defined(_EDITOR)
	void LightOscilationJson(const std::shared_ptr<Light>& light, nlohmann::json& j) {
		if (!lightOscilationEffects.contains(light)) return;

		if (j.empty()) j = nlohmann::json::array();

		auto effect = lightOscilationEffects[light];
		ULONGLONG target = (byte*)effect->target - (byte*)light.get();
		j.push_back(
			{
			LightOscilationEffect,
				{
					{"offset", effect->offset },
					{"amplitude", effect->amplitude },
					{"angularFrequency", effect->angularFrequency },
					{"target", target}
				}
			}
		);
	}
#endif

}