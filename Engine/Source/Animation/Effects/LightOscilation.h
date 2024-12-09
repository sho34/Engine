#pragma once

using namespace DirectX;
namespace Scene::Lights { struct Light; };
typedef std::shared_ptr<Scene::Lights::Light> LightPtr;
namespace Animation::Effects {

	static const std::wstring LightOscilationEffect = L"LightOscilation";

	struct LightOscilation
	{
		XMFLOAT3 originalValue;
		XMFLOAT3* target;
		FLOAT offset = 0.0f;
		FLOAT amplitude = 1.0f;
		FLOAT angularFrequency = 1.0f;
		FLOAT currentTime = 0.0f;
	};

	void CreateLightOscilation(LightPtr&, void* constructionData);
	void DestroyLightOscilations();
	void StepLightOscilation(FLOAT delta);
}

typedef Animation::Effects::LightOscilation LightOscilationT;
typedef std::shared_ptr<LightOscilationT> LightOscilationPtr;

