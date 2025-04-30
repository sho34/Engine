#pragma once

#include "../../../d3dx12.h"
#include "../../../Templates/Material/Material.h"

typedef std::tuple<
	std::vector<D3D12_INPUT_ELEMENT_DESC>,
	ShaderByteCode, ShaderByteCode, size_t,
	D3D12_BLEND_DESC, D3D12_RASTERIZER_DESC, D3D12_PRIMITIVE_TOPOLOGY_TYPE,
	std::vector<DXGI_FORMAT>, DXGI_FORMAT
> GraphicsPipelineStateDesc;

typedef std::tuple<
	ShaderByteCode,
	size_t
> ComputePipelineStateDesc;

typedef std::tuple<size_t, CComPtr<ID3D12PipelineState>> HashedPipelineState;

namespace DeviceUtils
{
	using namespace Templates;

	HashedPipelineState CreateGraphicsPipelineState(GraphicsPipelineStateDesc& p);

	CComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(
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

	HashedPipelineState CreateComputePipelineState(ComputePipelineStateDesc& p);

	CComPtr<ID3D12PipelineState> CreateComputePipelineState(
		std::string name,
		ShaderByteCode& csCode,
		CComPtr<ID3D12RootSignature>& rootSignature
	);

#if defined(_EDITOR)
	void ImDrawBlendStatesRenderTargets(nlohmann::json& BlendState);

	void ImDrawGraphicsPipelineState(nlohmann::json& PipelineState);
#endif
};
