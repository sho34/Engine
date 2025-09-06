#include "pch.h"
#include "Lights.h"
#include <Camera/Camera.h>
#include <d3dx12.h>
#include <DirectXHelper.h>
#include <Renderer.h>
#include <RenderPass/RenderToTexturePass.h>
#include <DeviceUtils/D3D12Device/Builder.h>
#include <DeviceUtils/D3D12Device/Interop.h>
#include <DeviceUtils/RootSignature/RootSignature.h>
#include <DeviceUtils/PipelineState/PipelineState.h>
#if defined(_EDITOR)
#include <Editor.h>
#include <RenderPass/Override/MinMaxChainPass.h>
#include <RenderPass/Override/MinMaxChainResultPass.h>
#endif

extern std::shared_ptr<Renderer> renderer;

namespace Scene {

	using namespace DeviceUtils;
#if defined(_EDITOR)
	using namespace Editor;
#endif

	std::shared_ptr<ConstantsBuffer> shadowMapsCbv; //CBV for shadowmaps
	std::vector<std::shared_ptr<Light>> lightsWithShadowMaps;
	CD3DX12_CPU_DESCRIPTOR_HANDLE shadowMapSrvCpuDescriptorHandle[MaxLights]; //SRV CPU Handles for shadowmaps
	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapSrvGpuDescriptorHandle[MaxLights]; //SRV GPU Handles for shadowmaps
	std::set<unsigned int> usedShadowMapSlots;

