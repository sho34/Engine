#pragma once

#include "../../../d3dx12.h"
#include "../../../Templates/Mesh/Mesh.h"
#include "../../../Templates/Material/Material.h"

namespace DeviceUtils
{
	using namespace Templates;

	struct RenderablePipelineState
	{
		D3D12_BLEND_DESC BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		D3D12_RASTERIZER_DESC RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		std::vector<DXGI_FORMAT> renderTargetsFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };
		DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D32_FLOAT;
	};

	CComPtr<ID3D12PipelineState> CreatePipelineState(
		std::string name,
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout,
		ShaderByteCode& vsCode,
		ShaderByteCode& psCode,
		CComPtr<ID3D12RootSignature>& rootSignature,
		const RenderablePipelineState& renderablePipelineState
	);
};
