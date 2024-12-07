#pragma once

#include <wrl.h>
#include "../../../d3dx12.h"
#include "../../../Templates/Material.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace Templates::Material;

namespace DeviceUtils::RootSignature
{
	ComPtr<ID3D12RootSignature> CreateRootSignature(ComPtr<ID3D12Device2> d3dDevice, const MaterialPtr& material);
	std::map<INT, CD3DX12_DESCRIPTOR_RANGE> GetRootSignatureRanges(const MaterialPtr& material);
	std::vector<D3D12_STATIC_SAMPLER_DESC> GetRootSignatureSamplerDesc(const MaterialPtr& material);
};
