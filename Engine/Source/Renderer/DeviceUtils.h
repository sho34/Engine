#pragma once
//#include "../Common/DirectXHelper.h"
//#include "VertexFormats.h"
//#include "Renderer.h"
//#include "../Shaders/Compiler/ShaderCompiler.h"

//using namespace Microsoft::WRL;
//using namespace DirectX;
//using namespace ShaderCompiler;

//typedef std::vector<byte> ShaderByteCode;
//typedef std::shared_ptr<ShaderByteCode> ShaderByteCodePtr;

#include "DeviceUtils/D3D12Device/Builder.h"
#include "DeviceUtils/D3D12Device/Interop.h"
#include "DeviceUtils/RenderTarget/RenderTarget.h"
#include "DeviceUtils/VertexBuffer/VertexBuffer.h"
#include "DeviceUtils/IndexBuffer/IndexBuffer.h"
#include "DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include "DeviceUtils/RootSignature/RootSignature.h"
#include "DeviceUtils/PipelineState/PipelineState.h"

//samplers
/*
void InitializeSampler(D3D12_STATIC_SAMPLER_DESC& sampler, UINT shaderRegister);
*/


//shadow map
/*
void CreateShadowMapResourceView(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12Resource> shadowMap, CD3DX12_CPU_DESCRIPTOR_HANDLE shadowMapSrvCpuHandle);
template<typename T>
CD3DX12_CPU_DESCRIPTOR_HANDLE CreateTexturesResourceViews( UINT numFrames, ComPtr<ID3D12Device2> d3dDevice, ConstantsBufferViewData<T>& cbvMeshData, bool hasNormalMap, ComPtr<ID3D12Resource>& diffuseTexture, D3D12_SHADER_RESOURCE_VIEW_DESC& diffuseSrvDesc, ComPtr<ID3D12Resource>& normalMapTexture, D3D12_SHADER_RESOURCE_VIEW_DESC& normalMapSrvDesc) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = InitializeConstantsBufferView(numFrames, 5U, d3dDevice, cbvMeshData);
	CD3DX12_CPU_DESCRIPTOR_HANDLE diffuseCpuHandle(cbvCpuHandle);
	d3dDevice->CreateShaderResourceView(diffuseTexture.Get(), &diffuseSrvDesc, diffuseCpuHandle);
	if (hasNormalMap) {
		CD3DX12_CPU_DESCRIPTOR_HANDLE normalMapCpuHandle(cbvCpuHandle);
		normalMapCpuHandle.Offset(cbvMeshData.cbvDescriptorSize);
		d3dDevice->CreateShaderResourceView(normalMapTexture.Get(), &normalMapSrvDesc, normalMapCpuHandle);
	}
	return cbvCpuHandle;
}
*/

//pipe line state
/*
typedef void InitPipelineStateFn(D3D12_GRAPHICS_PIPELINE_STATE_DESC& state, const D3D12_INPUT_ELEMENT_DESC inputLayout[], UINT inputLayoutSize, ComPtr<ID3D12RootSignature> rootSignature, std::shared_ptr<ShaderByteCode>& vertexShader, std::shared_ptr<ShaderByteCode>& pixelShader);
InitPipelineStateFn InitializePipelineState;
InitPipelineStateFn InitializeShadowMapPipelineState;

typedef void CreateGraphicsPipelineStateFn(
	ComPtr<ID3D12Device2> d3dDevice,
	ComPtr<ID3D12RootSignature>& rootSignature,
	ComPtr<ID3D12PipelineState>& pipelineState,
	InitPipelineStateFn* initPipelineState,
	ShaderByteCodePtr& vertexShader,
	ShaderByteCodePtr& pixelShader
);
*/

/*
template<typename T>
void CreateGraphicsPipelineState(
	ComPtr<ID3D12Device2> d3dDevice,
	ComPtr<ID3D12RootSignature>& rootSignature,
	ComPtr<ID3D12PipelineState>& pipelineState,
	InitPipelineStateFn* initPipelineState,
	ShaderByteCodePtr& vertexShader,
	ShaderByteCodePtr& pixelShader
) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
	initPipelineState(state, VertexInputLayout<T>::inputLayout, _countof(VertexInputLayout<T>::inputLayout), rootSignature.Get(), vertexShader, pixelShader);
	pipelineState.ReleaseAndGetAddressOf();
	DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf())));
}
*/

