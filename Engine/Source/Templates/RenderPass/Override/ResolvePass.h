#pragma once

#include "OverridePass.h"

struct ResolvePass : public OverridePass
{
	enum ResolveMode
	{
		ResolveMode_FullScreenQuad,
		ResolveMode_CopyFromRenderToTexture
	};

	ResolveMode mode;

	ResolvePass(std::shared_ptr<Camera> cam, unsigned int rpI, std::shared_ptr<RenderPassInstance> rp);
	virtual void Pass();
	void Render();
};

