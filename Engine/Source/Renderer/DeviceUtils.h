#pragma once
#include "../framework.h"
#include "../Common/DirectXHelper.h"
#include "VertexFormats.h"

#include "../Shaders/Compiler/ShaderCompiler.h"

using namespace Microsoft::WRL;
/*using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Platform;
*/
using namespace DirectX;

ComPtr<IDXGIAdapter4> GetAdapter();
ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter);
ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device2> device);
ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hwnd, ComPtr<ID3D12CommandQueue> commandQueue, UINT bufferCount);
ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device2> device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap, ComPtr<ID3D12Resource> renderTargets[], UINT bufferCount);
void UpdateDepthStencilView(ComPtr<ID3D12Device2> device, ComPtr<ID3D12DescriptorHeap> descriptorHeap, ComPtr<ID3D12Resource>& depthStencil, UINT width, UINT height);
ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<ID3D12Device2> device);
ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandAllocator);
ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> device);
HANDLE CreateEventHandle();
UINT64 Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, UINT64& fenceValue);
void WaitForFenceValue(ComPtr<ID3D12Fence> fence, UINT64 fenceValue, HANDLE fenceEvent);
void Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, UINT64& fenceValue, HANDLE fenceEvent);
void UpdateBufferResource(ComPtr<ID3D12Device2> device, ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>& pDestinationResource, ComPtr<ID3D12Resource>& pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData);
void CreateTextureResource(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, const LPWSTR path, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& textureUpload, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
void CreateTextureArrayResource(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, const LPWSTR path, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& textureUpload, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc, UINT numFrames, DXGI_FORMAT textureFormat = DXGI_FORMAT_BC7_UNORM_SRGB);

//D3D11On12
void CreateD3D11On12Device(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D11Device>& d3d11Device, ComPtr<ID3D11On12Device>& d3d11on12Device, ComPtr<ID3D11DeviceContext>& d3d11DeviceContext, ComPtr<IDXGIDevice>& dxgiDevice);
void CreateD2D1Device(ComPtr<IDXGIDevice> dxgiDevice, ComPtr<ID2D1Factory6>& d2d1Factory, ComPtr<ID2D1Device5>& d2d1Device, ComPtr<ID2D1DeviceContext5>& d2d1DeviceContext);
template<size_t bufferCount>
void UpdateD2D1RenderTargets(ComPtr<ID3D11On12Device>	d3d11on12Device, ComPtr<ID2D1DeviceContext5> d2d1DeviceContext, ComPtr<ID3D12Resource> renderTargets[], ComPtr<ID3D11Resource>(&wrappedBackBuffers)[bufferCount], ComPtr<ID2D1Bitmap1>(&d2dRenderTargets)[bufferCount]) {
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), 144.0f, 144.0f
	);

	for (UINT i = 0; i < bufferCount; i++) {
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		DX::ThrowIfFailed(d3d11on12Device->CreateWrappedResource(
			renderTargets[i].Get(), &d3d11Flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(&wrappedBackBuffers[i])
		));

		ComPtr<IDXGISurface> surface;
		DX::ThrowIfFailed(wrappedBackBuffers[i].As(&surface));
		DX::ThrowIfFailed(d2d1DeviceContext->CreateBitmapFromDxgiSurface(surface.Get(), &bitmapProperties, &d2dRenderTargets[i]));
	}
}

//vertex buffer
template<typename T> struct VertexBufferViewData {
	ComPtr<ID3D12Resource>    vertexBuffer;
	ComPtr<ID3D12Resource>    vertexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW  vertexBufferView;
};

template<typename T>
void InitializeVertexBufferView(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, const void* vertices, UINT verticesCount, VertexBufferViewData<T>& vbvData) {
	UpdateBufferResource(d3dDevice, commandList, vbvData.vertexBuffer, vbvData.vertexBufferUpload, verticesCount, sizeof(T), vertices);
	vbvData.vertexBufferView.BufferLocation = vbvData.vertexBuffer->GetGPUVirtualAddress();
	vbvData.vertexBufferView.SizeInBytes = sizeof(T) * verticesCount;
	vbvData.vertexBufferView.StrideInBytes = sizeof(T);
	NAME_D3D12_OBJECT(vbvData.vertexBuffer);
}

//index buffer
struct IndexBufferViewData {
	ComPtr<ID3D12Resource>    indexBuffer;
	ComPtr<ID3D12Resource>    indexBufferUpload;
	D3D12_INDEX_BUFFER_VIEW   indexBufferView;
};

void InitializeIndexBufferView(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, const void* indices, UINT indicesCount, IndexBufferViewData& ibvData);
void InitializeIndexBufferView32(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, const void* indices, UINT indicesCount, IndexBufferViewData& ibvData);

//constants buffers
template<typename T> struct ConstantsBufferViewData {
	static constexpr UINT alignedConstantBufferSize = (sizeof(T) + 255) & ~255;

	ComPtr<ID3D12Resource>        constantBuffer;
	UINT8* mappedConstantBuffer;
	ComPtr<ID3D12DescriptorHeap>	cbvsrvHeap;
	UINT										      cbvDescriptorSize;
};

