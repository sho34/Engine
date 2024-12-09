#pragma once

namespace Scene::Renderable { struct Renderable; };
typedef std::shared_ptr<Scene::Renderable::Renderable> RenderablePtr;
namespace Animation::Effects {

	static const std::wstring DecalLoopEffect = L"DecalLoop";

	struct DecalLoop
	{
		UINT numFrames;
		FLOAT timePerFrames;
		FLOAT currentTime = 0.0f;
		UINT currentFrame = 0;
	};

	void CreateDecalLoop(RenderablePtr&, void* constructionData);
	void DestroyDecalLoops();
	void StepDecalLoop(FLOAT delta);
	void WriteDecalLoopConstantsBuffers(UINT backbufferIndex);
}

typedef Animation::Effects::DecalLoop DecalLoopT;
typedef std::shared_ptr<DecalLoopT> DecalLoopPtr;
