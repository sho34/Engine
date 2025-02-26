#include "pch.h"
#include "DecalLoop.h"
#include "../Scene/Renderable/Renderable.h"

namespace Effects {
	using namespace Scene;

	std::map<std::shared_ptr<Renderable>, std::shared_ptr<DecalLoop>> decalLoopEffects;

	void CreateDecalLoop(const std::shared_ptr<Renderable>& renderable, void* constructionData)
	{
		DecalLoop* decaldata = (DecalLoop*)constructionData;
		decalLoopEffects[renderable] = std::make_shared<DecalLoop>(*decaldata);
	}

	void DestroyDecalLoops()
	{
		decalLoopEffects.clear();
	}

	void StepDecalLoop(float delta)
	{
		for (auto& [renderable, decal] : decalLoopEffects) {
			decal->currentTime += delta;
			decal->currentFrame = static_cast<unsigned int>(decal->currentTime / decal->timePerFrames) % decal->numFrames;
		}
	}

	void WriteDecalLoopConstantsBuffers(unsigned int backbufferIndex)
	{
		for (auto& [renderable, decal] : decalLoopEffects) {
			renderable->WriteConstantsBuffer("frameIndex", decal->currentFrame, backbufferIndex);
		}
	}

#if defined(_EDITOR)
	void DecalLoopJson(const std::shared_ptr<Renderable>& renderable, nlohmann::json& j) {
		if (!decalLoopEffects.contains(renderable)) return;

		if (j.empty()) j = nlohmann::json::array();

		auto effect = decalLoopEffects[renderable];
		j.push_back(
			{ DecalLoopEffect,
				{
					{ "numFrames", effect->numFrames },
					{ "timePerFrames", effect->timePerFrames }
				}
			}
		);
	}
#endif

}