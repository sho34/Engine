#include "pch.h"
#include "DeviceUtils.h"
#include "Renderer.h"
#include "../Common/DirectXHelper.h"

/*
void InitializeSampler(D3D12_STATIC_SAMPLER_DESC& sampler, UINT shaderRegister) {
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = shaderRegister;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}
*/

/*
void CreateRootSignature(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12RootSignature>& rootSignature, UINT numCBV, UINT numSRV, UINT numSamplers) {
	CD3DX12_DESCRIPTOR_RANGE rangeCBV = {};
	CD3DX12_DESCRIPTOR_RANGE rangeSRV = {};
	CD3DX12_ROOT_PARAMETER parameter[2] = {};

	rangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, numCBV, 0);
	rangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, numSRV, 0);
	parameter[0].InitAsDescriptorTable(1, &rangeCBV, D3D12_SHADER_VISIBILITY_ALL);
	parameter[1].InitAsDescriptorTable(1, &rangeSRV, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
	samplers.resize(numSamplers);
	for (UINT i = 0; i < numSamplers; i++) {
		InitializeSampler(samplers[i], i);
	}

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(_countof(parameter), parameter, numSamplers, &samplers[0], rootSignatureFlags);

	ComPtr<ID3DBlob> pSignature;
	ComPtr<ID3DBlob> pError;
	DX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
	DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(rootSignature.ReleaseAndGetAddressOf())));
	NAME_D3D12_OBJECT(rootSignature);
}*/

/*
void InitializeShadowMapRootSignature(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12RootSignature>& shadowMapRootSignature) {
	CD3DX12_DESCRIPTOR_RANGE rangeCBV = {};
	CD3DX12_ROOT_PARAMETER parameter = {};

	rangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	parameter.InitAsDescriptorTable(1, &rangeCBV, D3D12_SHADER_VISIBILITY_ALL);

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

	ComPtr<ID3DBlob> pSignature;
	ComPtr<ID3DBlob> pError;
	DX::ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()));
	DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(shadowMapRootSignature.ReleaseAndGetAddressOf())));
	NAME_D3D12_OBJECT(shadowMapRootSignature);
}
*/

/*
void InitializePipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& state, const D3D12_INPUT_ELEMENT_DESC inputLayout[], UINT inputLayoutSize, ComPtr<ID3D12RootSignature> rootSignature, ShaderByteCodePtr& vertexShader, ShaderByteCodePtr& pixelShader) {
	state.InputLayout = { inputLayout, inputLayoutSize };
	state.pRootSignature = rootSignature.Get();
	state.VS = CD3DX12_SHADER_BYTECODE(&vertexShader.get()->at(0), vertexShader.get()->size());
	state.PS = CD3DX12_SHADER_BYTECODE(&pixelShader.get()->at(0), pixelShader.get()->size());
	state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	state.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	state.SampleMask = UINT_MAX;
	state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	state.NumRenderTargets = 1;
	state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	state.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	state.SampleDesc.Count = 1;
}
*/

/*
void InitializeShadowMapPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& state, const D3D12_INPUT_ELEMENT_DESC inputLayout[], UINT inputLayoutSize, ComPtr<ID3D12RootSignature> rootSignature, std::shared_ptr<ShaderByteCode>& vertexShader, std::shared_ptr<ShaderByteCode>& pixelShader) {
	InitializePipelineState(state, inputLayout, inputLayoutSize, rootSignature, vertexShader, pixelShader);
	state.NumRenderTargets = 0;
	state.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
}
*/

/*
void CreateShadowMapResourceView(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12Resource> shadowMap, CD3DX12_CPU_DESCRIPTOR_HANDLE shadowMapSrvCpuHandle) {
	//create the shader resource view descriptor using R32_FLOAT format
	//crea el descriptor del shader resource view usando el formato R32_FLOAT
	D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
	shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shadowSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSrvDesc.Texture2D.MipLevels = 1U;
	shadowSrvDesc.Texture2D.MostDetailedMip = 0;
	shadowSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	d3dDevice->CreateShaderResourceView(shadowMap.Get(), &shadowSrvDesc, shadowMapSrvCpuHandle);
}
*/