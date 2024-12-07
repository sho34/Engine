#pragma once

namespace Renderable { struct Renderable; };
namespace Animation::Effects {

	struct DecalLoop
	{
		UINT numFrames;
		FLOAT timePerFrames;
		FLOAT currentTime = 0.0f;
		UINT currentFrame = 0;
	};

	void CreateDecalLoop(std::shared_ptr<Renderable::Renderable>, void* constructionData);
	void StepDecalLoop(FLOAT delta);
	void WriteDecalLoopConstantsBuffers(UINT backbufferIndex);
}

typedef Animation::Effects::DecalLoop DecalLoopT;
typedef std::shared_ptr<DecalLoopT> DecalLoopPtr;
