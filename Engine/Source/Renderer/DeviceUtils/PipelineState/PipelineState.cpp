#include "pch.h"
#include "PipelineState.h"
#include "../../Renderer.h"
#include "../../VertexFormats.h"
#include "../../../Common/DirectXHelper.h"
#include <ios>

extern std::shared_ptr<Renderer> renderer;
namespace DeviceUtils
{
	CComPtr<ID3D12PipelineState> CreatePipelineState(
		std::string name,
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout,
		ShaderByteCode& vsCode,
		ShaderByteCode& psCode,
		CComPtr<ID3D12RootSignature>& rootSignature,
		const RenderablePipelineState& renderablePipelineState
	)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};

		//input layout
		state.InputLayout = { inputLayout.data(), static_cast<UINT>(inputLayout.size()) };

		//root signature
		state.pRootSignature = rootSignature;

		//shader based state
		state.VS = CD3DX12_SHADER_BYTECODE(vsCode.data(), vsCode.size());
		state.PS = CD3DX12_SHADER_BYTECODE(psCode.data(), psCode.size());

		//material based state
		state.RasterizerState = renderablePipelineState.RasterizerState;
		state.BlendState = renderablePipelineState.BlendState;
		state.SampleDesc.Count = 1;

		//render target based
		state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		if (renderablePipelineState.depthStencilFormat == DXGI_FORMAT_UNKNOWN) { state.DepthStencilState.DepthEnable = false; }
		state.SampleMask = UINT_MAX;
		state.PrimitiveTopologyType = renderablePipelineState.PrimitiveTopologyType;
		state.NumRenderTargets = static_cast<UINT>(max(1, renderablePipelineState.renderTargetsFormats.size()));
		state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		for (unsigned int i = 0; i < renderablePipelineState.renderTargetsFormats.size(); i++)
		{
			state.RTVFormats[i] = renderablePipelineState.renderTargetsFormats[i];
		}
		state.DSVFormat = renderablePipelineState.depthStencilFormat;

		CComPtr<ID3D12PipelineState> pipelineState;
		DX::ThrowIfFailed(renderer->d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&pipelineState)));
		CCNAME_D3D12_OBJECT(pipelineState);

		LogCComPtrAddress(name, pipelineState);

		return pipelineState;
	}
};
