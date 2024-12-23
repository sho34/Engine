#pragma once

#include "../../../d3dx12.h"
#include "../../../Templates/Material/MaterialImpl.h"

namespace DeviceUtils::RootSignature
{
	CComPtr<ID3D12RootSignature> CreateRootSignature(CComPtr<ID3D12Device2> d3dDevice, const MaterialPtr& material);
	std::map<INT, CD3DX12_DESCRIPTOR_RANGE> GetRootSignatureRanges(const MaterialPtr& material);
	std::vector<D3D12_STATIC_SAMPLER_DESC> GetRootSignatureSamplerDesc(const MaterialPtr& material);
};
