#include "../framework.h"
#include "Floor.h"
#include "../Renderer/Render3D.h"
#include "../Common/DirectXHelper.h"
#include "../Renderer/DeviceUtils.h"
#include "../Shaders/Compiler/ShaderCompiler.h"

void Floor::Initialize(UINT numFrames, ComPtr<ID3D12Device2>	d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>	directionalLightShadowMap, ComPtr<ID3D12Resource>	spotLightShadowMap, ComPtr<ID3D12Resource> pointLightShadowMap) {
	//upload the vertex buffer to the GPU and create the vertex buffer view
	//subir el vertex buffer a la GPU y crear el vertex buffer view
	InitializeVertexBufferView(d3dDevice, commandList, vertices, _countof(vertices), vbvData);

	//upload the index buffer to the GPU and create the index buffer view
	//subir el index buffer a la GPU y crear el index buffer view
	InitializeIndexBufferView(d3dDevice, commandList, indices, _countof(indices), ibvData);

	//create the constant buffer view descriptors for each frame plus three for shadowmaps
	//crea los descriptors de constant buffer view por cada frame mas tres para los shadowmaps
	CD3DX12_CPU_DESCRIPTOR_HANDLE directionalLightShadowMapCpuHandle = InitializeConstantsBufferView(numFrames, 3U, d3dDevice, cbvData);
	CD3DX12_CPU_DESCRIPTOR_HANDLE spotLightShadowMapCpuHandle(directionalLightShadowMapCpuHandle);
	spotLightShadowMapCpuHandle.Offset(cbvData.cbvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE pointLightShadowMapCpuHandle(spotLightShadowMapCpuHandle);
	pointLightShadowMapCpuHandle.Offset(cbvData.cbvDescriptorSize);

	CreateShadowMapResourceView(d3dDevice, directionalLightShadowMap, directionalLightShadowMapCpuHandle);
	CreateShadowMapResourceView(d3dDevice, spotLightShadowMap, spotLightShadowMapCpuHandle);
	CreateShadowMapResourceView(d3dDevice, pointLightShadowMap, pointLightShadowMapCpuHandle);

	InitializeRootSignature(d3dDevice, rootSignature, 1, 3);

	auto shaderTasks = {
		ShaderCompiler::Bind(d3dDevice, this, vertexShader, LoadPipeline<Floor>, L"Floor_vs", L"main", L"vs_6_1"),
		ShaderCompiler::Bind(d3dDevice, this, pixelShader, LoadPipeline<Floor>, L"Floor_ps", L"main", L"ps_6_1"),
	};

	auto waitForShaders = when_all(std::begin(shaderTasks), std::end(shaderTasks));

	waitForShaders.wait();

	loadingComplete = true;
}

void Floor::DestroyUploadResources() {
	vbvData.vertexBufferUpload = nullptr;//->Release();
	ibvData.indexBufferUpload = nullptr;//->Release();
}

void Floor::Destroy() {
	vbvData.vertexBuffer = nullptr;//->Release();
	ibvData.indexBuffer = nullptr;//->Release();
	cbvData.constantBuffer = nullptr;//->Release();
	cbvData.cbvsrvHeap = nullptr;//->Release();
	rootSignature = nullptr;//->Release();
	pipelineState = nullptr;//->Release();
}

void Floor::UpdateConstantsBuffer(UINT backBufferIndex, BOOL useBlinnPhong, XMMATRIX viewProjection, XMVECTOR eyePos, XMVECTOR ambientLightColor, XMVECTOR directionalLightDirection, XMVECTOR directionalLightColor, XMVECTOR spotLightPosition, XMVECTOR spotLightColor, XMVECTOR spotLightDirectionAndAngle, XMVECTOR spotLightAttenuation, XMVECTOR pointLightColor, XMVECTOR pointLightPosition, XMVECTOR pointLightAttenuation, BOOL shadowMapsEnabled, XMMATRIX directionalLightShadowMapProjection, XMFLOAT2 directionalLightShadowMapTexelInvSize, XMMATRIX spotLightShadowMapProjection, XMFLOAT2 spotLightShadowMapTexelInvSize, XMMATRIX pointLightShadowMapProjection[6])
{
	if (!loadingComplete) return;

	LightingShaderConstants constants;

	XMMATRIX world = XMMatrixMultiply(XMMatrixScaling(10.0f, 1.0f, 10.0f), XMMatrixTranslation(0.0f, -2.0f, 0.0f));
	constants.useBlinnPhong = useBlinnPhong;
	constants.materialSpecularExponent = materialSpecularExponent;
	constants.worldViewProjection = XMMatrixTranspose(XMMatrixMultiply(world, viewProjection));
	constants.world = XMMatrixTranspose(world);
	constants.shadowMaps = shadowMapsEnabled;
	constants.directionalLightShadowMapProjection = XMMatrixTranspose(directionalLightShadowMapProjection);
	constants.directionalLightShadowMapTexelInvSize = directionalLightShadowMapTexelInvSize;
	constants.spotLightShadowMapProjection = XMMatrixTranspose(spotLightShadowMapProjection);
	constants.spotLightShadowMapTexelInvSize = spotLightShadowMapTexelInvSize;
	for (UINT i = 0; i < 6U; i++) {
		constants.pointLightShadowMapProjection[i] = XMMatrixTranspose(pointLightShadowMapProjection[i]);
	}
	constants.pointLightShadowMapPartialDerivativeScale = 3.0f;
	constants.directionalLightShadowMapZBias = 0.0f;
	constants.spotLightShadowMapZBias = 0.0f;
	constants.pointLightShadowMapZBias = 0.0f;
	constants.eyePos = eyePos;
	constants.ambientLightColor = ambientLightColor;
	constants.directionalLightDirection = directionalLightDirection;
	constants.directionalLightColor = directionalLightColor;
	constants.spotLightPosition = spotLightPosition;
	constants.spotLightColor = spotLightColor;
	constants.spotLightDirectionAndAngle = spotLightDirectionAndAngle;
	constants.spotLightAttenuation = spotLightAttenuation;
	constants.pointLightColor = pointLightColor;
	constants.pointLightPosition = pointLightPosition;
	constants.pointLightAttenuation = pointLightAttenuation;

	UINT8* destination = cbvData.mappedConstantBuffer + (backBufferIndex * cbvData.alignedConstantBufferSize);
	memcpy(destination, &constants, sizeof(constants));
}

void Floor::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex)
{
	Render3DPrimitive(commandList, backBufferIndex, *this);
}
