#include "pch.h"
#include "Lights.h"
#include "../../Common/d3dx12.h"
#include "../../Common/DirectXHelper.h"
#include "../../Renderer/Renderer.h"
#include "../Camera/Camera.h"
#include "../../Renderer/DeviceUtils/D3D12Device/Builder.h"
#include "../../Renderer/DeviceUtils/D3D12Device/Interop.h"
#include "../../Renderer/DeviceUtils/RootSignature/RootSignature.h"
#include "../../Renderer/DeviceUtils/PipelineState/PipelineState.h"

namespace Scene::Lights {

	using namespace DeviceUtils::ConstantsBuffer;
	using namespace DeviceUtils::D3D12Device;

	//CBV for lights pool
	static ConstantsBufferViewDataPtr lightsCbv;
	//CBV for shadowmaps
	static ConstantsBufferViewDataPtr shadowMapsCbv;
	//SRV for shadowmaps
	static UINT numShadowMaps = 0U;
	static CD3DX12_CPU_DESCRIPTOR_HANDLE shadowMapSrvCpuDescriptorHandle[MaxLights];
	static CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapSrvGpuDescriptorHandle[MaxLights];

	ConstantsBufferViewDataPtr GetLightsConstantBufferView() { return lightsCbv; }
	ConstantsBufferViewDataPtr GetShadowMapConstantBufferView() { return shadowMapsCbv; }
	CD3DX12_GPU_DESCRIPTOR_HANDLE& GetShadowMapGpuDescriptorHandleStart() { return shadowMapSrvGpuDescriptorHandle[0]; }

