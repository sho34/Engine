#pragma once

#include "../../../d3dx12.h"
#include "../../../Templates/Material/Material.h"

typedef std::tuple<
	std::vector<D3D12_INPUT_ELEMENT_DESC>,
	ShaderByteCode, ShaderByteCode, size_t,
	D3D12_BLEND_DESC, D3D12_RASTERIZER_DESC, D3D12_PRIMITIVE_TOPOLOGY_TYPE,
	std::vector<DXGI_FORMAT>, DXGI_FORMAT
> PipelineStateDesc;

typedef std::tuple<size_t, CComPtr<ID3D12PipelineState>> HashedPipelineState;

namespace DeviceUtils
{
	using namespace Templates;

	HashedPipelineState CreatePipelineState(PipelineStateDesc& p);

	CComPtr<ID3D12PipelineState> CreatePipelineState(
		std::string name,
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout,
		ShaderByteCode& vsCode,
		ShaderByteCode& psCode,
		CComPtr<ID3D12RootSignature>& rootSignature,
		D3D12_BLEND_DESC& BlendState,
		D3D12_RASTERIZER_DESC& RasterizerState,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE& PrimitiveTopologyType,
		std::vector<DXGI_FORMAT>& renderTargetsFormats,
		DXGI_FORMAT& depthStencilFormat
	);

#if defined(_EDITOR)
	void ImDrawBlendStatesRenderTargets(nlohmann::json& BlendState);

	void ImDrawPipelineState(nlohmann::json& PipelineState);
#endif
};
