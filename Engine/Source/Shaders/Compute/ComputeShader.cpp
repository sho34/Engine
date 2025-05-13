#include "pch.h"
#include "ComputeShader.h"
#include "../../Renderer/Renderer.h"

extern std::shared_ptr<Renderer> renderer;

namespace ComputeShader
{
	void ComputeShader::Init(std::string shaderName)
	{
		using namespace DeviceUtils;

		//Get an instance of the BoundingBox Compute shader
		shader = GetShaderInstance({ .shaderType = COMPUTE_SHADER, .shaderUUID = FindShaderUUIDByName(shaderName) });

		//Build the shader's root signature
		auto& vsCBparams = shader->constantsBuffersParameters;
		auto& psCBparams = shader->constantsBuffersParameters;
		auto& uavParams = shader->uavParameters;
		auto& psSRVCSparams = shader->srvCSParameters;
		auto& psSRVTexparams = shader->srvTexParameters;
		auto& psSamplersParams = shader->samplersParameters;
		std::vector<MaterialSamplerDesc> samplers;

		RootSignatureDesc rootSignatureDesc = std::tie(vsCBparams, psCBparams, uavParams, psSRVCSparams, psSRVTexparams, psSamplersParams, samplers);
		rootSignature = CreateRootSignature(rootSignatureDesc);

		size_t rootSignatureHash = std::get<0>(rootSignature);
		ComputePipelineStateDesc pipelineStateDesc = std::tie(shader->byteCode, rootSignatureHash);
		pipelineState = CreateComputePipelineState(pipelineStateDesc);
	}

	void ComputeShader::SetComputeState()
	{
		CComPtr<ID3D12GraphicsCommandList2>& commandList = renderer->commandList;
		CComPtr<ID3D12RootSignature>& rs = std::get<1>(rootSignature);
		CComPtr<ID3D12PipelineState>& ps = std::get<1>(pipelineState);

		commandList->SetComputeRootSignature(rs);
		commandList->SetPipelineState(ps);
	}
}