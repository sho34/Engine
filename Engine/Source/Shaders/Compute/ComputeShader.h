#pragma once
#include <string>
#include <vector>
#include <Material/SamplerDesc.h>
#include <Shader/Shader.h>
#include <Shader/ShaderInstance.h>
//#include <DeviceUtils/RootSignature/RootSignature.h>
//#include <DeviceUtils/PipelineState/PipelineState.h>

using namespace Templates;

namespace ComputeShader
{
	//struct ComputeInterface;

	struct ComputeShader
	{
		std::shared_ptr<ShaderInstance> shader = nullptr;
		CComPtr<ID3D12RootSignature> rootSignature;
		CComPtr<ID3D12PipelineState> pipelineState;
		//HashedRootSignature rootSignature;
		//HashedPipelineState pipelineState;

		~ComputeShader() {
			RemoveShaderInstance(shader->instanceUUID, shader);
			shader = nullptr;
			/*DestroyShaderBinary(shader);*/
		}
		void Init(std::string shaderName, std::vector<MaterialSamplerDesc> samplers = {});
		void SetComputeState();
	};
}

