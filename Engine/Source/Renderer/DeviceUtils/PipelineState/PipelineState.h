#pragma once

#include "../../../d3dx12.h"
//#include "../../../Templates/Mesh/Mesh.h"
#include "../../../Templates/Material/Material.h"
//#include "../RootSignature/RootSignature.h"

/*
struct RenderablePipelineState
{
	D3D12_BLEND_DESC BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	D3D12_RASTERIZER_DESC RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	std::vector<DXGI_FORMAT> renderTargetsFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };
	DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D32_FLOAT;

	bool operator<(const RenderablePipelineState& other) const
	{
		return std::tie(
			BlendState.AlphaToCoverageEnable,
			BlendState.IndependentBlendEnable,
			BlendState.RenderTarget[0].BlendEnable,
			BlendState.RenderTarget[0].LogicOpEnable,
			BlendState.RenderTarget[0].SrcBlend,
			BlendState.RenderTarget[0].DestBlend,
			BlendState.RenderTarget[0].BlendOp,
			BlendState.RenderTarget[0].SrcBlendAlpha,
			BlendState.RenderTarget[0].DestBlendAlpha,
			BlendState.RenderTarget[0].BlendOpAlpha,
			BlendState.RenderTarget[0].LogicOp,
			BlendState.RenderTarget[0].RenderTargetWriteMask,
			BlendState.RenderTarget[1].BlendEnable,
			BlendState.RenderTarget[1].LogicOpEnable,
			BlendState.RenderTarget[1].SrcBlend,
			BlendState.RenderTarget[1].DestBlend,
			BlendState.RenderTarget[1].BlendOp,
			BlendState.RenderTarget[1].SrcBlendAlpha,
			BlendState.RenderTarget[1].DestBlendAlpha,
			BlendState.RenderTarget[1].BlendOpAlpha,
			BlendState.RenderTarget[1].LogicOp,
			BlendState.RenderTarget[1].RenderTargetWriteMask,
			BlendState.RenderTarget[2].BlendEnable,
			BlendState.RenderTarget[2].LogicOpEnable,
			BlendState.RenderTarget[2].SrcBlend,
			BlendState.RenderTarget[2].DestBlend,
			BlendState.RenderTarget[2].BlendOp,
			BlendState.RenderTarget[2].SrcBlendAlpha,
			BlendState.RenderTarget[2].DestBlendAlpha,
			BlendState.RenderTarget[2].BlendOpAlpha,
			BlendState.RenderTarget[2].LogicOp,
			BlendState.RenderTarget[2].RenderTargetWriteMask,
			BlendState.RenderTarget[3].BlendEnable,
			BlendState.RenderTarget[3].LogicOpEnable,
			BlendState.RenderTarget[3].SrcBlend,
			BlendState.RenderTarget[3].DestBlend,
			BlendState.RenderTarget[3].BlendOp,
			BlendState.RenderTarget[3].SrcBlendAlpha,
			BlendState.RenderTarget[3].DestBlendAlpha,
			BlendState.RenderTarget[3].BlendOpAlpha,
			BlendState.RenderTarget[3].LogicOp,
			BlendState.RenderTarget[3].RenderTargetWriteMask,
			BlendState.RenderTarget[4].BlendEnable,
			BlendState.RenderTarget[4].LogicOpEnable,
			BlendState.RenderTarget[4].SrcBlend,
			BlendState.RenderTarget[4].DestBlend,
			BlendState.RenderTarget[4].BlendOp,
			BlendState.RenderTarget[4].SrcBlendAlpha,
			BlendState.RenderTarget[4].DestBlendAlpha,
			BlendState.RenderTarget[4].BlendOpAlpha,
			BlendState.RenderTarget[4].LogicOp,
			BlendState.RenderTarget[4].RenderTargetWriteMask,
			BlendState.RenderTarget[5].BlendEnable,
			BlendState.RenderTarget[5].LogicOpEnable,
			BlendState.RenderTarget[5].SrcBlend,
			BlendState.RenderTarget[5].DestBlend,
			BlendState.RenderTarget[5].BlendOp,
			BlendState.RenderTarget[5].SrcBlendAlpha,
			BlendState.RenderTarget[5].DestBlendAlpha,
			BlendState.RenderTarget[5].BlendOpAlpha,
			BlendState.RenderTarget[5].LogicOp,
			BlendState.RenderTarget[5].RenderTargetWriteMask,
			BlendState.RenderTarget[6].BlendEnable,
			BlendState.RenderTarget[6].LogicOpEnable,
			BlendState.RenderTarget[6].SrcBlend,
			BlendState.RenderTarget[6].DestBlend,
			BlendState.RenderTarget[6].BlendOp,
			BlendState.RenderTarget[6].SrcBlendAlpha,
			BlendState.RenderTarget[6].DestBlendAlpha,
			BlendState.RenderTarget[6].BlendOpAlpha,
			BlendState.RenderTarget[6].LogicOp,
			BlendState.RenderTarget[6].RenderTargetWriteMask,
			BlendState.RenderTarget[7].BlendEnable,
			BlendState.RenderTarget[7].LogicOpEnable,
			BlendState.RenderTarget[7].SrcBlend,
			BlendState.RenderTarget[7].DestBlend,
			BlendState.RenderTarget[7].BlendOp,
			BlendState.RenderTarget[7].SrcBlendAlpha,
			BlendState.RenderTarget[7].DestBlendAlpha,
			BlendState.RenderTarget[7].BlendOpAlpha,
			BlendState.RenderTarget[7].LogicOp,
			BlendState.RenderTarget[7].RenderTargetWriteMask,
			RasterizerState.FillMode,
			RasterizerState.CullMode,
			RasterizerState.FrontCounterClockwise,
			RasterizerState.DepthBias,
			RasterizerState.DepthBiasClamp,
			RasterizerState.SlopeScaledDepthBias,
			RasterizerState.DepthClipEnable,
			RasterizerState.MultisampleEnable,
			RasterizerState.AntialiasedLineEnable,
			RasterizerState.ForcedSampleCount,
			RasterizerState.ConservativeRaster,
			PrimitiveTopologyType,
			renderTargetsFormats,
			depthStencilFormat
		) < std::tie(
			other.BlendState.AlphaToCoverageEnable,
			other.BlendState.IndependentBlendEnable,
			other.BlendState.RenderTarget[0].BlendEnable,
			other.BlendState.RenderTarget[0].LogicOpEnable,
			other.BlendState.RenderTarget[0].SrcBlend,
			other.BlendState.RenderTarget[0].DestBlend,
			other.BlendState.RenderTarget[0].BlendOp,
			other.BlendState.RenderTarget[0].SrcBlendAlpha,
			other.BlendState.RenderTarget[0].DestBlendAlpha,
			other.BlendState.RenderTarget[0].BlendOpAlpha,
			other.BlendState.RenderTarget[0].LogicOp,
			other.BlendState.RenderTarget[0].RenderTargetWriteMask,
			other.BlendState.RenderTarget[1].BlendEnable,
			other.BlendState.RenderTarget[1].LogicOpEnable,
			other.BlendState.RenderTarget[1].SrcBlend,
			other.BlendState.RenderTarget[1].DestBlend,
			other.BlendState.RenderTarget[1].BlendOp,
			other.BlendState.RenderTarget[1].SrcBlendAlpha,
			other.BlendState.RenderTarget[1].DestBlendAlpha,
			other.BlendState.RenderTarget[1].BlendOpAlpha,
			other.BlendState.RenderTarget[1].LogicOp,
			other.BlendState.RenderTarget[1].RenderTargetWriteMask,
			other.BlendState.RenderTarget[2].BlendEnable,
			other.BlendState.RenderTarget[2].LogicOpEnable,
			other.BlendState.RenderTarget[2].SrcBlend,
			other.BlendState.RenderTarget[2].DestBlend,
			other.BlendState.RenderTarget[2].BlendOp,
			other.BlendState.RenderTarget[2].SrcBlendAlpha,
			other.BlendState.RenderTarget[2].DestBlendAlpha,
			other.BlendState.RenderTarget[2].BlendOpAlpha,
			other.BlendState.RenderTarget[2].LogicOp,
			other.BlendState.RenderTarget[2].RenderTargetWriteMask,
			other.BlendState.RenderTarget[3].BlendEnable,
			other.BlendState.RenderTarget[3].LogicOpEnable,
			other.BlendState.RenderTarget[3].SrcBlend,
			other.BlendState.RenderTarget[3].DestBlend,
			other.BlendState.RenderTarget[3].BlendOp,
			other.BlendState.RenderTarget[3].SrcBlendAlpha,
			other.BlendState.RenderTarget[3].DestBlendAlpha,
			other.BlendState.RenderTarget[3].BlendOpAlpha,
			other.BlendState.RenderTarget[3].LogicOp,
			other.BlendState.RenderTarget[3].RenderTargetWriteMask,
			other.BlendState.RenderTarget[4].BlendEnable,
			other.BlendState.RenderTarget[4].LogicOpEnable,
			other.BlendState.RenderTarget[4].SrcBlend,
			other.BlendState.RenderTarget[4].DestBlend,
			other.BlendState.RenderTarget[4].BlendOp,
			other.BlendState.RenderTarget[4].SrcBlendAlpha,
			other.BlendState.RenderTarget[4].DestBlendAlpha,
			other.BlendState.RenderTarget[4].BlendOpAlpha,
			other.BlendState.RenderTarget[4].LogicOp,
			other.BlendState.RenderTarget[4].RenderTargetWriteMask,
			other.BlendState.RenderTarget[5].BlendEnable,
			other.BlendState.RenderTarget[5].LogicOpEnable,
			other.BlendState.RenderTarget[5].SrcBlend,
			other.BlendState.RenderTarget[5].DestBlend,
			other.BlendState.RenderTarget[5].BlendOp,
			other.BlendState.RenderTarget[5].SrcBlendAlpha,
			other.BlendState.RenderTarget[5].DestBlendAlpha,
			other.BlendState.RenderTarget[5].BlendOpAlpha,
			other.BlendState.RenderTarget[5].LogicOp,
			other.BlendState.RenderTarget[5].RenderTargetWriteMask,
			other.BlendState.RenderTarget[6].BlendEnable,
			other.BlendState.RenderTarget[6].LogicOpEnable,
			other.BlendState.RenderTarget[6].SrcBlend,
			other.BlendState.RenderTarget[6].DestBlend,
			other.BlendState.RenderTarget[6].BlendOp,
			other.BlendState.RenderTarget[6].SrcBlendAlpha,
			other.BlendState.RenderTarget[6].DestBlendAlpha,
			other.BlendState.RenderTarget[6].BlendOpAlpha,
			other.BlendState.RenderTarget[6].LogicOp,
			other.BlendState.RenderTarget[6].RenderTargetWriteMask,
			other.BlendState.RenderTarget[7].BlendEnable,
			other.BlendState.RenderTarget[7].LogicOpEnable,
			other.BlendState.RenderTarget[7].SrcBlend,
			other.BlendState.RenderTarget[7].DestBlend,
			other.BlendState.RenderTarget[7].BlendOp,
			other.BlendState.RenderTarget[7].SrcBlendAlpha,
			other.BlendState.RenderTarget[7].DestBlendAlpha,
			other.BlendState.RenderTarget[7].BlendOpAlpha,
			other.BlendState.RenderTarget[7].LogicOp,
			other.BlendState.RenderTarget[7].RenderTargetWriteMask,
			other.RasterizerState.FillMode,
			other.RasterizerState.CullMode,
			other.RasterizerState.FrontCounterClockwise,
			other.RasterizerState.DepthBias,
			other.RasterizerState.DepthBiasClamp,
			other.RasterizerState.SlopeScaledDepthBias,
			other.RasterizerState.DepthClipEnable,
			other.RasterizerState.MultisampleEnable,
			other.RasterizerState.AntialiasedLineEnable,
			other.RasterizerState.ForcedSampleCount,
			other.RasterizerState.ConservativeRaster,
			other.PrimitiveTopologyType,
			other.renderTargetsFormats,
			other.depthStencilFormat
		);
	}
};
*/

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

	void ImDrawPipelineRenderTargetFormats(nlohmann::json& PipelineState);

	void ImDrawPipelineState(nlohmann::json& PipelineState);
#endif
};
