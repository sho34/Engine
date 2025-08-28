#include "pch.h"
#include "ComputeShader.h"
#include <Renderer.h>
#include "ComputeInterface.h"
#include <DeviceUtils/RootSignature/RootSignature.h>
#include <DeviceUtils/PipelineState/PipelineState.h>

extern std::shared_ptr<Renderer> renderer;

namespace ComputeShader
{
	/*
	std::set<std::shared_ptr<ComputeInterface>> computes;

	void RegisterComputation(std::shared_ptr<ComputeInterface> compute)
	{
		computes.insert(compute);
	}

	void UnregisterComputation(std::shared_ptr<ComputeInterface> compute)
	{
		computes.erase(compute);
	}

	std::set<std::shared_ptr<ComputeInterface>>& GetComputeUnits()
	{
		return computes;
	}

	void RunComputeShaders()
	{
		for (auto& compute : GetComputeUnits())
		{
			compute->Compute();
		}
	}

	void ComputeShaderSolution()
	{
		for (auto& compute : GetComputeUnits())
		{
			compute->Solution();
		}
	}
	*/
	void ComputeShader::Init(std::string shaderName, std::vector<MaterialSamplerDesc> samplers)
	{
		using namespace DeviceUtils;

		//Get an instance of the BoundingBox Compute shader
		std::string csShaderInstanceUUID = FindShaderUUIDByName(shaderName);
		Source compCS = { .shaderType = COMPUTE_SHADER, .shaderUUID = csShaderInstanceUUID };
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
		//
		//size_t rootSignatureHash = std::get<0>(rootSignature);
		//ComputePipelineStateDesc pipelineStateDesc = std::tie(shader->byteCode, rootSignatureHash);
		pipelineState = CreateComputePipelineState(std::string("pipelineState:" + shaderName), shader->byteCode, rootSignature);
	}

	void ComputeShader::SetComputeState()
	{
		CComPtr<ID3D12GraphicsCommandList2>& commandList = renderer->commandList;
		//CComPtr<ID3D12RootSignature>& rs = std::get<1>(rootSignature);
		//CComPtr<ID3D12PipelineState>& ps = std::get<1>(pipelineState);
		//
		commandList->SetComputeRootSignature(rootSignature);
		commandList->SetPipelineState(pipelineState);
	}
}