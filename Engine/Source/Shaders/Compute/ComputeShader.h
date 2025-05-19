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
		void Init(std::string shaderName);
		void SetComputeState();
	};

	//void RegisterComputation(std::shared_ptr<ComputeInterface> compute);
	//void UnregisterComputation(std::shared_ptr<ComputeInterface> compute);
	//std::set<std::shared_ptr<ComputeInterface>>& GetComputeUnits();
	//void RunComputeShaders();
	//void ComputeShaderSolution();
}

