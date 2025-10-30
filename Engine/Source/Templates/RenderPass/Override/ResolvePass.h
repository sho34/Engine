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

	ResolvePass(JUUID cam, unsigned int rpI, JUUID rp);
	virtual void Pass();
	void Render();
};

