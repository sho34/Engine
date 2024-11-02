#include "../framework.h"
#include "AnimatedQuad.h"
#include "../Renderer/Render3D.h"
#include "../Common/DirectXHelper.h"
#include "../Renderer/DeviceUtils.h"
#include "../Shaders/Compiler/ShaderCompiler.h"

void AnimatedQuad::Initialize(UINT numFrames, ComPtr<ID3D12Device2>	d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, std::wstring path, UINT numFramesInTexture, FLOAT timeBetweenFrames, FLOAT alphaThreshold, DXGI_FORMAT textureFormat) {
	//upload the vertex buffer to the GPU and create the vertex buffer view
	//subir el vertex buffer a la GPU y crear el vertex buffer view
	InitializeVertexBufferView(d3dDevice, commandList, vertices, _countof(vertices), vbvData);

	//upload the index buffer to the GPU and create the index buffer view
	//subir el index buffer a la GPU y crear el index buffer view
	InitializeIndexBufferView(d3dDevice, commandList, indices, _countof(indices), ibvData);

	//create the constant buffer view descriptors for each frame, add one descriptor slot for the textures
	//crea los descriptors de constant buffer view por cada frame, agrega un slot de descriptore para la texturas
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = InitializeConstantsBufferView(numFrames, 1U, d3dDevice, cbvData);
	D3D12_SHADER_RESOURCE_VIEW_DESC texSrvDesc = {};
	CD3DX12_CPU_DESCRIPTOR_HANDLE texCpuHandle(cbvCpuHandle);

	CreateTextureArrayResource(d3dDevice, commandList, (const LPWSTR)path.c_str(), texture, textureUpload, texSrvDesc, numFramesInTexture, textureFormat);

	d3dDevice->CreateShaderResourceView(texture.Get(), &texSrvDesc, texCpuHandle);

	numAnimatedFrames = numFramesInTexture;
	timePerFrames = timeBetweenFrames;
	alphaCut = alphaThreshold;

	InitializeRootSignature(d3dDevice, rootSignature, 1, 1);

	auto shaderTasks = {
		ShaderCompiler::Bind(d3dDevice, this, vertexShader, LoadPipeline<AnimatedQuad>, L"AnimatedQuad_vs", L"main", L"vs_6_1"),
		ShaderCompiler::Bind(d3dDevice, this, pixelShader, LoadPipeline<AnimatedQuad>, L"AnimatedQuad_ps", L"main", L"ps_6_1"),
	};

	auto waitForShaders = when_all(std::begin(shaderTasks), std::end(shaderTasks));

	waitForShaders.wait();

	loadingComplete = true;
}

template<>
void CreateGraphicsPipelineState<VertexInputLayout<AnimatedQuad::VertexType>>(
	ComPtr<ID3D12Device2> d3dDevice,
	ComPtr<ID3D12RootSignature>& rootSignature,
	ComPtr<ID3D12PipelineState>& pipelineState,
	InitPipelineState* initPipelineState,
	std::shared_ptr<ShaderCompiler::ShaderByteCode>& vertexShader,
	std::shared_ptr<ShaderCompiler::ShaderByteCode>& pixelShader
) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
	initPipelineState(state, VertexInputLayout<AnimatedQuad::VertexType>::inputLayout, _countof(VertexInputLayout<AnimatedQuad::VertexType>::inputLayout), rootSignature.Get(), vertexShader, pixelShader);
	state.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	pipelineState.ReleaseAndGetAddressOf();
	DX::ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf())));
}

void AnimatedQuad::DestroyUploadResources() {
	if(textureUpload) textureUpload = nullptr;//->Release();
	vbvData.vertexBufferUpload = nullptr;//->Release();
	ibvData.indexBufferUpload = nullptr;//->Release();
}

void AnimatedQuad::Destroy() {
	texture = nullptr;//->Release();
	vbvData.vertexBuffer = nullptr;//->Release();
	ibvData.indexBuffer = nullptr;//->Release();
	cbvData.constantBuffer = nullptr;//->Release();
	cbvData.cbvsrvHeap = nullptr;//->Release();
	rootSignature = nullptr;//->Release();
	pipelineState = nullptr;//->Release();
}

void AnimatedQuad::Step(FLOAT delta) {
	currentTime += delta;
	currentFrame = static_cast<UINT>(currentTime / timePerFrames) % numAnimatedFrames;
}

void AnimatedQuad::UpdateConstantsBuffer(UINT backBufferIndex, XMMATRIX viewProjection)
{
	if (!loadingComplete) return;

	AnimatedQuadShaderConstants constants;
	constants.worldViewProjection = XMMatrixTranspose(XMMatrixMultiply(world, viewProjection));
	constants.index = currentFrame;
	constants.alphaCut = alphaCut;

	UINT8* destination = cbvData.mappedConstantBuffer + (backBufferIndex * cbvData.alignedConstantBufferSize);
	memcpy(destination, &constants, sizeof(constants));
}

void AnimatedQuad::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex)
{
	Render3DPrimitive(commandList, backBufferIndex, *this);
}
