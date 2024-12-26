#include "pch.h"
#include "Lights.h"
#include "../../Common/d3dx12.h"
#include "../../Common/DirectXHelper.h"
#include "../../Renderer/Renderer.h"
#include "../Camera/CameraImpl.h"
#include "../../Renderer/DeviceUtils/D3D12Device/Builder.h"
#include "../../Renderer/DeviceUtils/D3D12Device/Interop.h"
#include "../../Renderer/DeviceUtils/RootSignature/RootSignature.h"
#include "../../Renderer/DeviceUtils/PipelineState/PipelineState.h"
#if defined(_EDITOR)
#include "../../Animation/Effects/Effects.h"
#endif

extern std::shared_ptr<Renderer> renderer;

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

	std::map<std::pair<VertexClass, const MaterialPtr>, std::pair<CComPtr<ID3D12RootSignature>, CComPtr<ID3D12PipelineState>>> shadowMapRenderAttributes;
	static std::vector<CComPtr<ID3D12Resource>> shadowMapsRenderTargets;

	ConstantsBufferViewDataPtr GetLightsConstantBufferView() { return lightsCbv; }
	ConstantsBufferViewDataPtr GetShadowMapConstantBufferView() { return shadowMapsCbv; }
	CD3DX12_GPU_DESCRIPTOR_HANDLE& GetShadowMapGpuDescriptorHandleStart() { return shadowMapSrvGpuDescriptorHandle[0]; }

	Concurrency::task<void> CreateLightsResources() {
		return Concurrency::create_task([]() {
			lightsCbv = CreateConstantsBufferViewData(renderer, sizeof(LightPool), L"lightsCbv");
			shadowMapsCbv = CreateConstantsBufferViewData(renderer, sizeof(LightAttributes)*MaxLights, L"shadowMapsCbv");
		}).then([]() {
			for (UINT i = 0; i < MaxLights; i++) {
				AllocCSUDescriptor(shadowMapSrvCpuDescriptorHandle[i], shadowMapSrvGpuDescriptorHandle[i]);
			}
		});
	}

	void DestroyLightsResources() {

		for (UINT i = 0; i < MaxLights; i++) {
			FreeCSUDescriptor(shadowMapSrvCpuDescriptorHandle[i], shadowMapSrvGpuDescriptorHandle[i]);
		}

		lightsCbv->constantBuffer.Release();
		lightsCbv->constantBuffer = nullptr;
		shadowMapsCbv->constantBuffer.Release();
		shadowMapsCbv->constantBuffer = nullptr;

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

	static std::map<std::wstring, LightPtr> lightsByName;
	static std::vector<LightPtr> lights;

	std::shared_ptr<Light> CreateLight(nlohmann::json lightj) {

		LightPtr light = std::make_shared<LightT>();
		light->this_ptr = light;
		light->name = StringToWString(lightj["name"]);
		light->lightType = StrToLightType[StringToWString(lightj["lightType"])];

		switch (light->lightType) {
		case Ambient:
			light->ambient = {
				.color = JsonToFloat3(lightj["ambient"]["color"])
			};
			break;
		case Directional:
			replaceFromJson(light->hasShadowMaps, lightj["directional"], "hasShadowMaps");
			light->directional = {
				.color = JsonToFloat3(lightj["directional"]["color"]),
				.distance = lightj["directional"]["distance"],
				.rotation = JsonToFloat2(lightj["directional"]["rotation"])
			};
			break;
		case Spot:
			replaceFromJson(light->hasShadowMaps, lightj["spot"], "hasShadowMaps");
			light->spot = {
				.color = JsonToFloat3(lightj["spot"]["color"]),
				.position = JsonToFloat3(lightj["spot"]["position"]),
				.rotation = JsonToFloat2(lightj["spot"]["rotation"]),
				.coneAngle = lightj["spot"]["coneAngle"],
				.attenuation = JsonToFloat3(lightj["spot"]["attenuation"])
			};
			break;
		case Point:
			replaceFromJson(light->hasShadowMaps, lightj["point"], "hasShadowMaps");
			light->point = {
				.color = JsonToFloat3(lightj["point"]["color"]),
				.position = JsonToFloat3(lightj["point"]["position"]),
				.attenuation = JsonToFloat3(lightj["point"]["attenuation"])
			};
			break;
		}

		lights.push_back(light);
		lightsByName[light->name] = light;

		if (light->hasShadowMaps) {
			switch (light->lightType) {
			case Directional:
				CreateDirectionalLightShadowMap(light, lightj["directional"]["directionalShadowMap"]);
				break;
			case Spot:
				CreateSpotLightShadowMap(light, lightj["spot"]["spotShadowMap"]);
				break;
			case Point:
				CreatePointLightShadowMap(light, lightj["point"]["pointShadowMap"]);
				break;
			default:
				assert(light->lightType != Directional || light->lightType != Spot || light->lightType != Point);
				break;
			}
			CreateShadowMapDepthStencilResource(light);
		}

		return light;
	}

	std::vector<std::shared_ptr<Light>> GetLights() { return lights; }

	std::shared_ptr<Light> GetLight(std::wstring lightName)	{
		return lightsByName[lightName];
	}

	std::vector<std::wstring> GetLightsNames() {
		std::vector<std::wstring> names;
		std::transform(lights.begin(), lights.end(), std::back_inserter(names), [](LightPtr l) { return l->name; });
		return names;
	}

	std::map<std::wstring, std::shared_ptr<Light>> GetNamedLights() {
		return lightsByName;
	}

	void DestroyLights()
	{
		for (auto& l : lights) {
			if (!l->hasShadowMaps) continue;

			for (auto& cam : l->shadowMapCameras) {
				cam->cameraCbv->constantBuffer.Release();
				cam->cameraCbv->constantBuffer = nullptr;
				cam->cameraCbv.reset();
				cam->cameraCbv = nullptr;
			}
			l->shadowMapCameras.clear();

			l->shadowMap.Release();
			l->shadowMap = nullptr;
			l->shadowMapDsvDescriptorHeap.Release();
			l->shadowMapDsvDescriptorHeap = nullptr;
			l = nullptr;
		}
		lights.clear();

		for (auto& [name, l] : lightsByName) {
			l = nullptr;
		}
		lightsByName.clear();

		for (auto& rt : shadowMapsRenderTargets) {
			rt.Release();
			rt = nullptr;
		}
		shadowMapsRenderTargets.clear();
		
		numShadowMaps = 0;
	}

	void CreateDirectionalLightShadowMap(const std::shared_ptr<Light>& light, nlohmann::json params) {
		light->shadowMapParams = params;
		light->directionalShadowMap.shadowMapScissorRect = { 0, 0, static_cast<LONG>(params["shadowMapWidth"]), static_cast<LONG>(params["shadowMapHeight"])};
		light->directionalShadowMap.shadowMapViewport = { 0.0f, 0.0f, static_cast<FLOAT>(params["shadowMapWidth"]), static_cast<FLOAT>(params["shadowMapHeight"]), 0.0f, 1.0f };
		light->directionalShadowMap.shadowMapProjectionMatrix = XMMatrixOrthographicRH(params["viewWidth"], params["viewHeight"], params["nearZ"], params["farZ"]);
		light->directionalShadowMap.shadowMapTexelInvSize = { 1.0f / static_cast<FLOAT>(params["shadowMapWidth"]), 1.0f / static_cast<FLOAT>(params["shadowMapHeight"]) };
		
		auto camera = CreateCamera({
			{ "name", WStringToString(light->name + L".cam") },
			{ "projectionType", "Orthographic" },
			{ "orthographic", {
				{ "nearZ", params["nearZ"] },
				{ "farZ", params["farZ"] },
				{ "width", params["viewWidth"] },
				{ "height", params["viewHeight"] },
			}},
			{ "rotation", { light->directional.rotation.x, light->directional.rotation.y } },
			{ "light", WStringToString(light->name) }
		});
		XMVECTOR camPos = XMVectorScale(XMVector3Normalize(camera->CameraFw()), -light->directional.distance);
		camera->position = *(XMFLOAT3*)camPos.m128_f32;
		camera->orthographic.updateProjectionMatrix(params["viewWidth"], params["viewHeight"]);
		camera->CreateConstantsBufferView(renderer);
		light->shadowMapCameras.push_back(camera);
	}

	void CreateSpotLightShadowMap(const std::shared_ptr<Light>& light, nlohmann::json params) {
		light->shadowMapParams = params;
		light->spotShadowMap.shadowMapScissorRect = { 0, 0, static_cast<LONG>(params["shadowMapWidth"]), static_cast<LONG>(params["shadowMapHeight"])};
		light->spotShadowMap.shadowMapViewport = { 0.0f, 0.0f, static_cast<FLOAT>(params["shadowMapWidth"]), static_cast<FLOAT>(params["shadowMapHeight"]), 0.0f, 1.0f };
		light->spotShadowMap.shadowMapProjectionMatrix = XMMatrixPerspectiveFovRH(light->spot.coneAngle * 2.0f, 1.0f, params["nearZ"], params["farZ"]);
		light->spotShadowMap.shadowMapTexelInvSize = { 1.0f / static_cast<FLOAT>(params["shadowMapWidth"]), 1.0f / static_cast<FLOAT>(params["shadowMapHeight"]) };

		using namespace Scene::Camera;

		auto camera = CreateCamera({
			{ "name", WStringToString(light->name + L".cam") },
			{ "projectionType", "Perspective" },
			{ "perspective", {
				{ "fovAngleY", light->spot.coneAngle * 2.0f },
				{ "width", params["viewWidth"] },
				{ "height", params["viewHeight"] },
			}},
			{ "position", { light->spot.position.x, light->spot.position.y, light->spot.position.z }},
			{ "rotation", { light->spot.rotation.x, light->spot.rotation.y }},
			{ "light", WStringToString(light->name) }
		});
		camera->perspective.updateProjectionMatrix(params["viewWidth"], params["viewHeight"]);
		camera->CreateConstantsBufferView(renderer);

		light->shadowMapCameras.push_back(camera);
	}

	void CreatePointLightShadowMap(const std::shared_ptr<Light>& light, nlohmann::json params) {
		light->shadowMapParams = params;
		for (UINT i = 0U; i < 6U; i++) {
			light->pointShadowMap.shadowMapScissorRect[i] = { 0, static_cast<LONG>(i) * static_cast<LONG>(params["shadowMapHeight"]), static_cast<LONG>(params["shadowMapWidth"]), static_cast<LONG>(i + 1U) * static_cast<LONG>(params["shadowMapHeight"])};
			light->pointShadowMap.shadowMapViewport[i] = { 0.0f, static_cast<FLOAT>(i) * static_cast<FLOAT>(params["shadowMapHeight"]), static_cast<FLOAT>(params["shadowMapWidth"]), static_cast<FLOAT>(params["shadowMapHeight"]), 0.0f, 1.0f };
		}
		light->pointShadowMap.shadowMapClearScissorRect = { 0, 0, static_cast<LONG>(params["shadowMapWidth"]), 6L * static_cast<LONG>(params["shadowMapHeight"]) };
		light->pointShadowMap.shadowMapClearViewport = { 0.0f, 0.0f , static_cast<FLOAT>(params["shadowMapWidth"]), 6L * static_cast<FLOAT>(params["shadowMapHeight"]), 0.0f, 1.0f };
		light->pointShadowMap.shadowMapProjectionMatrix = XMMatrixPerspectiveFovRH(DirectX::XM_PIDIV2, 1.0f, params["nearZ"], params["farZ"]);
		for(UINT i = 0U; i < 6U; i++) {
			auto camera = CreateCamera({
				{ "name", WStringToString(light->name + L".cam." + std::to_wstring(i)) },
				{ "projectionType", "Perspective" },
				{ "perspective", {
					{ "fovAngleY", DirectX::XM_PIDIV2 },
					{ "width", static_cast<float>(params["shadowMapWidth"]) },
					{ "height", static_cast<float>(params["shadowMapHeight"]) },
				}},
				{ "position", { light->point.position.x, light->point.position.y, light->point.position.z }},
				{ "light", WStringToString(light->name) }
			});
			camera->perspective.updateProjectionMatrix(static_cast<float>(params["shadowMapWidth"]), static_cast<float>(params["shadowMapHeight"]));
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
	
	void CreateShadowMapDepthStencilResource(const std::shared_ptr<Light>& light) {
		//shadow maps
		light->shadowMapDsvDescriptorHeap = CreateDescriptorHeap(renderer->d3dDevice, 1U, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		light->shadowMapDsvCpuHandle = light->shadowMapDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		light->shadowMapDescriptorSize = renderer->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		std::wstring shadowMapHeapName = light->name.c_str(); shadowMapHeapName+=+L" ShadowMapHeap";
		DX::SetName(light->shadowMapDsvDescriptorHeap, shadowMapHeapName.c_str());

		const CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

		CD3DX12_RESOURCE_DESC depthStencilDesc;

		switch (light->lightType) {
		case Directional: {
			depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, light->shadowMapParams["shadowMapWidth"],
				light->shadowMapParams["shadowMapHeight"], 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		}
		break;
		case Spot: {
			depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, light->shadowMapParams["shadowMapWidth"],
				light->shadowMapParams["shadowMapHeight"], 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		}
		break;
		case Point: {
			depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, light->shadowMapParams["shadowMapWidth"],
				light->shadowMapParams["shadowMapHeight"] * 6U, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		}
		break;
		default:
			assert(light->lightType != Directional || light->lightType != Spot || light->lightType != Point);
		break;
		}

		//create the GPU texture representation of the same DSV memory area
		DX::ThrowIfFailed(renderer->d3dDevice->CreateCommittedResource(&depthHeapProperties, D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &shadowMapOptimizedClearValue, IID_PPV_ARGS(&light->shadowMap)));
		std::wstring shadowMapTextureName = light->name.c_str(); shadowMapTextureName+=L" ShadowMapTexture";
		DX::SetName(light->shadowMap, shadowMapTextureName.c_str());
		shadowMapsRenderTargets.push_back(light->shadowMap);
		
		renderer->d3dDevice->CreateDepthStencilView(light->shadowMap, &shadowMapDepthStencilViewDesc, light->shadowMapDsvCpuHandle);

		//get the cpu/gpu handlers and move the offsets
		renderer->d3dDevice->CreateShaderResourceView(light->shadowMap, &shadowMapSrvDesc, shadowMapSrvCpuDescriptorHandle[numShadowMaps]);
		numShadowMaps++;
	}

	void RenderShadowMap(std::shared_ptr<Light> light, std::function<void(UINT)> renderScene) {
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

	Concurrency::task<void> CreateShadowMapPipeline(const std::map<VertexClass, const MaterialPtr> shadowMapsInputLayoutMaterial) {
		return Concurrency::create_task([shadowMapsInputLayoutMaterial] {
			for (auto& [vertexClass, material] : shadowMapsInputLayoutMaterial) {
				using namespace DeviceUtils::RootSignature;
				auto rootSignature = CreateRootSignature(renderer->d3dDevice, material);

				using namespace DeviceUtils::PipelineState;
				auto pipelineState = CreateShadowMapPipelineState(renderer->d3dDevice, vertexClass, material, rootSignature);

				std::pair<VertexClass, const MaterialPtr> key(vertexClass, material);
				std::pair<CComPtr<ID3D12RootSignature>, CComPtr<ID3D12PipelineState>> value(rootSignature, pipelineState);

				shadowMapRenderAttributes[key] = value;
			}
		});
	}

	std::pair<CComPtr<ID3D12RootSignature>, CComPtr<ID3D12PipelineState>> GetShadowMapRenderAttributes(VertexClass vertexClass, const MaterialPtr material) {
		std::pair<VertexClass, const MaterialPtr> key(vertexClass, material);
		if (shadowMapRenderAttributes.contains(key)) {
			return shadowMapRenderAttributes[key];
		} else {
			assert(true);
			return std::pair<CComPtr<ID3D12RootSignature>, CComPtr<ID3D12PipelineState>>();
		}
	}

	void DestroyShadowMapAttributes() {

		for (auto& [p1, p2] : shadowMapRenderAttributes) {
			p2.first.Release();
			p2.first = nullptr;
			p2.second.Release();
			p2.second = nullptr;
		}
		shadowMapRenderAttributes.clear();

	}

#if defined(_EDITOR)
	nlohmann::json Light::json() {
		nlohmann::json j = nlohmann::json({});

		std::string jname;
		std::transform(name.begin(), name.end(), std::back_inserter(jname), [](wchar_t c) { return (char)c; });
		j["name"] = jname;

		std::string jlightType;
		std::transform(LightTypesStr[lightType].begin(), LightTypesStr[lightType].end(), std::back_inserter(jlightType), [](wchar_t c) { return (char)c; });
		j["lightType"] = jlightType;

		auto buildJsonAmbientLight = [this](auto& j) {
			j["ambient"]["color"] = { ambient.color.x, ambient.color.y, ambient.color.z };
		};

		auto buildJsonDirectionalLight = [this](auto& j) {
			j["directional"]["color"] = { directional.color.x, directional.color.y, directional.color.z };
			j["directional"]["distance"] = directional.distance;
			j["directional"]["rotation"] = { directional.rotation.x, directional.rotation.y };
			j["directional"]["hasShadowMaps"] = hasShadowMaps;
			if (hasShadowMaps) {
				j["directional"]["directionalShadowMap"]["shadowMapWidth"] = shadowMapParams["shadowMapWidth"];
				j["directional"]["directionalShadowMap"]["shadowMapHeight"] = shadowMapParams["shadowMapHeight"];
				j["directional"]["directionalShadowMap"]["viewWidth"] = shadowMapParams["viewWidth"];
				j["directional"]["directionalShadowMap"]["viewHeight"] = shadowMapParams["viewHeight"];
				j["directional"]["directionalShadowMap"]["nearZ"] = shadowMapParams["nearZ"];
				j["directional"]["directionalShadowMap"]["farZ"] = shadowMapParams["farZ"];
			}
		};

		auto buildJsonSpotLight = [this](auto& j) {
			j["spot"]["color"] = { spot.color.x, spot.color.y, spot.color.z };
			j["spot"]["position"] = { spot.position.x, spot.position.y, spot.position.z };
			j["spot"]["rotation"] = { spot.rotation.x, spot.rotation.y };
			j["spot"]["coneAngle"] = spot.coneAngle;
			j["spot"]["attenuation"] = { spot.attenuation.x, spot.attenuation.y, spot.attenuation.z };
			j["spot"]["hasShadowMaps"] = hasShadowMaps;
			if (hasShadowMaps) {
				j["spot"]["spotShadowMap"]["shadowMapWidth"] = shadowMapParams["shadowMapWidth"];
				j["spot"]["spotShadowMap"]["shadowMapHeight"] = shadowMapParams["shadowMapHeight"];
				j["spot"]["spotShadowMap"]["viewWidth"] = shadowMapParams["viewWidth"];
				j["spot"]["spotShadowMap"]["viewHeight"] = shadowMapParams["viewHeight"];
				j["spot"]["spotShadowMap"]["nearZ"] = shadowMapParams["nearZ"];
				j["spot"]["spotShadowMap"]["farZ"] = shadowMapParams["farZ"];
			}
		};

		auto buildJsonPointLight = [this](auto& j) {
			j["point"]["color"] = { point.color.x, point.color.y, point.color.z };
			j["point"]["position"] = { point.position.x, point.position.y, point.position.z };
			j["point"]["attenuation"] = { point.attenuation.x, point.attenuation.y, point.attenuation.z };
			j["point"]["hasShadowMaps"] = hasShadowMaps;
			if (hasShadowMaps) {
				j["point"]["pointShadowMap"]["shadowMapWidth"] = shadowMapParams["shadowMapWidth"];
				j["point"]["pointShadowMap"]["shadowMapHeight"] = shadowMapParams["shadowMapHeight"];
				j["point"]["pointShadowMap"]["nearZ"] = shadowMapParams["nearZ"];
				j["point"]["pointShadowMap"]["farZ"] = shadowMapParams["farZ"];
			}
		};

		switch (lightType) {
		case Ambient:
			buildJsonAmbientLight(j);
			break;
		case Directional:
			buildJsonDirectionalLight(j);
			break;
		case Spot:
			buildJsonSpotLight(j);
			break;
		case Point:
			buildJsonPointLight(j);
			break;
		}

		using namespace Animation::Effects;
		auto effects = GetLightEffects(this_ptr);
		if (!effects.empty()) {
			j["effects"] = effects;
		}

		return j;
	}
#endif
}