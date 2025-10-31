#include "pch.h"
#include "Light.h"
#include <Camera/Camera.h>
#include <Renderer.h>
#include <Scene.h>
#include <Renderable/Renderable.h>
#include <RenderPass/RenderPass.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#if defined(_EDITOR)
#include <RenderPass/Override/MinMaxChainPass.h>
#include <RenderPass/Override/MinMaxChainResultPass.h>
#endif

extern std::unique_ptr<Renderer> renderer;

namespace Scene
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE shadowMapSrvCpuDescriptorHandle[MaxLights]; //SRV CPU Handles for shadowmaps
	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapSrvGpuDescriptorHandle[MaxLights]; //SRV GPU Handles for shadowmaps
	std::set<unsigned int> usedShadowMapSlots;

	//CREATE
	void CreateShadowMapResources()
	{
		for (UINT i = 0; i < MaxLights; i++)
		{
			AllocCSUDescriptor(shadowMapSrvCpuDescriptorHandle[i], shadowMapSrvGpuDescriptorHandle[i]);
		}
	}

	void DestroyShadowMapResources()
	{
		for (UINT i = 0; i < MaxLights; i++)
		{
			FreeCSUDescriptor(shadowMapSrvCpuDescriptorHandle[i], shadowMapSrvGpuDescriptorHandle[i]);
		}
	}

	void AllocShadowMapSlot(unsigned int slot)
	{
		usedShadowMapSlots.insert(slot);
	}

	void FreeShadowMapSlot(unsigned int slot)
	{
		usedShadowMapSlots.erase(slot);
	}

	unsigned int GetNextAvailableShadowMapSlot()
	{
		for (unsigned int i = 0; i < MaxLights; i++)
		{
			if (usedShadowMapSlots.contains(i)) continue;
			return i;
		}
		assert(MaxLights != MaxLights);
		return 0xFFFFFFFF;
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandleStart()
	{
		return shadowMapSrvGpuDescriptorHandle[0];
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandle(unsigned int index)
	{
		return shadowMapSrvGpuDescriptorHandle[index];
	}

	void Light::CreateShadowMap()
	{
		switch (lightType())
		{
		case LT_Directional:
		{
			CreateDirectionalLightShadowMap();
		}
		break;
		case LT_Spot:
		{
			CreateSpotLightShadowMap();
		}
		break;
		case LT_Point:
		{
			CreatePointLightShadowMap();
		}
		break;
		default:
		{
			assert(lightType() != LT_Directional || lightType() != LT_Spot || lightType() != LT_Point);
		}
		break;
		}

		CreateShadowMapDepthStencilResource();
	}

	nlohmann::json Light::CreateDirectionalShadowMapCameraJson()
	{
		XMFLOAT3 rot = rotation();
		nlohmann::json j = {
			{ "uuid", uuid() + "-cam"},
			{ "hidden", true },
			{ "fitWindow", false },
			{ "name", name() + ".cam" },
			{ "projectionType", ProjectionsTypesToString.at(PROJ_Orthographic) },
			{ "orthographic", {
				{ "nearZ", nearZ() },
				{ "farZ", farZ() },
				{ "width", viewWidth() },
				{ "height", viewHeight() },
			}},
			{ "rotation", { rot.x, rot.y, rot.z}},
			{ "shadowMapLight", uuid()},
			{ "renderPasses", { GetRenderPassUUIDByName("ShadowMap") }}
		};
		return j;
	}

	void Light::CreateDirectionalLightShadowMap()
	{
		shadowMapScissorRect.clear();
		shadowMapViewport.clear();

		nlohmann::json camJ = CreateDirectionalShadowMapCameraJson();
		JUUID uuid = camJ.at("uuid");
		CreateCamera(camJ);
		auto& cam = GetCameraSceneObject(uuid);
		cam->BindToScene();
		XMVECTOR camPos = XMVectorScale(XMVector3Normalize(cam->forward()), -dirDist());
		cam->position(*(XMFLOAT3*)camPos.m128_f32);
		shadowMapCameras.push_back(uuid);
		UpdateShadowMapCameraProperties();
	}

	nlohmann::json Light::CreateSpotShadowMapCameraJson()
	{
		float spotDim = 2.0f * sinf(XMConvertToRadians(coneAngle())) * farZ();

		XMFLOAT3 pos = position();
		XMFLOAT3 rot = rotation();

		nlohmann::json j = {
			{ "uuid", uuid() + "-cam"},
			{ "hidden", true },
			{ "fitWindow", false },
			{ "name", name() + ".cam" },
			{ "projectionType", ProjectionsTypesToString.at(PROJ_Perspective) },
			{ "perspective", {
				{ "nearZ", nearZ() },
				{ "farZ", farZ() },
				{ "fovAngleY", coneAngle() * 2.0f },
				{ "width", spotDim },
				{ "height", spotDim }
			}},
			{ "position", { pos.x, pos.y, pos.z}},
			{ "rotation", { rot.x, rot.y, rot.z }},
			{ "shadowMapLight", uuid()},
			{ "renderPasses", { GetRenderPassUUIDByName("ShadowMap") }}
		};
		return j;
	}

	void Light::CreateSpotLightShadowMap()
	{
		shadowMapScissorRect.clear();
		shadowMapViewport.clear();

		nlohmann::json camJ = CreateSpotShadowMapCameraJson();
		JUUID uuid = camJ.at("uuid");
		CreateCamera(camJ);
		auto& cam = GetCameraSceneObject(uuid);
		cam->BindToScene();
		shadowMapCameras.push_back(uuid);
		UpdateShadowMapCameraProperties();
	}

	nlohmann::json Light::CreatePointShadowMapCameraJson(unsigned camIndex)
	{
		float fDim = static_cast<float>(shadowMapWidth());;
		XMFLOAT3 pos = position();

		nlohmann::json j = {
			{ "uuid", uuid() + "-cam-" + std::to_string(camIndex)},
			{ "hidden", true },
			{ "name", name() + ".cam." + std::to_string(camIndex)},
			{ "fitWindow", false },
			{ "projectionType", ProjectionsTypesToString.at(PROJ_Perspective) },
			{ "perspective",
			{
				{ "nearZ", nearZ() },
				{ "farZ", farZ() },
				{ "fovAngleY", 90.0f },
				{ "width", fDim },
				{ "height", fDim },
			}
			},
			{ "position", { pos.x, pos.y, pos.z}},
			{ "shadowMapLight", uuid()},
			{ "renderPasses", { GetRenderPassUUIDByName("ShadowMap") }}
		};
		return j;
	}

	void Light::CreatePointLightShadowMap()
	{
		shadowMapScissorRect.clear();
		shadowMapViewport.clear();

		for (unsigned int i = 0U; i < 6U; i++)
		{
			nlohmann::json camJ = CreatePointShadowMapCameraJson(i);
			JUUID uuid = camJ.at("uuid");
			CreateCamera(camJ);
			auto& cam = GetCameraSceneObject(uuid);
			cam->BindToScene();
			shadowMapCameras.push_back(uuid);
		}
		UpdateShadowMapCameraProperties();
	}

	void Light::UpdateShadowMapCameraProperties()
	{
		if (shadowMapCameras.empty()) return;

		switch (lightType())
		{
		case LT_Directional:
		{
			UpdateDirectionalShadowMapCameraProperties();
		}
		break;
		case LT_Spot:
		{
			UpdateSpotShadowMapCameraProperties();
		}
		break;
		case LT_Point:
		{
			UpdatePointShadowMapCameraProperties();
		}
		break;
		default:
		{
			assert(lightType() != LT_Directional || lightType() != LT_Spot || lightType() != LT_Point);
		}
		break;
		}
	}

	void Light::UpdateDirectionalShadowMapCameraProperties()
	{
		shadowMapScissorRect.push_back({ 0, 0, static_cast<long>(shadowMapWidth()), static_cast<long>(shadowMapHeight()) });
		shadowMapViewport.push_back({ 0.0f , 0.0f, static_cast<float>(shadowMapWidth()), static_cast<float>(shadowMapHeight()), 0.0f, 1.0f });
		shadowMapTexelInvSize = { 1.0f / static_cast<float>(shadowMapWidth()), 1.0f / static_cast<float>(shadowMapHeight()) };
		auto& cam = shadowMapCameras.at(0);
		cam->orthographicProjection.updateProjectionMatrix(viewWidth(), viewHeight());
	}

	void Light::UpdateSpotShadowMapCameraProperties()
	{
		float spotDim = 2.0f * sinf(XMConvertToRadians(coneAngle())) * farZ();
		shadowMapScissorRect.push_back({ 0, 0, static_cast<long>(shadowMapWidth()), static_cast<long>(shadowMapHeight()) });
		shadowMapViewport.push_back({ 0.0f , 0.0f, static_cast<float>(shadowMapWidth()), static_cast<float>(shadowMapHeight()), 0.0f, 1.0f });
		shadowMapTexelInvSize = { 1.0f / static_cast<float>(shadowMapWidth()), 1.0f / static_cast<float>(shadowMapHeight()) };
		auto& cam = shadowMapCameras.at(0);
		cam->perspectiveProjection.fovAngleY = coneAngle() * 2.0f;
		cam->perspectiveProjection.updateProjectionMatrix(spotDim, spotDim);
	}

	void Light::UpdatePointShadowMapCameraProperties()
	{
		float fDim = static_cast<float>(shadowMapWidth());;
		long lDim = static_cast<long>(shadowMapWidth());;
		XMFLOAT3 pos = position();

		shadowMapScissorRect.clear();
		shadowMapViewport.clear();
		for (unsigned int i = 0U; i < 6U; i++)
		{
			shadowMapScissorRect.push_back({ 0, static_cast<long>(i) * lDim, lDim, static_cast<long>(i + 1U) * lDim });
			shadowMapViewport.push_back({ 0.0f, static_cast<float>(i) * fDim, fDim, fDim, 0.0f, 1.0f });
		}

		for (unsigned int i = 0U; i < 6U; i++)
		{
			auto& cam = shadowMapCameras.at(i);
			cam->perspectiveProjection.updateProjectionMatrix(fDim, fDim);
		}
	}

	void Light::UpdateShadowMapCameraTransformation()
	{
		if (shadowMapCameras.empty()) return;

		switch (lightType())
		{
		case LT_Directional:
		{
			UpdateDirectionalShadowMapCameraTransformation();
		}
		break;
		case LT_Spot:
		{
			UpdateSpotShadowMapCameraTransformation();
		}
		break;
		case LT_Point:
		{
			UpdatePointShadowMapCameraTransformation();
		}
		break;
		default:
		{
			assert(lightType() != LT_Directional || lightType() != LT_Spot || lightType() != LT_Point);
		}
		break;
		}
	}

	void Light::UpdateDirectionalShadowMapCameraTransformation()
	{
		auto& cam = shadowMapCameras.at(0);
		XMVECTOR camPos = XMVectorScale(XMVector3Normalize(cam->forward()), -dirDist());
		cam->position(*(XMFLOAT3*)camPos.m128_f32);
		cam->rotation(rotation());
	}

	void Light::UpdateSpotShadowMapCameraTransformation()
	{
		auto& cam = shadowMapCameras.at(0);
		cam->rotation(rotation());
		cam->position(position());
	}

	void Light::UpdatePointShadowMapCameraTransformation()
	{
		XMFLOAT3 pos = position();
		std::for_each(shadowMapCameras.begin(), shadowMapCameras.end(), [&pos](CameraUUID cam)
			{
				cam->position(pos);
			}
		);
	}

	void Light::CreateShadowMapDepthStencilResource()
	{
		unsigned int w = shadowMapWidth();
		unsigned int h = ((lightType() == LT_Point) ? 6U * shadowMapWidth() : shadowMapHeight());
		shadowMapRenderPass = CreateRenderToTexturePass(name() + "->shadowMap", {}, DXGI_FORMAT_D32_FLOAT, w, h);

		shadowMapIndex = GetNextAvailableShadowMapSlot();
		AllocShadowMapSlot(shadowMapIndex);
		CreateShadowMapShaderResourceView();
	}

	void Light::CreateShadowMapShaderResourceView()
	{
		auto& smPass = shadowMapRenderPass;
		D3D12_SHADER_RESOURCE_VIEW_DESC shadowMapSrvDesc = {
			.Format = Renderer::depthFormatSRVConversion.contains(smPass->depthStencilFormat) ? Renderer::depthFormatSRVConversion.at(smPass->depthStencilFormat) : smPass->depthStencilFormat,
			.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Texture2D = {.MostDetailedMip = 0, .MipLevels = 1U, .ResourceMinLODClamp = 0.0f },
		};
		renderer->d3dDevice->CreateShaderResourceView(smPass->depthStencilTexture, &shadowMapSrvDesc, shadowMapSrvCpuDescriptorHandle[shadowMapIndex]);
	}

#if defined(_EDITOR)

	void Light::CreateShadowMapMinMaxChain()
	{
		//pick the gpu handles for the final shadowmap and copies for the min/max chain initial calculation
		CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle = GetShadowMapGpuDescriptorHandle(shadowMapIndex);
		CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle1 = shadowMapChainGpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle2 = shadowMapChainGpuHandle;

		float texWidth = static_cast<float>(shadowMapWidth());
		float texHeight = static_cast<float>(((lightType() == LT_Point) ? 6U * shadowMapWidth() : shadowMapHeight()));

		//calculate the width/height of the texture and the TexelInvSize of the shadow map texture for the current pass
		unsigned int width = static_cast<unsigned int>(texWidth) >> 1;
		unsigned int height = static_cast<unsigned int>(texHeight) >> 1;

		unsigned int renderPassIndex = 0;
		unsigned int chainIndex = 0;
		do
		{
			shadowMapMinMaxChainRenderPass.push_back(
				CreateRenderPassInstance("", GetRenderPassUUIDByName("ShadowMapMinMaxChainPass"), renderPassIndex, max(2U, width), max(2U, height))
			);
			auto& rpInstance = shadowMapMinMaxChainRenderPass.back();
			MinMaxChainPass* chainPass = static_cast<MinMaxChainPass*>(rpInstance->overridePass.get());
			chainPass->shadowMapChainGpuHandle1 = shadowMapChainGpuHandle1;
			chainPass->shadowMapChainGpuHandle2 = shadowMapChainGpuHandle2;

			auto& rtt0 = chainPass->renderPassInstance->renderToTexturePass->renderToTexture.at(0);
			auto& rtt1 = chainPass->renderPassInstance->renderToTexturePass->renderToTexture.at(1);

			rtt0->renderToTexture->SetName(nostd::StringToWString(std::string(name() + "->smC[" + std::to_string(chainIndex) + "][0]")).c_str());
			rtt1->renderToTexture->SetName(nostd::StringToWString(std::string(name() + "->smC[" + std::to_string(chainIndex) + "][1]")).c_str());

			shadowMapChainGpuHandle1 = rtt0->gpuTextureHandle;
			shadowMapChainGpuHandle2 = rtt1->gpuTextureHandle;

			//calculate the next width and height
			width = max(1U, width >> 1);
			height = max(1U, height >> 1);
			chainIndex++;
		} while (width != 1U || height != 1U);

		unsigned int texUWidth = 512U;
		unsigned int texUHeight = 512U * ((lightType() == LT_Point) ? 6U : 1U);
		shadowMapMinMaxChainResultRenderPass = CreateRenderPassInstance(
			"", GetRenderPassUUIDByName("ShadowMapMinMaxChainResultPass"), 0, texUWidth, texUHeight);

		MinMaxChainResultPass* resultPass = static_cast<MinMaxChainResultPass*>(shadowMapMinMaxChainResultRenderPass->overridePass.get());

		RenderPassInstanceUUID last = shadowMapMinMaxChainRenderPass.back();
		resultPass->depthGpuHandle = shadowMapChainGpuHandle;
		resultPass->shadowMapChainGpuHandle1 = last->renderToTexturePass->renderToTexture.at(0)->gpuTextureHandle;
		resultPass->shadowMapChainGpuHandle2 = last->renderToTexturePass->renderToTexture.at(1)->gpuTextureHandle;
		resultPass->CreateFSQuad((lightType() != LT_Spot) ? "DepthMinMaxToRGBA" : "DepthMinMaxToRGBASpot");
	}

	void Light::DestroyShadowMapMinMaxChain()
	{
		DestroyRenderPassInstance(shadowMapMinMaxChainResultRenderPass());
		for (auto& rp : shadowMapMinMaxChainRenderPass)
		{
			DestroyRenderPassInstance(rp());
		}
		shadowMapMinMaxChainRenderPass.clear();
	}

	void RenderShadowMapMinMaxChain()
	{
		auto& lights = GetLightsSceneObjects();
		std::for_each(lights.begin(), lights.end(), [](auto& light)
			{
				auto& tup = std::get<1>(light);
				auto& l = std::get<1>(tup);
				if (l->shadowMapMinMaxChainRenderPass.empty()) return;
				l->RenderShadowMapMinMaxChain();
			}
		);
	}

	void Light::RenderShadowMapMinMaxChain()
	{
		for (auto& rpi : shadowMapMinMaxChainRenderPass)
		{
			rpi->Pass();
		}
		shadowMapMinMaxChainResultRenderPass->Pass();
	}

	std::function<bool(JObject*)> Light::GetAssetsConditioner()
	{
		return [this](JObject* j)
			{
				SceneObject* so = static_cast<SceneObject*>(j);
				switch (so->JType())
				{
				case SO_Lights:
				{
					std::set<std::tuple<LightType, LightType>> compatibility = {
						std::make_tuple(LightType::LT_Spot,LightType::LT_Point),
						std::make_tuple(LightType::LT_Point,LightType::LT_Spot)
					};
					Light* l = static_cast<Light*>(so);
					return compatibility.contains(std::make_tuple(l->lightType(), lightType()));
				}
				break;
				}
				return true;
			};
	}
#endif

	void Light::BindRenderablesToShadowMapCamera()
	{
		for (RenderableUUID r : GetShadowCasts())
		{
			auto rvCams = r->cameras();
			auto lvCams = cameras();
			std::set<CameraUUID> rCams(rvCams.begin(), rvCams.end());
			std::set<CameraUUID> lCams(lvCams.begin(), lvCams.end());
			bool mkbind = std::any_of(rCams.begin(), rCams.end(), [&lCams](auto uuid)
				{
					return lCams.contains(uuid);
				}
			);
			if (!mkbind)
				continue;

			for (auto& cuuid : shadowMapCameras)
			{
				Scene::BindToScene(cuuid(), r());
			}
		}
	}

	void Light::UnbindRenderablesFromShadowMapCameras()
	{
		for (auto& r : GetShadowCasts())
		{
			for (auto& cuuid : shadowMapCameras)
			{
				Scene::UnbindFromScene(cuuid(), r);
			}
		}
	}


	void Light::WriteConstantsBufferShadowMapAttributes(ShadowMapAttributes& atts)
	{
		ZeroMemory(&atts, sizeof(atts));

		switch (lightType()) {
		case LT_Directional:
		{
			auto& cam = shadowMapCameras[0];
			XMMATRIX view = cam->view();
			XMMATRIX projection = cam->orthographicProjection.projectionMatrix;
			atts.atts0 = XMMatrixMultiply(view, projection);
			atts.atts6 = { zBias(), shadowMapTexelInvSize.x, shadowMapTexelInvSize.y, 0.0f }; //ZBias, TexelInvSize
		}
		break;
		case LT_Spot:
		{
			auto& cam = shadowMapCameras[0];
			XMMATRIX view = cam->view();
			XMMATRIX projection = cam->perspectiveProjection.projectionMatrix;
			atts.atts0 = XMMatrixMultiply(view, projection);
			atts.atts6 = { zBias(), shadowMapTexelInvSize.x, shadowMapTexelInvSize.y, 0.0f }; //ZBias, TexelInvSize
		}
		break;
		case LT_Point:
		{
			XMMATRIX* attsN = &atts.atts0;
			for (UINT i = 0U; i < 6U; i++) {
				auto& cam = shadowMapCameras[i];
				XMMATRIX view = cam->view();
				XMMATRIX projection = cam->perspectiveProjection.projectionMatrix;
				*attsN = XMMatrixMultiply(view, projection);
				attsN++;
			}
			atts.atts6 = { zBias(), 3.0f, 0.0f, 0.0f }; //ZBias, PartialDerivativeScale
		}
		break;
		default:
		{
			assert(lightType() != LT_Directional || lightType() != LT_Spot || lightType() != LT_Point);
		}
		break;
		}
	}

	//DESTROY
	void Light::DestroyShadowMap()
	{
		DestroyShadowMapCameras();

		unsigned int numShadowMaps = static_cast<unsigned int>(usedShadowMapSlots.size());
		FreeShadowMapSlot(shadowMapIndex);

		//this is tricky hence worth of explaining, if there are more than one shadowmap we need to shift left the CPU/GPU handles representing the textures adresses
		if (numShadowMaps > 1)
		{
			//get how many lights are needed to shift left
			unsigned int numHandlesToShift = numShadowMaps - shadowMapIndex - 1;
			unsigned int shiftFrom = shadowMapIndex + 1;
			unsigned int shiftTo = shadowMapIndex;

			if (numHandlesToShift > 0)
			{
				//reasign the index of each light which shadow map index is greather than the this light shadow map index to a one less index value
				std::set<LightUUID> lightsToShift;
				for (CameraUUID cam : cameras())
				{
					for (LightUUID light : cam->lightsWithShadowMaps)
					{
						if (light->shadowMapIndex <= shadowMapIndex || light->shadowMapIndex == 0 || light->shadowMapIndex == 0xFFFFFFFF) continue;
						lightsToShift.insert(light);
					}
				}
				for (LightUUID light : lightsToShift)
				{
					light->shadowMapIndex--;
					light->CreateShadowMapShaderResourceView();
				}

				AllocShadowMapSlot(shadowMapIndex);
				FreeShadowMapSlot(numShadowMaps - 1);
			}
		}

		//destroy the render pass
		DeleteRenderToTexturePass(shadowMapRenderPass());
		//and make the shadowmap index FFF...
		shadowMapIndex = 0xFFFFFFFF;
	}

	void Light::DestroyShadowMapCameras()
	{
		for (auto cam : shadowMapCameras)
		{
			cam->DestroyRenderPasses();
			DestroyConstantsBuffer(cam->cameraCb());
			DeleteCameraSceneObject(cam());
		}
		shadowMapCameras.clear();
	}

	//RENDER
	std::set<LightType> onePassTypes = { LT_Directional, LT_Spot };
	void Light::RenderShadowMap(std::function<void(unsigned int)> renderScene)
	{
		auto& commandList = renderer->commandList;
		auto& smPass = shadowMapRenderPass;

		smPass->Pass([this, renderScene, &commandList]
			{
				if (onePassTypes.contains(lightType()))
				{
					renderScene(0);
				}
				else
				{
					for (unsigned int i = 0; i < 6; i++)
					{
						commandList->RSSetViewports(1, &shadowMapViewport.at(i));
						commandList->RSSetScissorRects(1, &shadowMapScissorRect.at(i));
						renderScene(i);
					}
				}
			}
		);
	}

}