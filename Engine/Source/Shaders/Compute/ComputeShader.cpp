#include "pch.h"
#include "ComputeShader.h"
#include <Renderer.h>
#include "ComputeInterface.h"
#include <DeviceUtils/RootSignature/RootSignature.h>
#include <DeviceUtils/PipelineState/PipelineState.h>

extern std::shared_ptr<Renderer> renderer;

namespace ComputeShader
{
	void ComputeShader::Init(std::string shaderName, std::vector<MaterialSamplerDesc> samplers, std::wstring target)
	{
		using namespace DeviceUtils;

		//Get an instance of the BoundingBox Compute shader
		std::string csShaderInstanceUUID = FindShaderUUIDByName(shaderName);
		Source compCS = { .shaderType = COMPUTE_SHADER, .shaderTarget = target, .shaderUUID = csShaderInstanceUUID };
		shader = GetShaderInstance(csShaderInstanceUUID, [csShaderInstanceUUID, compCS]
			{
				return std::make_shared<ShaderInstance>(compCS.shaderUUID, compCS.shaderUUID, compCS);
			}
		);

		//Build the shader's root signature
		auto& vsCBparams = shader->constantsBuffersParameters;
		auto& psCBparams = shader->constantsBuffersParameters;
		auto& uavParams = shader->uavParameters;
		auto& psSRVCSparams = shader->srvCSParameters;
		auto& psSRVTexparams = shader->srvTexParameters;
		auto& psSamplersParams = shader->samplersParameters;

		rootSignature = CreateRootSignature(std::string("rootSignature:" + shaderName), vsCBparams, psCBparams, uavParams, psSRVCSparams, psSRVTexparams, psSamplersParams, samplers);
		pipelineState = CreateComputePipelineState(std::string("pipelineState:" + shaderName), shader->byteCode, rootSignature);
	}

	void ComputeShader::SetComputeState()
	{
		CComPtr<ID3D12GraphicsCommandList2>& commandList = renderer->commandList;
		commandList->SetComputeRootSignature(rootSignature);
		commandList->SetPipelineState(pipelineState);
	}
}