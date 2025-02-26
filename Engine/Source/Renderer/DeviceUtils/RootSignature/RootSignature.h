#pragma once

#include "../../../d3dx12.h"
#include "../../../Templates/Material/Material.h"

namespace DeviceUtils
{
	using namespace Templates;
	CComPtr<ID3D12RootSignature> CreateRootSignature(
		std::string name,
		ShaderConstantsBufferParametersMap& cbufferVSParamsDef,
		ShaderConstantsBufferParametersMap& cbufferPSParamsDef,
		ShaderTextureParametersMap& srvPSParamsDef,
		ShaderSamplerParametersMap& samplersDef,
		std::vector<MaterialSamplerDesc>& matSamplers);
	std::map<INT, CD3DX12_DESCRIPTOR_RANGE> GetRootSignatureRanges(
		ShaderConstantsBufferParametersMap& cbufferVSParamsDef,
		ShaderConstantsBufferParametersMap& cbufferPSParamsDef,
		ShaderTextureParametersMap& srvPSParamsDef);
	std::vector<D3D12_STATIC_SAMPLER_DESC> GetRootSignatureSamplerDesc(ShaderSamplerParametersMap& samplersDef, std::vector<MaterialSamplerDesc>& matSamplers);
};
