#pragma once

#include "OverridePass.h"
namespace Scene
{
	struct Camera;
};

struct ResolvePass : public OverridePass
{
	enum ResolveMode
	{
		ResolveMode_FullScreenQuad,
		ResolveMode_CopyFromRenderToTexture
	};

	ResolveMode mode;

	ResolvePass(std::shared_ptr<Scene::Camera> cam, unsigned int rpI, std::shared_ptr<RenderPassInstance> rp);
	virtual void Pass();
	void Render();
};

