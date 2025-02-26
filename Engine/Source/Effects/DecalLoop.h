#pragma once

namespace Scene { struct Renderable; };
namespace Effects {

	inline static const std::string DecalLoopEffect = "DecalLoop";

	struct DecalLoop
	{
		unsigned int numFrames;
		float timePerFrames;
		float currentTime = 0.0f;
		unsigned int currentFrame = 0;
	};

	void CreateDecalLoop(const std::shared_ptr<Scene::Renderable>& renderable, void* constructionData);
	void DestroyDecalLoops();
	void StepDecalLoop(float delta);
	void WriteDecalLoopConstantsBuffers(unsigned int backbufferIndex);
#if defined(_EDITOR)
	void DecalLoopJson(const std::shared_ptr<Scene::Renderable>& renderable, nlohmann::json& j);
#endif
}