/*
namespace DeviceUtils {
	static const std::map<VertexClass, CreateGraphicsPipelineStateFn*> pipelineCreators = {
		{ VertexClass::POS, CreateGraphicsPipelineState<Vertex<VertexClass::POS>> },
		{ VertexClass::POS_COLOR, CreateGraphicsPipelineState<Vertex<VertexClass::POS_COLOR>> },
		{ VertexClass::POS_TEXCOORD0, CreateGraphicsPipelineState<Vertex<VertexClass::POS_TEXCOORD0>> },
		{ VertexClass::POS_NORMAL, CreateGraphicsPipelineState<Vertex<VertexClass::POS_NORMAL>> },
		{ VertexClass::POS_NORMAL_TEXCOORD0, CreateGraphicsPipelineState<Vertex<VertexClass::POS_NORMAL_TEXCOORD0>> },
		{ VertexClass::POS_NORMAL_TANGENT_TEXCOORD0, CreateGraphicsPipelineState<Vertex<VertexClass::POS_NORMAL_TANGENT_TEXCOORD0>> },
		{ VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0, CreateGraphicsPipelineState<Vertex<VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0>> },
		{ VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0_SKINNING, CreateGraphicsPipelineState<Vertex<VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0_SKINNING>> },
	};
}
*/

/*
template<typename T>
void CreateGraphicsPipelineState(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12RootSignature>& rootSignature, ComPtr<ID3D12PipelineState>& pipelineState, InitPipelineState* initPipelineState, std::shared_ptr<ShaderCompiler::ShaderByteCode>& vertexShader, std::shared_ptr<ShaderCompiler::ShaderByteCode>& pixelShader) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
	initPipelineState(state, T::inputLayout, _countof(T::inputLayout), rootSignature.Get(), vertexShader, pixelShader);
	pipelineState.ReleaseAndGetAddressOf();
	DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf())));
}
*/

/*
template<typename T>
Concurrency::task<void> LoadPipeline(void* obj, ComPtr<ID3D12Device2> d3dDevice) {
	return task<void>([obj, d3dDevice]() {
		T* self = (T*)obj;
		if (self->vertexShader!=nullptr && self->vertexShader->size() > 0 && self->pixelShader!=nullptr && self->pixelShader->size() > 0) {
			std::lock_guard<std::mutex> lock(self->shaderMutex);
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(200ms);
			auto createPipeline = DeviceUtils::pipelineCreators.at(self->vertexClass);
			(*createPipeline)(d3dDevice, self->rootSignature, self->pipelineState, InitializePipelineState, self->vertexShader, self->pixelShader);
			//*DeviceUtils::pipelineCreators[self->vertexClass](d3dDevice, self->rootSignature, self->pipelineState, InitializePipelineState, self->vertexShader, self->pixelShader);
			//CreateGraphicsPipelineState<VertexInputLayout<T::VertexType>>(d3dDevice, self->rootSignature, self->pipelineState, InitializePipelineState, self->vertexShader, self->pixelShader);
		}
	});
}
*/

/*
template<typename T>
Concurrency::task<void> LoadShadowMapPipeline(void* obj, ComPtr<ID3D12Device2> d3dDevice) {
	return task<void>([obj, d3dDevice]() {
		T* self = (T*)obj;
		if (self->shadowMapVertexShader != nullptr &&  self->shadowMapVertexShader->size() > 0 &&
			self->shadowMapPixelShader != nullptr && self->shadowMapPixelShader->size() > 0)
		{
			std::lock_guard<std::mutex> lock(self->shaderMutex);
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(200ms);
			CreateGraphicsPipelineState<VertexInputLayoutShadowMap<T::VertexType>>(d3dDevice, self->shadowMapRootSignature, self->shadowMapPipelineState, InitializeShadowMapPipelineState, self->shadowMapVertexShader, self->shadowMapPixelShader);
		}
	});
}
*/

