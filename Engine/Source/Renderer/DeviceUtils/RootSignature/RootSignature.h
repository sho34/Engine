#pragma once

#include "../../../d3dx12.h"
#include "../../../Templates/Material/Material.h"

typedef std::tuple<
	ShaderConstantsBufferParametersMap,
	ShaderConstantsBufferParametersMap,
	ShaderUAVParametersMap,
	ShaderSRVCSParametersMap,
	ShaderSRVTexParametersMap,
	ShaderSamplerParametersMap,
	std::vector<MaterialSamplerDesc>
> RootSignatureDesc;

typedef std::tuple<size_t, CComPtr<ID3D12RootSignature>> HashedRootSignature;

namespace DeviceUtils
{
	using namespace Templates;

	HashedRootSignature CreateRootSignature(RootSignatureDesc& rootSignatureDesc);

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

	CComPtr<ID3D12RootSignature> GetRootSignature(size_t rootSignatureHash);

	std::map<INT, CD3DX12_DESCRIPTOR_RANGE> GetRootSignatureRanges(
		ShaderConstantsBufferParametersMap& cbufferVSParamsDef,
		ShaderConstantsBufferParametersMap& cbufferPSParamsDef,
		ShaderUAVParametersMap& uavParamsDef,
		ShaderSRVCSParametersMap& srvCSParamsDef,
		ShaderSRVTexParametersMap& srvTexParamsDef
	);

	std::vector<D3D12_STATIC_SAMPLER_DESC> GetRootSignatureSamplerDesc(ShaderSamplerParametersMap& samplersDef, std::vector<MaterialSamplerDesc>& matSamplers);

};
