#pragma once
#include <string>
#include "../../Renderer/DeviceUtils/RootSignature/RootSignature.h"
#include "../../Renderer/DeviceUtils/PipelineState/PipelineState.h"

namespace Templates { struct ShaderInstance; };

namespace ComputeShader
{
	using namespace Templates;
	struct ComputeInterface;

	struct ComputeShader
	{
		std::shared_ptr<ShaderInstance> shader = nullptr;
		HashedRootSignature rootSignature;
		HashedPipelineState pipelineState;

		~ComputeShader() { DestroyShaderBinary(shader); }
		void Init(std::string shaderName, std::vector<MaterialSamplerDesc> samplers = {});
		void SetComputeState();
	};
}

