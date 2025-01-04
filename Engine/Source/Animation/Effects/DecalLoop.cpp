#include "pch.h"
#include "DecalLoop.h"
#include "../../Scene/Renderable/Renderable.h"

std::map<RenderablePtr, DecalLoopPtr> decalLoopEffects;

namespace Animation::Effects {

	void CreateDecalLoop(RenderablePtr& renderable, void* constructionData)
	{

		DecalLoopT* decaldata = (DecalLoopT*)constructionData;
		decalLoopEffects[renderable] = std::make_shared<DecalLoopT>(*decaldata);

	}

	void DestroyDecalLoops()
	{
		decalLoopEffects.clear();
	}

	void StepDecalLoop(FLOAT delta)
	{
		for (auto& [renderable, decal] : decalLoopEffects) {
			decal->currentTime += delta;
			decal->currentFrame = static_cast<UINT>(decal->currentTime / decal->timePerFrames) % decal->numFrames;
		}
	}

	void WriteDecalLoopConstantsBuffers(UINT backbufferIndex)
	{
		for (auto& [renderable, decal] : decalLoopEffects) {
			renderable->WriteConstantsBuffer("frameIndex", decal->currentFrame, backbufferIndex);
		}
	}

#if defined(_EDITOR)
	void DecalLoopJson(RenderablePtr& renderable, nlohmann::json& j) {
		if (!decalLoopEffects.contains(renderable)) return;
		
		if (j.empty()) j = nlohmann::json::array();

		auto effect = decalLoopEffects[renderable];
		std::string jname;
		std::transform(DecalLoopEffect.begin(), DecalLoopEffect.end(), std::back_inserter(jname), [](wchar_t c) { return (char)c; });
		j.push_back({
			jname, {
				{"numFrames", effect->numFrames },
				{"timePerFrames", effect->timePerFrames }
			}
		});

	}
#endif

}