/*
template<typename T, typename V = T::VertexType>
Concurrency::task<void> Load3DModelPipeline(void* obj, ComPtr<ID3D12Device2> d3dDevice) {
	return task<void>([obj, d3dDevice]() {
		T* self = (T*)obj;
		if (self->vertexShader != nullptr && self->vertexShader->size() > 0 &&
			self->pixelShader != nullptr && self->pixelShader->size() > 0)
		{
			std::lock_guard<std::mutex> lock(self->shaderMutex);
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(200ms);
			
			//whatis(VertexInputLayout<T::VertexType>::inputLayout);

			//create the pipelineState for the opaque meshes
			//crea el pipelineState para los meshs opacos
			D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueState = {};
			InitializePipelineState(opaqueState, VertexInputLayout<V>::inputLayout, _countof(VertexInputLayout<V>::inputLayout), self->rootSignature, self->vertexShader, self->pixelShader);
			DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&opaqueState, IID_PPV_ARGS(&self->pipelineState)));
			//create the pipeline for the opaque two sided meshes
			//crear el pipeline para las mallas opacas de dos lados
			D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueTwoSidedState = D3D12_GRAPHICS_PIPELINE_STATE_DESC(opaqueState);
			opaqueTwoSidedState.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&opaqueTwoSidedState, IID_PPV_ARGS(&self->twoSidedPipelineState)));
		}
	});
}
*/

/*
template<typename T, typename V = T::VertexType>
Concurrency::task<void> Load3DModelShadowMapPipeline(void* obj, ComPtr<ID3D12Device2> d3dDevice) {
	return task<void>([obj, d3dDevice]() {
		T* self = (T*)obj;
		if (self->shadowMapVertexShader != nullptr && self->shadowMapVertexShader->size() > 0 &&
			self->shadowMapPixelShader != nullptr && self->shadowMapPixelShader->size() > 0 &&
			self->shadowMapAlphaCutVertexShader != nullptr && self->shadowMapAlphaCutVertexShader->size() > 0 &&
			self->shadowMapAlphaCutPixelShader != nullptr && self->shadowMapAlphaCutPixelShader->size())
		{
			std::lock_guard<std::mutex> lock(self->shaderMutex);
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(200ms);

			//initialize the pipeline state for the shadow map
			//inicializa el pipeline state para el shadow map
			D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowMapOpaqueState = {};
			InitializeShadowMapPipelineState(shadowMapOpaqueState, VertexInputLayoutShadowMap<V>::inputLayout, _countof(VertexInputLayoutShadowMap<V>::inputLayout), self->shadowMapRootSignature.Get(), self->shadowMapVertexShader, self->shadowMapPixelShader);
			DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&shadowMapOpaqueState, IID_PPV_ARGS(&self->shadowMapPipelineState)));

			//initialize the pipeline state for the shadow map with two sides
			//inicializa el pipeline state para el shadow map con dos lados
			D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowMapOpaqueTwoSidedState = D3D12_GRAPHICS_PIPELINE_STATE_DESC(shadowMapOpaqueState);
			shadowMapOpaqueTwoSidedState.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&shadowMapOpaqueTwoSidedState, IID_PPV_ARGS(&self->shadowMapTwoSidedPipelineState)));

			//initialize the pipeline state for the shadow map with alpha testing(cut)
			//inicializa el pipeline state para el shadow map con alpha testing(corte)
			D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowMapAlphaCutState = {};
			InitializeShadowMapPipelineState(shadowMapAlphaCutState, VertexInputLayoutShadowMapAlphaCut<V>::inputLayout, _countof(VertexInputLayoutShadowMapAlphaCut<V>::inputLayout), self->shadowMapAlphaCutRootSignature.Get(), self->shadowMapAlphaCutVertexShader, self->shadowMapAlphaCutPixelShader);
			DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&shadowMapAlphaCutState, IID_PPV_ARGS(&self->shadowMapAlphaCutPipelineState)));

			//initialize the pipeline state for the shadow map with alpha testing(cut) and two sides
			//inicializa el pipeline state para el shadow map con alpha testing(corte) y dos lados
			D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowMapAlphaCutTwoSidedState = D3D12_GRAPHICS_PIPELINE_STATE_DESC(shadowMapAlphaCutState);
			shadowMapAlphaCutTwoSidedState.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&shadowMapAlphaCutTwoSidedState, IID_PPV_ARGS(&self->shadowMapAlphaCutTwoSidedPipelineState)));
		}
	});
}
*/

