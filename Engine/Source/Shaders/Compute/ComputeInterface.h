#pragma once
#include "ComputeShader.h"

namespace Scene { struct Renderable; };

namespace ComputeShader
{
	using namespace DeviceUtils;

	struct ComputeInterface
	{
		ComputeShader shader;

		ComputeInterface(std::string shaderName, std::vector<MaterialSamplerDesc> samplers = {})
		{
			shader.Init(shaderName, samplers);
		}

		virtual void Compute() = 0;
		virtual void Solution() = 0;
	};
};