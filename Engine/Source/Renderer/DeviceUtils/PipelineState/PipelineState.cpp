#include "pch.h"
#include "PipelineState.h"
#include "../../../Common/DirectXHelper.h"
#include "../../VertexFormats.h"

namespace DeviceUtils::PipelineState
{

	ComPtr<ID3D12PipelineState> CreatePipelineState(ComPtr<ID3D12Device2> d3dDevice, std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout, const MaterialPtr& material, ComPtr<ID3D12RootSignature>& rootSignature) {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};

		//input layout
		state.InputLayout = { inputLayout.data(), static_cast<UINT>(inputLayout.size()) };

		//root signature
		state.pRootSignature = rootSignature.Get();

		//shader based state
		state.VS = CD3DX12_SHADER_BYTECODE(&material->shader->vertexShader->byteCode->at(0), material->shader->vertexShader->byteCode->size());
		state.PS = CD3DX12_SHADER_BYTECODE(&material->shader->pixelShader->byteCode->at(0), material->shader->pixelShader->byteCode->size());

		//material based state
		state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		state.RasterizerState.CullMode = material->materialDefinition.twoSided ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_FRONT;
		state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); //should be material based
		state.SampleDesc.Count = 1;

		//render target based
		state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		state.SampleMask = UINT_MAX;
		state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		state.NumRenderTargets = 1;
		state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		state.DSVFormat = DXGI_FORMAT_D32_FLOAT;

		ComPtr<ID3D12PipelineState> pipelineState;
		DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf())));
		return pipelineState;
	}

	ComPtr<ID3D12PipelineState> CreatePipelineState(ComPtr<ID3D12Device2> d3dDevice, VertexClass vertexClass, const MaterialPtr& material, ComPtr<ID3D12RootSignature>& rootSignature) {
		return CreatePipelineState(d3dDevice, vertexInputLayoutsMap[vertexClass], material, rootSignature);
	}

	ComPtr<ID3D12PipelineState> CreateShadowMapPipelineState(ComPtr<ID3D12Device2> d3dDevice, VertexClass vertexClass, const MaterialPtr& material, ComPtr<ID3D12RootSignature>& rootSignature) {
		return CreatePipelineState(d3dDevice, shadowMapVertexInputLayoutsMap[vertexClass], material, rootSignature);
	}

};
