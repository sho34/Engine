#pragma once

using namespace DirectX;
namespace Scene { struct Light; };
namespace Effects {

	using namespace Scene;

	inline static const std::string LightOscilationEffect = "LightOscilation";

	struct LightOscilation
	{
		XMFLOAT3 originalValue;
		XMFLOAT3* target;
		float offset = 0.0f;
		float amplitude = 1.0f;
		float angularFrequency = 1.0f;
		float currentTime = 0.0f;
	};

	void CreateLightOscilation(const std::shared_ptr<Light>& light, void* constructionData);
	void DestroyLightOscilations();
	void StepLightOscilation(float delta);
#if defined(_EDITOR)
	void LightOscilationJson(const std::shared_ptr<Light>& light, nlohmann::json& j);
#endif
}