	Concurrency::task<void> CreateLightsResources(const std::shared_ptr<Renderer>& renderer) {
		return Concurrency::create_task([&renderer]() {
			lightsCbv = CreateConstantsBufferViewData(renderer, sizeof(LightPool), L"lightsCbv");
			shadowMapsCbv = CreateConstantsBufferViewData(renderer, sizeof(LightAttributes)*MaxLights, L"shadowMapsCbv");
		}).then([]() {
			for (UINT i = 0; i < MaxLights; i++) {
				shadowMapSrvCpuDescriptorHandle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(GetCpuDescriptorHandleCurrent());
				shadowMapSrvGpuDescriptorHandle[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(GetGpuDescriptorHandleCurrent());
				GetCpuDescriptorHandleCurrent().Offset(GetCSUDescriptorSize());
				GetGpuDescriptorHandleCurrent().Offset(GetCSUDescriptorSize());
			}
		});
	}

	void UpdateConstantsBufferNumLights(UINT backbufferIndex, UINT numLights) {

		size_t offset = lightsCbv->alignedConstantBufferSize * backbufferIndex;
		LightPool* lpool = (LightPool*)(lightsCbv->mappedConstantBuffer + offset);
		lpool->numLights.D[0] = numLights;

	}

	void UpdateConstantsBufferLightAttributes(std::shared_ptr<Light>& light, UINT backbufferIndex, UINT lightIndex) {

		LightAttributes atts{};
		ZeroMemory(&atts, sizeof(atts));
		atts.lightType = light->lightType;

		switch (light->lightType) {
		case Ambient:
			atts.lightColor = light->ambient.color;
			break;
		case Directional:
			atts.lightColor = light->directional.color;
			atts.atts1 = {
				sinf(light->directional.rotation.x) * cosf(light->directional.rotation.y),
				sinf(light->directional.rotation.y),
				cosf(light->directional.rotation.x) * cosf(light->directional.rotation.y),
				0.0f
			};
			atts.hasShadowMap = light->hasShadowMaps;
			break;
		case Spot:
			atts.lightColor = light->spot.color;
			atts.atts1 = { light->spot.position.x, light->spot.position.y, light->spot.position.z, 0.0f };
			atts.atts2 = {
				sinf(light->spot.rotation.x) * cosf(light->spot.rotation.y),
				sinf(light->spot.rotation.y),
				cosf(light->spot.rotation.x) * cosf(light->spot.rotation.y),
				cosf(light->spot.coneAngle)
			};
			atts.atts3 = { light->spot.constant, light->spot.linear, light->spot.quadratic };
			atts.hasShadowMap = light->hasShadowMaps;
			break;
		case Point:
			atts.lightColor = light->point.color;
			atts.atts1 = { light->point.position.x, light->point.position.y, light->point.position.z, 0.0f };
			atts.atts2 = { light->point.constant, light->point.linear, light->point.quadratic, 0.0f };
			atts.hasShadowMap = light->hasShadowMaps;
			break;
		}
		
		size_t offset = lightsCbv->alignedConstantBufferSize * backbufferIndex;
		offset += sizeof(LightPool::numLights);
		offset += sizeof(atts) * lightIndex;
		memcpy(lightsCbv->mappedConstantBuffer + offset, &atts, sizeof(atts));

	}

	void UpdateConstantsBufferShadowMapAttributes(std::shared_ptr<Light>& light, UINT backbufferIndex, UINT shadowMapIndex) {

		if (!light->hasShadowMaps) return;

		ShadowMapAttributes atts{};
		ZeroMemory(&atts, sizeof(atts));

		switch (light->lightType) {
		case Directional:
		{
			XMMATRIX view = light->shadowMapCameras[0]->ViewMatrix();
			XMMATRIX projection = light->shadowMapCameras[0]->orthographic.projectionMatrix;
			atts.atts0 = XMMatrixMultiply(view, projection);
			atts.atts6 = { 0.0002f, light->directionalShadowMap.shadowMapTexelInvSize.x, light->directionalShadowMap.shadowMapTexelInvSize.y, 0.0f }; //ZBias, TexelInvSize
		}
		break;
		case Spot:
		{
			XMMATRIX view = light->shadowMapCameras[0]->ViewMatrix();
			XMMATRIX projection = light->shadowMapCameras[0]->perspective.projectionMatrix;
			atts.atts0 = XMMatrixMultiply(view, projection);
			atts.atts6 = { 0.0001f, light->spotShadowMap.shadowMapTexelInvSize.x, light->spotShadowMap.shadowMapTexelInvSize.y, 0.0f }; //ZBias, TexelInvSize
		}
		break;
		case Point:
		{
			XMMATRIX* attsN = &atts.atts0;
			for (UINT i = 0U; i < 6U; i++) {
				XMMATRIX view = light->shadowMapCameras[i]->ViewMatrix();
				XMMATRIX projection = light->shadowMapCameras[i]->perspective.projectionMatrix;
				*attsN = XMMatrixMultiply(view, projection);
				attsN++;
			}
			atts.atts6 = { 0.0001f, 3.0f, 0.0f, 0.0f }; //ZBias, PartialDerivativeScale
		}
		break;
		default:
			assert(light->lightType != Directional || light->lightType != Spot || light->lightType != Point);
		break;
		}

		size_t offset = shadowMapsCbv->alignedConstantBufferSize * backbufferIndex;
		offset += sizeof(ShadowMapAttributes) * shadowMapIndex;
		memcpy(shadowMapsCbv->mappedConstantBuffer + offset, &atts, sizeof(atts));

	}

	static std::vector<LightPtr> lights;
	std::shared_ptr<Light> CreateLight(const std::shared_ptr<Renderer>& renderer, const LightDefinition& lightParam, LoadLightFn loadFn) {
		LightPtr light = std::make_shared<LightT>(LightT{
			.name = lightParam.name,
			.lightType = lightParam.lightType,
			.hasShadowMaps = lightParam.hasShadowMap
		});
		switch (light->lightType) {
		case Ambient:
			light->ambient = lightParam.ambient;
		break;
		case Directional:
			light->directional = lightParam.directional;
		break;
		case Spot:
			light->spot = lightParam.spot;
		break;
		case Point:
			light->point = lightParam.point;
		break;
		}

		if (light->hasShadowMaps) {
			switch (light->lightType) {
			case Directional:
				CreateDirectionalLightShadowMap(renderer, light, lightParam.directionalLightShadowMapParams);
			break;
			case Spot:
				CreateSpotLightShadowMap(renderer, light, lightParam.spotLightShadowMapParams);
			break;
			case Point:
				CreatePointLightShadowMap(renderer, light, lightParam.pointLightShadowMapParams);
			break;
			default:
				assert(light->lightType != Directional || light->lightType != Spot || light->lightType != Point);
			break;
			}
			CreateShadowMapDepthStencilResource(renderer, light);
		}
		lights.push_back(light);
		if (loadFn) loadFn(light);
		return light;
	}

	std::vector<std::shared_ptr<Light>> GetLights() { return lights; }

	void DestroyLights()
	{
		auto renderer = Renderer::GetPtr();

		for (auto& l : lights) {
			if (!l->hasShadowMaps) continue;
			//l->shadowMap->Release();
			l->shadowMap = nullptr;
			l->shadowMapDsvDescriptorHeap = nullptr;
		}

		lights.clear();
	}

	void CreateDirectionalLightShadowMap(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<Light>& light, DirectionalLightShadowMapParams params) {
		light->directionalShadowMap.creation_params = params;
		light->directionalShadowMap.shadowMapScissorRect = { 0, 0, static_cast<LONG>(params.shadowMapWidth), static_cast<LONG>(params.shadowMapHeight) };
		light->directionalShadowMap.shadowMapViewport = { 0.0f, 0.0f, static_cast<FLOAT>(params.shadowMapWidth), static_cast<FLOAT>(params.shadowMapHeight), 0.0f, 1.0f };
		light->directionalShadowMap.shadowMapProjectionMatrix = XMMatrixOrthographicRH(params.viewWidth, params.viewHeight, params.nearZ, params.farZ);
		light->directionalShadowMap.shadowMapTexelInvSize = { 1.0f / static_cast<FLOAT>(params.shadowMapWidth), 1.0f / static_cast<FLOAT>(params.shadowMapHeight) };
		
		auto camera = Scene::Camera::CreateCamera({
			.name = light->name + L".cam",
			.projectionType = Camera::ProjectionsTypes::Orthographic,
			.orthographic = {
				.nearZ = params.nearZ,
				.farZ = params.farZ,
				.width = params.viewWidth,
				.height = params.viewHeight,
			},
			.rotation = light->directional.rotation,
			.light = light,
		});
		XMVECTOR camPos = XMVectorScale(XMVector3Normalize(camera->CameraFw()), -light->directional.distance);
		camera->position = *(XMFLOAT3*)camPos.m128_f32;
		camera->orthographic.updateProjectionMatrix(params.viewWidth, params.viewHeight);
		camera->CreateConstantsBufferView(renderer);
		light->shadowMapCameras.push_back(camera);
	}

	void CreateSpotLightShadowMap(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<Light>& light, SpotLightShadowMapParams params) {
		light->spotShadowMap.creation_params = params;
		light->spotShadowMap.shadowMapScissorRect = { 0, 0, static_cast<LONG>(params.shadowMapWidth), static_cast<LONG>(params.shadowMapHeight) };
		light->spotShadowMap.shadowMapViewport = { 0.0f, 0.0f, static_cast<FLOAT>(params.shadowMapWidth), static_cast<FLOAT>(params.shadowMapHeight), 0.0f, 1.0f };
		light->spotShadowMap.shadowMapProjectionMatrix = XMMatrixPerspectiveFovRH(light->spot.coneAngle * 2.0f, 1.0f, params.nearZ, params.farZ);
		light->spotShadowMap.shadowMapTexelInvSize = { 1.0f / static_cast<FLOAT>(params.shadowMapWidth), 1.0f / static_cast<FLOAT>(params.shadowMapHeight) };

		using namespace Scene::Camera;

		auto camera = CreateCamera({
			.name = light->name + L".cam",
			.projectionType = ProjectionsTypes::Perspective,
			.perspective = {
				.fovAngleY = light->spot.coneAngle * 2.0f,
				.width = static_cast<UINT>(params.viewWidth),
				.height = static_cast<UINT>(params.viewHeight),
			},
			.position = light->spot.position,
			.rotation = light->spot.rotation,
			.light = light,
		});
		camera->perspective.updateProjectionMatrix(static_cast<UINT>(params.viewWidth), static_cast<UINT>(params.viewHeight));
		camera->CreateConstantsBufferView(renderer);

		light->shadowMapCameras.push_back(camera);
	}

	void CreatePointLightShadowMap(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<Light>& light, PointLightShadowMapParams params) {
		light->pointShadowMap.creation_params = params;
		for (UINT i = 0U; i < 6U; i++) {
			light->pointShadowMap.shadowMapScissorRect[i] = { 0, static_cast<LONG>(i) * static_cast<LONG>(params.shadowMapHeight), static_cast<LONG>(params.shadowMapWidth), static_cast<LONG>(i + 1U) * static_cast<LONG>(params.shadowMapHeight) };
			light->pointShadowMap.shadowMapViewport[i] = { 0.0f, static_cast<FLOAT>(i) * static_cast<FLOAT>(params.shadowMapHeight), static_cast<FLOAT>(params.shadowMapWidth), static_cast<FLOAT>(params.shadowMapHeight), 0.0f, 1.0f };
		}
		light->pointShadowMap.shadowMapClearScissorRect = { 0, 0, static_cast<LONG>(params.shadowMapWidth), 6L * static_cast<LONG>(params.shadowMapHeight) };
		light->pointShadowMap.shadowMapClearViewport = { 0.0f, 0.0f , static_cast<FLOAT>(params.shadowMapWidth), 6L * static_cast<FLOAT>(params.shadowMapHeight), 0.0f, 1.0f };
		light->pointShadowMap.shadowMapProjectionMatrix = XMMatrixPerspectiveFovRH(DirectX::XM_PIDIV2, 1.0f, params.nearZ, params.farZ);
		for(UINT i = 0U; i < 6U; i++) {
			auto camera = Scene::Camera::CreateCamera({
				.name = light->name + L".cam("+std::to_wstring(i) + L")",
				.projectionType = Camera::ProjectionsTypes::Perspective,
				.perspective = {
					.fovAngleY = DirectX::XM_PIDIV2,
					.width = params.shadowMapWidth,
					.height = params.shadowMapHeight,
				},
				.position = light->point.position,
				//.rotation = PointLightRotations[i],
				.light = light,
			});
			camera->perspective.updateProjectionMatrix(static_cast<UINT>(params.shadowMapWidth), static_cast<UINT>(params.shadowMapHeight));
			camera->CreateConstantsBufferView(renderer);

			light->shadowMapCameras.push_back(camera);
		}
	}

	static D3D12_CLEAR_VALUE shadowMapOptimizedClearValue = {
		.Format = DXGI_FORMAT_D32_FLOAT, .DepthStencil = { 1.0f , 0 },
	};
	static D3D12_DEPTH_STENCIL_VIEW_DESC shadowMapDepthStencilViewDesc = {
		.Format = DXGI_FORMAT_D32_FLOAT, .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
		.Flags = D3D12_DSV_FLAG_NONE, .Texture2D = { .MipSlice = 0 },
	};
	static D3D12_SHADER_RESOURCE_VIEW_DESC shadowMapSrvDesc = {
	.Format = DXGI_FORMAT_R32_FLOAT,
	.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
	.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
	};
	static std::vector<ComPtr<ID3D12Resource>> shadowMapsRenderTargets;
	void CreateShadowMapDepthStencilResource(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<Light>& light) {
		//shadow maps
		light->shadowMapDsvDescriptorHeap = CreateDescriptorHeap(renderer->d3dDevice, 1U, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		light->shadowMapDsvCpuHandle = light->shadowMapDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		light->shadowMapDescriptorSize = renderer->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		std::wstring shadowMapHeapName = light->name.c_str(); shadowMapHeapName+=+L" ShadowMapHeap";
		DX::SetName(light->shadowMapDsvDescriptorHeap.Get(), shadowMapHeapName.c_str());

		const CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

		CD3DX12_RESOURCE_DESC depthStencilDesc;

		switch (light->lightType) {
		case Directional: {
			depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, light->directionalShadowMap.creation_params.shadowMapWidth,
				light->directionalShadowMap.creation_params.shadowMapHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		}
		break;
		case Spot: {
			depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, light->spotShadowMap.creation_params.shadowMapWidth,
				light->spotShadowMap.creation_params.shadowMapHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		}
		break;
		case Point: {
			depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, light->pointShadowMap.creation_params.shadowMapWidth,
				light->pointShadowMap.creation_params.shadowMapHeight * 6U, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		}
		break;
		default:
			assert(light->lightType != Directional || light->lightType != Spot || light->lightType != Point);
		break;
		}

		//create the GPU texture representation of the same DSV memory area
		ComPtr<ID3D12Resource> shadowMap;
		DX::ThrowIfFailed(renderer->d3dDevice->CreateCommittedResource(&depthHeapProperties, D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &shadowMapOptimizedClearValue, IID_PPV_ARGS(&shadowMap)));
		std::wstring shadowMapTextureName = light->name.c_str(); shadowMapTextureName+=L" ShadowMapTexture";
		DX::SetName(shadowMap.Get(), shadowMapTextureName.c_str());
		shadowMapsRenderTargets.push_back(shadowMap);
		
		renderer->d3dDevice->CreateDepthStencilView(shadowMap.Get(), &shadowMapDepthStencilViewDesc, light->shadowMapDsvCpuHandle);
		shadowMap.CopyTo(light->shadowMap.GetAddressOf());

		//get the cpu/gpu handlers and move the offsets
		renderer->d3dDevice->CreateShaderResourceView(shadowMap.Get(), &shadowMapSrvDesc, shadowMapSrvCpuDescriptorHandle[numShadowMaps]);
		numShadowMaps++;
	}

	void RenderShadowMap(std::shared_ptr<Light> light, std::shared_ptr<Renderer>& renderer, std::function<void(UINT)> renderScene) {
		switch (light->lightType) {
		case Directional: {
			renderer->SetShadowMapTarget(light->shadowMap, light->shadowMapDsvCpuHandle, light->directionalShadowMap.shadowMapScissorRect, light->directionalShadowMap.shadowMapViewport);
			renderScene(0);
		}
		break;
		case Spot: {
			renderer->SetShadowMapTarget(light->shadowMap, light->shadowMapDsvCpuHandle, light->spotShadowMap.shadowMapScissorRect, light->spotShadowMap.shadowMapViewport);
			renderScene(0);
		}
		break;
		case Point: {
			renderer->SetShadowMapTarget(light->shadowMap, light->shadowMapDsvCpuHandle, light->pointShadowMap.shadowMapClearScissorRect, light->pointShadowMap.shadowMapClearViewport);
			for (UINT i = 0; i < 6; i++) {
				renderer->commandList->RSSetViewports(1, &light->pointShadowMap.shadowMapViewport[i]);
				renderer->commandList->RSSetScissorRects(1, &light->pointShadowMap.shadowMapScissorRect[i]);
				renderScene(i);
			}
		}
		break;
		default:
			assert(light->lightType != Directional || light->lightType != Spot || light->lightType != Point);
		break;
		}
	}

	std::map<std::pair<VertexClass, const MaterialPtr>, std::pair<ComPtr<ID3D12RootSignature>, ComPtr<ID3D12PipelineState>>> shadowMapRenderAttributes;
	Concurrency::task<void> CreateShadowMapPipeline(std::shared_ptr<Renderer>& renderer, const std::map<VertexClass, const MaterialPtr> shadowMapsInputLayoutMaterial) {
		return Concurrency::create_task([renderer, shadowMapsInputLayoutMaterial] {
			for (auto& [vertexClass, material] : shadowMapsInputLayoutMaterial) {
				using namespace DeviceUtils::RootSignature;
				auto rootSignature = CreateRootSignature(renderer->d3dDevice, material);

				using namespace DeviceUtils::PipelineState;
				auto pipelineState = CreateShadowMapPipelineState(renderer->d3dDevice, vertexClass, material, rootSignature);

				std::pair<VertexClass, const MaterialPtr> key(vertexClass, material);
				std::pair<ComPtr<ID3D12RootSignature>, ComPtr<ID3D12PipelineState>> value(rootSignature, pipelineState);

				shadowMapRenderAttributes[key] = value;
			}
		});
	}

	std::pair<ComPtr<ID3D12RootSignature>, ComPtr<ID3D12PipelineState>> GetShadowMapRenderAttributes(VertexClass vertexClass, const MaterialPtr material) {
		std::pair<VertexClass, const MaterialPtr> key(vertexClass, material);
		if (shadowMapRenderAttributes.contains(key)) {
			return shadowMapRenderAttributes[key];
		} else {
			assert(true);
			return std::pair<ComPtr<ID3D12RootSignature>, ComPtr<ID3D12PipelineState>>();
		}
	}
}