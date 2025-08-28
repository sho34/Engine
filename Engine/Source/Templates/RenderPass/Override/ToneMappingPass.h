#pragma once
#include "OverridePass.h"
#include <HDR/LuminanceHistogram.h>
#include <HDR/LuminanceHistogramAverage.h>

using namespace ComputeShader;

struct ToneMappingPass : public OverridePass
{
	std::shared_ptr<LuminanceHistogram> hdrHistogram;
	std::shared_ptr<LuminanceHistogramAverage> luminanceHistogramAverage;

	ToneMappingPass(std::shared_ptr<Camera> cam, unsigned int rpI, std::shared_ptr<RenderPassInstance> rp);
	virtual ~ToneMappingPass();
	virtual void Pass();
	void Render();
};

