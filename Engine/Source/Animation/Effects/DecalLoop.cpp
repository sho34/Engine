#include "pch.h"
#include "DecalLoop.h"
#include "../../Renderer/Renderable.h"

std::map<RenderablePtr, DecalLoopPtr> decalLoopEffects;

namespace Animation::Effects {

	void CreateDecalLoop(std::shared_ptr<Renderable::Renderable> renderable, void* constructionData)
	{

		DecalLoopT* decaldata = (DecalLoopT*)constructionData;
		decalLoopEffects[renderable] = std::make_shared<DecalLoopT>(*decaldata);

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
			renderable->WriteConstantsBuffer(L"frameIndex", decal->currentFrame, backbufferIndex);
		}
	}

}