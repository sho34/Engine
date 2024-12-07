#pragma once

using namespace DirectX;
namespace Scene::Lights { struct Light; };
namespace Animation::Effects {

	struct LightOscilation
	{
		XMFLOAT3 originalValue;
		XMFLOAT3* target;
		FLOAT offset = 0.0f;
		FLOAT amplitude = 1.0f;
		FLOAT angularFrequency = 1.0f;
		FLOAT currentTime = 0.0f;
	};

	//float flameLightScale = 0.9f + 0.1f * cosf(static_cast<FLOAT>(timer.GetTotalSeconds()) * 45.0f);

	void CreateLightOscilation(std::shared_ptr<Scene::Lights::Light>, void* constructionData);
	void StepLightOscilation(FLOAT delta);
}

typedef Animation::Effects::LightOscilation LightOscilationT;
typedef std::shared_ptr<LightOscilationT> LightOscilationPtr;

