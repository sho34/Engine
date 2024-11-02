#include "../framework.h"
#include "Pyramid.h"
#include "../Renderer/Render3D.h"
#include "../Common/DirectXHelper.h"
#include "../Renderer/DeviceUtils.h"
#include "../Shaders/Compiler/ShaderCompiler.h"

void Pyramid::Initialize(UINT numFrames, ComPtr<ID3D12Device2>	d3dDevice, ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>	directionalLightShadowMap, ComPtr<ID3D12Resource>	spotLightShadowMap, ComPtr<ID3D12Resource> pointLightShadowMap) {
	//upload the vertex buffer to the GPU and create the vertex buffer view
	//subir el vertex buffer a la GPU y crear el vertex buffer view
	InitializeVertexBufferView(d3dDevice, commandList, vertices, _countof(vertices), vbvData);

	//upload the index buffer to the GPU and create the index buffer view
	//subir el index buffer a la GPU y crear el index buffer view
	InitializeIndexBufferView(d3dDevice, commandList, indices, _countof(indices), ibvData);

	//create the constant buffer view descriptors for each frame, add two descriptor slot for the textures and three more for ths shadowmaps
	//crea los descriptors de constant buffer view por cada frame, agrega dos slot de descriptores para las texturas y tres mas para los shadowmaps
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = InitializeConstantsBufferView(numFrames, 5U, d3dDevice, cbvData);

	D3D12_SHADER_RESOURCE_VIEW_DESC pyramidSrvDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC pyramidNormalMapSrvDesc = {};
	CD3DX12_CPU_DESCRIPTOR_HANDLE pyramidCpuHandle(cbvCpuHandle);
	CD3DX12_CPU_DESCRIPTOR_HANDLE pyramidNormalMapCpuHandle(cbvCpuHandle);
	pyramidNormalMapCpuHandle.Offset(cbvData.cbvDescriptorSize);

	CreateTextureResource(d3dDevice, commandList, (LPWSTR)L"Assets/pyramid/pyramid.dds", pyramidTexture, pyramidTextureUpload, pyramidSrvDesc);
	CreateTextureResource(d3dDevice, commandList, (LPWSTR)L"Assets/pyramid/pyramidNormalMap.dds", pyramidNormalMapTexture, pyramidNormalMapTextureUpload, pyramidNormalMapSrvDesc, DXGI_FORMAT_R8G8B8A8_UNORM);

	d3dDevice->CreateShaderResourceView(pyramidTexture.Get(), &pyramidSrvDesc, pyramidCpuHandle);
	d3dDevice->CreateShaderResourceView(pyramidNormalMapTexture.Get(), &pyramidNormalMapSrvDesc, pyramidNormalMapCpuHandle);

	//create the descriptors and shaders resource views for the shadowmaps
	//crea los descriptores y los shader resource views de los shadowmaps
	CD3DX12_CPU_DESCRIPTOR_HANDLE directionalLightShadowMapCpuHandle(pyramidNormalMapCpuHandle);
	directionalLightShadowMapCpuHandle.Offset(cbvData.cbvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE spotLightShadowMapCpuHandle(directionalLightShadowMapCpuHandle);
	spotLightShadowMapCpuHandle.Offset(cbvData.cbvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE pointLightShadowMapCpuHandle(spotLightShadowMapCpuHandle);
	pointLightShadowMapCpuHandle.Offset(cbvData.cbvDescriptorSize);

	CreateShadowMapResourceView(d3dDevice, directionalLightShadowMap, directionalLightShadowMapCpuHandle);
	CreateShadowMapResourceView(d3dDevice, spotLightShadowMap, spotLightShadowMapCpuHandle);
	CreateShadowMapResourceView(d3dDevice, pointLightShadowMap, pointLightShadowMapCpuHandle);

	InitializeConstantsBufferView(numFrames, 0U, d3dDevice, directionalLightShadowMapCbvData);
	InitializeConstantsBufferView(numFrames, 0U, d3dDevice, spotLightShadowMapCbvData);
	for (UINT i = 0; i < 6; i++) {
		InitializeConstantsBufferView(numFrames, 0U, d3dDevice, pointLightShadowMapCbvData[i]);
	}

	InitializeRootSignature(d3dDevice, rootSignature, 1, 5);
	InitializeShadowMapRootSignature(d3dDevice, shadowMapRootSignature);
	
	auto shaderTasks = {
		ShaderCompiler::Bind(d3dDevice, this, vertexShader, LoadPipeline<Pyramid>, L"NormalMapping_vs", L"main", L"vs_6_1"),
		ShaderCompiler::Bind(d3dDevice, this, pixelShader, LoadPipeline<Pyramid>, L"NormalMapping_ps", L"main", L"ps_6_1"),
		ShaderCompiler::Bind(d3dDevice, this, shadowMapVertexShader, LoadShadowMapPipeline<Pyramid>, L"ShadowMap_vs", L"main", L"vs_6_1"),
		ShaderCompiler::Bind(d3dDevice, this, shadowMapPixelShader, LoadShadowMapPipeline<Pyramid>, L"ShadowMap_ps", L"main", L"ps_6_1")
	};

	auto waitForShaders = when_all(std::begin(shaderTasks), std::end(shaderTasks));

	waitForShaders.wait();

	loadingComplete = true;
}

void Pyramid::DestroyUploadResources() {
	vbvData.vertexBufferUpload = nullptr;// ->Release();
	ibvData.indexBufferUpload = nullptr;// ->Release();
	pyramidTextureUpload = nullptr;// ->Release();
	pyramidNormalMapTextureUpload = nullptr;// ->Release();
}

void Pyramid::Destroy() {
	pyramidTexture = nullptr; //->Release();
	pyramidNormalMapTexture = nullptr; //->Release();
	vbvData.vertexBuffer = nullptr;// ->Release();
	ibvData.indexBuffer = nullptr;// ->Release();
	cbvData.constantBuffer = nullptr;// ->Release();
	cbvData.cbvsrvHeap = nullptr;//->Release();
	directionalLightShadowMapCbvData.constantBuffer = nullptr;//->Release();
	spotLightShadowMapCbvData.constantBuffer = nullptr;//->Release();
	for (auto cbv : pointLightShadowMapCbvData) {
		cbv.constantBuffer = nullptr;//->Release();
	}
	rootSignature = nullptr;//->Release();
	pipelineState = nullptr;//->Release();
}

void Pyramid::Step() {
	if (!loadingComplete) return;

	yRotation += yRotationStep;
	yTranslation += yTranslationStep;
	zTranslation += zTranslationStep;

	position = { XMVectorGetX(position), 2 * sinf(yTranslation) + yOffset, 2 * sinf(zTranslation) + zOffset , 0 };
}

void Pyramid::UpdateConstantsBuffer(UINT backBufferIndex, BOOL useBlinnPhong, XMMATRIX viewProjection, XMVECTOR eyePos, XMVECTOR ambientLightColor, XMVECTOR directionalLightDirection, XMVECTOR directionalLightColor, XMVECTOR spotLightPosition, XMVECTOR spotLightColor, XMVECTOR spotLightDirectionAndAngle, XMVECTOR spotLightAttenuation, XMVECTOR pointLightColor, XMVECTOR pointLightPosition, XMVECTOR pointLightAttenuation, BOOL shadowMapsEnabled, XMMATRIX directionalLightShadowMapProjection, XMFLOAT2 directionalLightShadowMapTexelInvSize, XMMATRIX spotLightShadowMapProjection, XMFLOAT2 spotLightShadowMapTexelInvSize, XMMATRIX pointLightShadowMapProjection[6], BOOL normalMappingEnabled)
{
	if (!loadingComplete) return;

	LightingShaderConstants constants;

	XMMATRIX world = XMMatrixMultiply(XMMatrixRotationY(yRotation), XMMatrixTranslationFromVector(position));
	constants.numTextures = 1;
	constants.useBlinnPhong = useBlinnPhong;
	constants.materialSpecularExponent = materialSpecularExponent;
	constants.normalMaps = normalMappingEnabled;
	constants.normalMapTextureIndex = 1;
	constants.shadowMapsTextureIndex = 2;
	constants.shadowMaps = shadowMapsEnabled;
	constants.worldViewProjection = XMMatrixTranspose(XMMatrixMultiply(world, viewProjection));
	constants.world = XMMatrixTranspose(world);
	constants.directionalLightShadowMapProjection = XMMatrixTranspose(directionalLightShadowMapProjection);
	constants.directionalLightShadowMapTexelInvSize = directionalLightShadowMapTexelInvSize;
	constants.spotLightShadowMapProjection = XMMatrixTranspose(spotLightShadowMapProjection);
	constants.spotLightShadowMapTexelInvSize = spotLightShadowMapTexelInvSize;
	for (UINT i = 0; i < 6U; i++) {
		constants.pointLightShadowMapProjection[i] = XMMatrixTranspose(pointLightShadowMapProjection[i]);
	}
	constants.pointLightShadowMapPartialDerivativeScale = 3.0f;
	constants.directionalLightShadowMapZBias = 0.0002f;
	constants.spotLightShadowMapZBias = 0.0001f;
	constants.pointLightShadowMapZBias = 0.0001f;
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

void Pyramid::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex)
{
	Render3DPrimitive(commandList, backBufferIndex, *this);
}

void Pyramid::UpdateShadowMapConstantsBuffer(UINT backBufferIndex, XMMATRIX shadowMapViewProjection, ConstantsBufferViewData<XMMATRIX>& shadowMapCbvData) {
	if (!loadingComplete) return;

	XMMATRIX world = XMMatrixMultiply(XMMatrixRotationY(yRotation), XMMatrixTranslationFromVector(position));
	XMMATRIX shadowMapWorldViewProjection = XMMatrixTranspose(XMMatrixMultiply(world, shadowMapViewProjection));

	UINT8* destination = shadowMapCbvData.mappedConstantBuffer + (backBufferIndex * shadowMapCbvData.alignedConstantBufferSize);
	memcpy(destination, &shadowMapWorldViewProjection, sizeof(shadowMapWorldViewProjection));
}

void Pyramid::RenderShadowMap(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT backBufferIndex, ConstantsBufferViewData<XMMATRIX>& shadowMapCbvData) {
	Render3DPrimitiveShadowMap(commandList, backBufferIndex, shadowMapCbvData, *this);
}