	//CREATE
	void CreateShadowMapResources()
	{
		shadowMapsCbv = CreateConstantsBuffer(sizeof(ShadowMapAttributes) * MaxLights, "shadowMapsCbv");

		for (UINT i = 0; i < MaxLights; i++)
		{
			AllocCSUDescriptor(shadowMapSrvCpuDescriptorHandle[i], shadowMapSrvGpuDescriptorHandle[i]);
		}
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

		assert(this_ptr != nullptr);
		lightsWithShadowMaps.push_back(this_ptr);
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
			{ "light", uuid()},
			{ "renderPasses", { FindRenderPassUUIDByName("ShadowMap") }}
		};
		return j;
	}

	void Light::CreateDirectionalLightShadowMap()
	{
		shadowMapScissorRect.clear();
		shadowMapViewport.clear();

		std::shared_ptr<Camera> cam = std::make_shared<Camera>(CreateDirectionalShadowMapCameraJson());
		cam->this_ptr = cam;
		cam->Initialize();
		cam->BindToScene();
		XMVECTOR camPos = XMVectorScale(XMVector3Normalize(cam->forward()), -dirDist());
		cam->position(*(XMFLOAT3*)camPos.m128_f32);
		shadowMapCameras.push_back(cam);
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
			{ "light", uuid()},
			{ "renderPasses", { FindRenderPassUUIDByName("ShadowMap") }}
		};
		return j;
	}

	void Light::CreateSpotLightShadowMap()
	{
		shadowMapScissorRect.clear();
		shadowMapViewport.clear();

		std::shared_ptr<Camera> cam = std::make_shared<Camera>(CreateSpotShadowMapCameraJson());
		cam->this_ptr = cam;
		cam->Initialize();
		cam->BindToScene();
		shadowMapCameras.push_back(cam);
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
			{ "light", uuid()},
			{ "renderPasses", { FindRenderPassUUIDByName("ShadowMap") }}
		};
		return j;
	}

	void Light::CreatePointLightShadowMap()
	{
		shadowMapScissorRect.clear();
		shadowMapViewport.clear();

		for (unsigned int i = 0U; i < 6U; i++)
		{
			std::shared_ptr<Camera> cam = std::make_shared<Camera>(CreatePointShadowMapCameraJson(i));
			cam->this_ptr = cam;
			cam->Initialize();
			cam->BindToScene();
			shadowMapCameras.push_back(cam);
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
		shadowMapCameras.at(0)->orthographicProjection.updateProjectionMatrix(viewWidth(), viewHeight());
	}

	void Light::UpdateSpotShadowMapCameraProperties()
	{
		float spotDim = 2.0f * sinf(XMConvertToRadians(coneAngle())) * farZ();
		shadowMapScissorRect.push_back({ 0, 0, static_cast<long>(shadowMapWidth()), static_cast<long>(shadowMapHeight()) });
		shadowMapViewport.push_back({ 0.0f , 0.0f, static_cast<float>(shadowMapWidth()), static_cast<float>(shadowMapHeight()), 0.0f, 1.0f });
		shadowMapTexelInvSize = { 1.0f / static_cast<float>(shadowMapWidth()), 1.0f / static_cast<float>(shadowMapHeight()) };
		shadowMapCameras.at(0)->perspectiveProjection.fovAngleY = coneAngle() * 2.0f;
		shadowMapCameras.at(0)->perspectiveProjection.updateProjectionMatrix(spotDim, spotDim);
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
			shadowMapCameras.at(i)->perspectiveProjection.updateProjectionMatrix(fDim, fDim);
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
		auto cam = shadowMapCameras.at(0);
		XMVECTOR camPos = XMVectorScale(XMVector3Normalize(cam->forward()), -dirDist());
		cam->position(*(XMFLOAT3*)camPos.m128_f32);
		cam->rotation(rotation());
	}

	void Light::UpdateSpotShadowMapCameraTransformation()
	{
		shadowMapCameras.at(0)->rotation(rotation());
		shadowMapCameras.at(0)->position(position());
	}

	void Light::UpdatePointShadowMapCameraTransformation()
	{
		XMFLOAT3 pos = position();
		std::for_each(shadowMapCameras.begin(), shadowMapCameras.end(), [&pos](auto& cam)
			{
				cam->position(pos);
			}
		);
	}

	void Light::CreateShadowMapDepthStencilResource()
	{
		unsigned int w = shadowMapWidth();
		unsigned int h = ((lightType() == LT_Point) ? 6U * shadowMapWidth() : shadowMapHeight());
		shadowMapRenderPass = CreateRenderPass(name() + "->shadowMap", {}, DXGI_FORMAT_D32_FLOAT, w, h);

		shadowMapIndex = GetNextAvailableShadowMapSlot();
		AllocShadowMapSlot(shadowMapIndex);
		CreateShadowMapShaderResourceView();
	}

	void Light::CreateShadowMapShaderResourceView()
	{
		renderer->d3dDevice->CreateShaderResourceView(shadowMapRenderPass->depthStencilTexture, &shadowMapSrvDesc, shadowMapSrvCpuDescriptorHandle[shadowMapIndex]);
	}

#if defined(_EDITOR)
	void Light::EditorPreview(size_t flags)
	{
		if (flags & (1 << Light::Update_hasShadowMaps))
		{
			if (hasShadowMaps())
				CreateShadowMapMinMaxChain();
		}
	}

	void Light::DestroyEditorPreview()
	{
		DestroyShadowMapMinMaxChain();
	}

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
		do
		{
			shadowMapMinMaxChainRenderPass.push_back(
				GetRenderPassInstance(nullptr, renderPassIndex, FindRenderPassUUIDByName("ShadowMapMinMaxChainPass"), max(2U, width), max(2U, height))
			);
			auto& rpInstance = shadowMapMinMaxChainRenderPass.back();
			std::shared_ptr<MinMaxChainPass> chainPass = std::dynamic_pointer_cast<MinMaxChainPass>(rpInstance->overridePass);
			chainPass->shadowMapChainGpuHandle1 = shadowMapChainGpuHandle1;
			chainPass->shadowMapChainGpuHandle2 = shadowMapChainGpuHandle2;

			shadowMapChainGpuHandle1 = chainPass->renderPassInstance->rendererToTexturePass->renderToTexture.at(0)->gpuTextureHandle;
			shadowMapChainGpuHandle2 = chainPass->renderPassInstance->rendererToTexturePass->renderToTexture.at(1)->gpuTextureHandle;

			//rpInstance->overridePass->prevPassRTT = prevPassRTT;
			//prevPassRTT = rpInstance->rendererToTexturePass->renderToTexture.at(0);
			//push a render pass for the current chain depth
			/*
			std::string ShadowMinMaxChainRenderPassName = "ShadowMapMinMaxChainRenderPass[" + std::to_string(max(2U, width)) + "," + std::to_string(max(2U, height)) + "]";
			OutputDebugStringA((ShadowMinMaxChainRenderPassName + "\n").c_str());

			shadowMapMinMaxChainRenderPass.push_back(
				CreateRenderPass(
					ShadowMinMaxChainRenderPassName,
					{ DXGI_FORMAT_R32_FLOAT , DXGI_FORMAT_R32_FLOAT },
					DXGI_FORMAT_UNKNOWN,
					max(2U, width),
					max(2U, height)
				)
			);
			*/
			/*
			//push a renderable for the current
			shadowMapMinMaxChainRenderable.push_back(
				CreateRenderable(
					{
						{ "meshMaterials" ,
							{
								{
									{ "material", FindMaterialUUIDByName("DepthMinMax") },
									{ "mesh", FindMeshUUIDByName("decal") }
								}
							}
						},
						{ "uniqueMaterialInstance", true },
						{ "meshShadowMapMaterials", {} },
						{ "name", "ShadowMapMinMaxChainQuad" },
						{ "uuid", getUUID() },
						{ "hidden", true},
						{ "visible", false},
						{ "position", {0.0, 0.0, 0.0} },
						{ "rotation",{0.0, 0.0, 0.0 } },
						{ "scale", { 1.0, 1.0, 1.0} },
						{ "skipMeshes", {} },
						{ "pipelineState" ,
							{
								{ "renderTargetsFormats", { "R32_FLOAT","R32_FLOAT" } },
								{ "depthStencilFormat" , "UNKNOWN" }
							}
						}
					}
				)
			);
			*/
			/*
			//write the TexelInvSize to constants buffers
			for (unsigned int i = 0; i < renderer->numFrames; i++)
			{
				shadowMapMinMaxChainRenderable.back()->WriteConstantsBuffer("texelInvSize", TexelInvSize, i);
			}
			*/
			/*
			//get the material instance of the renderable and push the gpu handles 1&2 of the chain
			std::shared_ptr<MaterialInstance>& shadowMapMinMaxChainMaterial = shadowMapMinMaxChainRenderable.back()->meshMaterials.begin()->second;

			std::string ShadowMapMinMaxChainMat1 = "ShadowMapMinMaxChainMat1[" + std::to_string(max(2U, width)) + "," + std::to_string(max(2U, height)) + "]";
			std::string ShadowMapMinMaxChainMat2 = "ShadowMapMinMaxChainMat2[" + std::to_string(max(2U, width)) + "," + std::to_string(max(2U, height)) + "]";
			shadowMapMinMaxChainMaterial->textures.insert_or_assign(TextureShaderUsage_MinTexture, GetTextureFromGPUHandle(ShadowMapMinMaxChainMat1, shadowMapChainGpuHandle1));
			shadowMapMinMaxChainMaterial->textures.insert_or_assign(TextureShaderUsage_MaxTexture, GetTextureFromGPUHandle(ShadowMapMinMaxChainMat2, shadowMapChainGpuHandle1));

			//get the new gpu handles 1&2 for the next chain
			std::shared_ptr<RenderToTexturePass>& pass = shadowMapMinMaxChainRenderPass.back();
			shadowMapChainGpuHandle1 = pass->renderToTexture[0]->gpuTextureHandle;
			shadowMapChainGpuHandle2 = pass->renderToTexture[1]->gpuTextureHandle;

			//calculate the next TexelInvSize
			*/

			//calculate the next width and height
			width = max(1U, width >> 1);
			height = max(1U, height >> 1);
		} while (width != 1U || height != 1U);

		unsigned int texUWidth = 512U;
		unsigned int texUHeight = 512U * ((lightType() == LT_Point) ? 6U : 1U);
		shadowMapMinMaxChainResultRenderPass = GetRenderPassInstance(
			nullptr, 0, FindRenderPassUUIDByName("ShadowMapMinMaxChainResultPass"), texUWidth, texUHeight);

		std::shared_ptr<MinMaxChainResultPass> resultPass = std::dynamic_pointer_cast<MinMaxChainResultPass>(shadowMapMinMaxChainResultRenderPass->overridePass);

		std::shared_ptr<RenderPassInstance> last = shadowMapMinMaxChainRenderPass.back();
		resultPass->depthGpuHandle = shadowMapChainGpuHandle;
		resultPass->shadowMapChainGpuHandle1 = last->rendererToTexturePass->renderToTexture.at(0)->gpuTextureHandle;
		resultPass->shadowMapChainGpuHandle2 = last->rendererToTexturePass->renderToTexture.at(1)->gpuTextureHandle;
		resultPass->CreateFSQuad((lightType() != LT_Spot) ? "DepthMinMaxToRGBA" : "DepthMinMaxToRGBASpot");

		/*
		//create the end result render pass
		shadowMapMinMaxChainResultRenderPass = CreateRenderPass(
			"ShadowMinMaxChainRenderPassResult",
			{ DXGI_FORMAT_R8G8B8A8_UNORM },
			DXGI_FORMAT_UNKNOWN,
			static_cast<unsigned int>(texWidth),
			static_cast<unsigned int>(texHeight)
		);
		*/

		//create the end result renderable
		/*
		shadowMapMinMaxChainResultRenderable = CreateRenderable(
			{
				{ "meshMaterials" ,
					{
						{
							{ "material", (lightType() != LT_Spot) ? FindMaterialUUIDByName("DepthMinMaxToRGBA") : FindMaterialUUIDByName("DepthMinMaxToRGBASpot") },
							{ "mesh", FindMeshUUIDByName("decal") }
						}
					}
				},
				{ "uniqueMaterialInstance", true },
				{ "meshShadowMapMaterials", {} },
				{ "name", "ShadowMapMinMaxChainQuadResult" },
				{ "uuid", getUUID() },
				{ "hidden", true },
				{ "visible", false },
				{ "position", { 0.0, 0.0, 0.0 } },
				{ "rotation", { 0.0, 0.0, 0.0 } },
				{ "scale", { 1.0, 1.0, 1.0 } } ,
				{ "skipMeshes", {} },
				{ "pipelineState" ,
					{
						{ "renderTargetsFormats", { "R8G8B8A8_UNORM" } },
						{ "depthStencilFormat", "UNKNOWN" }
					}
				}
			}
		);
		*/
		/*
		std::shared_ptr<RenderToTexturePass>& lastMinMaxPass = shadowMapMinMaxChainRenderPass.back();
		std::shared_ptr<MaterialInstance>& shadowMapMinMaxChainResultMaterial = shadowMapMinMaxChainResultRenderable->meshMaterials.begin()->second;

		std::string ShadowMapResultChainMat1 = "ShadowMapMinMaxResult1";
		std::string ShadowMapResultChainMat2 = "ShadowMapMinMaxResult2";
		std::string ShadowMapResultChainMat3 = "ShadowMapMinMaxResult3";
		shadowMapMinMaxChainResultMaterial->textures.insert_or_assign(TextureShaderUsage_DepthTexture, GetTextureFromGPUHandle(ShadowMapResultChainMat1, shadowMapChainGpuHandle));
		shadowMapMinMaxChainResultMaterial->textures.insert_or_assign(TextureShaderUsage_MinTexture, GetTextureFromGPUHandle(ShadowMapResultChainMat2, lastMinMaxPass->renderToTexture[0]->gpuTextureHandle));
		shadowMapMinMaxChainResultMaterial->textures.insert_or_assign(TextureShaderUsage_MaxTexture, GetTextureFromGPUHandle(ShadowMapResultChainMat3, lastMinMaxPass->renderToTexture[1]->gpuTextureHandle));
		*/
	}

	void Light::DestroyShadowMapMinMaxChain()
	{
		DestroyRenderPassInstance(shadowMapMinMaxChainResultRenderPass);
		shadowMapMinMaxChainResultRenderPass = nullptr;
		for (auto& rp : shadowMapMinMaxChainRenderPass)
		{
			DestroyRenderPassInstance(rp);
			rp = nullptr;
		}
		shadowMapMinMaxChainRenderPass.clear();
	}

	void RenderShadowMapMinMaxChain()
	{
		std::vector<std::shared_ptr<Light>> lights = GetLights();
		std::for_each(lights.begin(), lights.end(), [](auto& l)
			{
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
#endif

	void Light::BindRenderablesToShadowMapCamera()
	{
		for (auto& r : GetShadowCasts())
		{
			for (auto& cam : shadowMapCameras)
			{
				Scene::BindToScene(cam, r);
			}
		}
	}

	void Light::UnbindRenderablesFromShadowMapCameras()
	{
		for (auto& r : GetShadowCasts())
		{
			for (auto& cam : shadowMapCameras)
			{
				Scene::UnbindFromScene(cam, r);
			}
		}
	}

	bool SceneHasShadowMaps()
	{
		return !lightsWithShadowMaps.empty();
	}

	//READ&GET
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

	std::shared_ptr<ConstantsBuffer> GetShadowMapConstantsBuffer()
	{
		return shadowMapsCbv;
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandleStart()
	{
		return shadowMapSrvGpuDescriptorHandle[0];
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetShadowMapGpuDescriptorHandle(unsigned int index)
	{
		return shadowMapSrvGpuDescriptorHandle[index];
	}

	//UPDATE
	void AllocShadowMapSlot(unsigned int slot)
	{
		usedShadowMapSlots.insert(slot);
	}

	void FreeShadowMapSlot(unsigned int slot)
	{
		usedShadowMapSlots.erase(slot);
	}

	void WriteConstantsBufferShadowMapAttributes(const std::shared_ptr<Light>& light, unsigned int backbufferIndex, unsigned int shadowMapIndex)
	{
		if (!light->hasShadowMaps()) return;

		ShadowMapAttributes atts{};
		ZeroMemory(&atts, sizeof(atts));

		switch (light->lightType()) {
		case LT_Directional:
		{
			XMMATRIX view = light->shadowMapCameras[0]->view();
			XMMATRIX projection = light->shadowMapCameras[0]->orthographicProjection.projectionMatrix;
			atts.atts0 = XMMatrixMultiply(view, projection);
			atts.atts6 = { light->zBias(), light->shadowMapTexelInvSize.x, light->shadowMapTexelInvSize.y, 0.0f }; //ZBias, TexelInvSize
		}
		break;
		case LT_Spot:
		{
			XMMATRIX view = light->shadowMapCameras[0]->view();
			XMMATRIX projection = light->shadowMapCameras[0]->perspectiveProjection.projectionMatrix;
			atts.atts0 = XMMatrixMultiply(view, projection);
			atts.atts6 = { light->zBias(), light->shadowMapTexelInvSize.x, light->shadowMapTexelInvSize.y, 0.0f }; //ZBias, TexelInvSize
		}
		break;
		case LT_Point:
		{
			XMMATRIX* attsN = &atts.atts0;
			for (UINT i = 0U; i < 6U; i++) {
				XMMATRIX view = light->shadowMapCameras[i]->view();
				XMMATRIX projection = light->shadowMapCameras[i]->perspectiveProjection.projectionMatrix;
				*attsN = XMMatrixMultiply(view, projection);
				attsN++;
			}
			atts.atts6 = { light->zBias(), 3.0f, 0.0f, 0.0f }; //ZBias, PartialDerivativeScale
		}
		break;
		default:
		{
			assert(light->lightType() != LT_Directional || light->lightType() != LT_Spot || light->lightType() != LT_Point);
		}
		break;
		}

		size_t offset = shadowMapsCbv->alignedConstantBufferSize * backbufferIndex;
		offset += sizeof(ShadowMapAttributes) * shadowMapIndex;
		memcpy(shadowMapsCbv->mappedConstantBuffer + offset, &atts, sizeof(atts));
	}

	void WriteShadowMapCamerasConstantsBuffers(const std::shared_ptr<Light>& light, unsigned int backbufferIndex)
	{
		if (!light->hasShadowMaps()) return;

		for (auto& cam : light->shadowMapCameras)
		{
			cam->WriteConstantsBuffer(backbufferIndex);
		}
	}

	void ResetConstantsBufferShadowMapAttributes(unsigned int backbufferIndex)
	{
		size_t offset = shadowMapsCbv->alignedConstantBufferSize * backbufferIndex;
		ShadowMapAttributes* atts = (ShadowMapAttributes*)(shadowMapsCbv->mappedConstantBuffer + offset);
		ZeroMemory(atts, sizeof(ShadowMapAttributes) * MaxLights);
	}

	//DESTROY
	void Light::DestroyShadowMap()
	{
		DestroyShadowMapCameras();

		unsigned int numShadowMaps = static_cast<unsigned int>(lightsWithShadowMaps.size());
		nostd::vector_erase(lightsWithShadowMaps, this_ptr);
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
				//also reasign the index of each light which shadow map index is greather than the this light shadow map index to a one less index value
				for (auto& light : lightsWithShadowMaps)
				{
					if (light->shadowMapIndex <= shadowMapIndex) continue;
					light->shadowMapIndex--;
					light->CreateShadowMapShaderResourceView();
				}

				AllocShadowMapSlot(shadowMapIndex);
				FreeShadowMapSlot(numShadowMaps - 1);
			}
		}

		//destroy the render pass
		shadowMapRenderPass = nullptr;
		//and make the shadowmap index FFF...
		shadowMapIndex = 0xFFFFFFFF;
	}

	void Light::DestroyShadowMapCameras()
	{
		for (auto& cam : shadowMapCameras)
		{
			SafeDeleteSceneObject(cam);
		}
		shadowMapCameras.clear();
	}

	//RENDER
	std::set<LightType> onePassTypes = { LT_Directional, LT_Spot };
	void Light::RenderShadowMap(std::function<void(unsigned int)> renderScene)
	{
		auto& commandList = renderer->commandList;

		shadowMapRenderPass->Pass([this, renderScene, &commandList]
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

	void DestroyShadowMaps()
	{
		lightsWithShadowMaps.clear();
		usedShadowMapSlots.clear();
	}

	void DestroyShadowMapResources()
	{
		for (UINT i = 0; i < MaxLights; i++)
		{
			FreeCSUDescriptor(shadowMapSrvCpuDescriptorHandle[i], shadowMapSrvGpuDescriptorHandle[i]);
		}

		DestroyConstantsBuffer(shadowMapsCbv);
	}
}