template<typename T>
CD3DX12_CPU_DESCRIPTOR_HANDLE InitializeConstantsBufferView(UINT numFrames, UINT numTextures, ComPtr<ID3D12Device2> d3dDevice, ConstantsBufferViewData<T>& cbvData) {
	//create a heap to store the location of 3(numFrames) constants buffers(mvp matrix) and (numTextures) others for the texture
	//crear un heap para guardar las direcciones de 3(numFrames) buffers de constantes(matriz mvp) y (numTextures) mas para la textura
	D3D12_DESCRIPTOR_HEAP_DESC cbvsrvHeapDesc = {};
	cbvsrvHeapDesc.NumDescriptors = numFrames + numTextures;
	cbvsrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvsrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DX::ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvsrvHeapDesc, IID_PPV_ARGS(&cbvData.cbvsrvHeap)));
	NAME_D3D12_OBJECT(cbvData.cbvsrvHeap);

	//create the constant buffer resource descriptor
	//crear el descriptor del constant buffer
	CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(numFrames * cbvData.alignedConstantBufferSize);
	CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&constantBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&cbvData.constantBuffer)));
	NAME_D3D12_OBJECT(cbvData.constantBuffer);

	//get the address of the constant buffer in both GPU/CPU
	//trae las direcciones del buffer de constantes en la GPU/CPU
	D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = cbvData.constantBuffer->GetGPUVirtualAddress();
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(cbvData.cbvsrvHeap->GetCPUDescriptorHandleForHeapStart());
	cbvData.cbvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//create a constant buffer view for each frame in the backbuffer
	//crear un constant buffer view por cada frame en el backbuffer
	for (UINT n = 0; n < numFrames; n++)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.BufferLocation = cbvGpuAddress;
		desc.SizeInBytes = cbvData.alignedConstantBufferSize;
		d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);

		cbvGpuAddress += desc.SizeInBytes;
		cbvCpuHandle.Offset(cbvData.cbvDescriptorSize);
	}
	//once this loop is done, cbvCpuHandle will be used for the SRV of the texture
	//una vez que termino este loop, cbvCpuHandle se usara para el SRV de la textura

	//map the CPU memory to the GPU and then mapped memory
	//mapea la memoria de la PCU con la GPU y luego vacia la memoria mapeada
	CD3DX12_RANGE readRange(0, 0);
	DX::ThrowIfFailed(cbvData.constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&cbvData.mappedConstantBuffer)));
	ZeroMemory(cbvData.mappedConstantBuffer, numFrames * cbvData.alignedConstantBufferSize);

	return cbvCpuHandle;
}

//samplers
void InitializeSampler(D3D12_STATIC_SAMPLER_DESC& sampler, UINT shaderRegister);

//root signature
void InitializeRootSignature(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12RootSignature>& rootSignature, UINT numCBV, UINT numSRV, UINT numSamplers=1U);
void InitializeShadowMapRootSignature(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12RootSignature>& shadowMapRootSignature);

//pipe line state
typedef void InitPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& state, const D3D12_INPUT_ELEMENT_DESC inputLayout[], UINT inputLayoutSize, ComPtr<ID3D12RootSignature> rootSignature, std::shared_ptr<ShaderCompiler::ShaderByteCode>& vertexShader,std::shared_ptr<ShaderCompiler::ShaderByteCode>& pixelShader);
InitPipelineState InitializePipelineState;
InitPipelineState InitializeShadowMapPipelineState;
template<typename T>
void CreateGraphicsPipelineState(
	ComPtr<ID3D12Device2> d3dDevice,
	ComPtr<ID3D12RootSignature>& rootSignature,
	ComPtr<ID3D12PipelineState>& pipelineState,
	InitPipelineState* initPipelineState,
	std::shared_ptr<ShaderCompiler::ShaderByteCode>& vertexShader,
	std::shared_ptr<ShaderCompiler::ShaderByteCode>& pixelShader
) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
	initPipelineState(state, T::inputLayout, _countof(T::inputLayout), rootSignature.Get(), vertexShader, pixelShader);
	pipelineState.ReleaseAndGetAddressOf();
	DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf())));
}

//shadow map
void CreateShadowMapResourceView(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12Resource> shadowMap, CD3DX12_CPU_DESCRIPTOR_HANDLE shadowMapCpuHandle);
template<typename T>
CD3DX12_CPU_DESCRIPTOR_HANDLE CreateTexturesResourceViews(
	UINT numFrames,
	ComPtr<ID3D12Device2> d3dDevice,
	ConstantsBufferViewData<T>& cbvMeshData,
	bool hasNormalMap,
	ComPtr<ID3D12Resource>& diffuseTexture,
	D3D12_SHADER_RESOURCE_VIEW_DESC& diffuseSrvDesc,
	ComPtr<ID3D12Resource>& normalMapTexture,
	D3D12_SHADER_RESOURCE_VIEW_DESC& normalMapSrvDesc
) {
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

template<typename T>
Concurrency::task<void> LoadPipeline(void* obj, ComPtr<ID3D12Device2> d3dDevice) {
	return task<void>([obj, d3dDevice]() {
		T* self = (T*)obj;
		if (self->vertexShader!=nullptr && self->vertexShader->size() > 0 &&
			self->pixelShader!=nullptr && self->pixelShader->size() > 0)
		{
			std::lock_guard<std::mutex> lock(self->shaderMutex);
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(200ms);
			CreateGraphicsPipelineState<VertexInputLayout<T::VertexType>>(d3dDevice, self->rootSignature, self->pipelineState, InitializePipelineState, self->vertexShader, self->pixelShader);
		}
	});
}

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

template<typename... Args> void whatis();
template<typename T> void whatis(T);

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
