#pragma once
#include "OverridePass.h"
//#include <HDR/LuminanceHistogram.h>
//#include <HDR/LuminanceHistogramAverage.h>

namespace ComputeShader
{
	struct LuminanceHistogram;
	struct LuminanceHistogramAverage;
};
namespace Scene
{
	struct Camera;
};

struct ToneMappingPass : public OverridePass
{
	std::shared_ptr<ComputeShader::LuminanceHistogram> hdrHistogram;
	std::shared_ptr<ComputeShader::LuminanceHistogramAverage> luminanceHistogramAverage;

	ToneMappingPass(std::shared_ptr<Scene::Camera> cam, unsigned int rpI, std::shared_ptr<RenderPassInstance> rp);
	virtual ~ToneMappingPass();
	virtual void Pass();
	void Render();
};

