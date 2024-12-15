#include "pch.h"
#include "LightOscilation.h"
#include "../../Scene/Lights/Lights.h"

std::map<LightPtr, LightOscilationPtr> lightOscilationEffects;

namespace Animation::Effects {

	void CreateLightOscilation(LightPtr& light, void* constructionData) {

		LightOscilationT* lightdata = (LightOscilationT*)constructionData;
		lightOscilationEffects[light] = std::make_shared<LightOscilationT>(*lightdata);
		lightOscilationEffects[light]->originalValue = *lightOscilationEffects[light]->target;

	}

	void DestroyLightOscilations()
	{
		lightOscilationEffects.clear();
	}

	void StepLightOscilation(FLOAT delta) {

		for (auto& [light, oscilation] : lightOscilationEffects) {
			oscilation->currentTime += delta;
			FLOAT value = oscilation->offset + oscilation->amplitude * XMScalarCos(oscilation->currentTime * oscilation->angularFrequency);
			XMVECTOR originalVector = { oscilation->originalValue.x, oscilation->originalValue.y, oscilation->originalValue.z, 0.0f };
			XMVECTOR newVector = XMVectorScale(originalVector, value);
			*oscilation->target = *(XMFLOAT3*)newVector.m128_f32;
		}
	}

#if defined(_EDITOR)
	void LightOscilationJson(LightPtr& light, nlohmann::json& j) {
		if (!lightOscilationEffects.contains(light)) return;

		if (j.empty()) j = nlohmann::json::array();

		auto effect = lightOscilationEffects[light];
		std::string jname;
		std::transform(LightOscilationEffect.begin(), LightOscilationEffect.end(), std::back_inserter(jname), [](wchar_t c) { return (char)c; });

		ULONGLONG target = (byte*)effect->target - (byte*)light.get();

		j.push_back({
			jname, {
				{"offset", effect->offset },
				{"amplitude", effect->amplitude },
				{"angularFrequency", effect->angularFrequency },
				{"target", target}
			}
		});

	}
#endif

}