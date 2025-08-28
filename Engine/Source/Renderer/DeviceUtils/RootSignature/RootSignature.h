#pragma once

#include <d3dx12.h>
#include <Templates.h>
#include <Material/SamplerDesc.h>
#include <ShaderMaterials.h>
#include <winrt/base.h>
#include <atlbase.h>

namespace DeviceUtils
{
	using namespace Templates;

	CComPtr<ID3D12RootSignature> CreateRootSignature(
		std::string name,
		ShaderConstantsBufferParametersMap& cbufferVSParamsDef,
		ShaderConstantsBufferParametersMap& cbufferPSParamsDef,
		ShaderUAVParametersMap& uavParamsDef,
		ShaderSRVCSParametersMap& srvCSParamsDef,
		ShaderSRVTexParametersMap& srvTexParamsDef,
		ShaderSamplerParametersMap& samplersDef,
		std::vector<MaterialSamplerDesc>& matSamplers
	);

	CComPtr<ID3D12RootSignature> CreateComputeShaderRootSignature(std::string name);

	std::map<INT, CD3DX12_DESCRIPTOR_RANGE> GetRootSignatureRanges(
		ShaderConstantsBufferParametersMap& cbufferVSParamsDef,
		ShaderConstantsBufferParametersMap& cbufferPSParamsDef,
		ShaderUAVParametersMap& uavParamsDef,
		ShaderSRVCSParametersMap& srvCSParamsDef,
		ShaderSRVTexParametersMap& srvTexParamsDef
	);

	std::vector<D3D12_STATIC_SAMPLER_DESC> GetRootSignatureSamplerDesc(ShaderSamplerParametersMap& samplersDef, std::vector<MaterialSamplerDesc>& matSamplers);

};
