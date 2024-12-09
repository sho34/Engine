#include "pch.h"
#include "LightOscilation.h"
#include "../../Scene/Lights/Lights.h"

std::map<LightPtr, LightOscilationPtr> lightOscilationEffects;

namespace Animation::Effects {

	void CreateLightOscilation(LightPtr& light, void* constructionData) {

		LightOscilationT* lightdata = (LightOscilationT*)constructionData;
		lightOscilationEffects[light] = std::make_shared<LightOscilationT>(*lightdata);

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

}