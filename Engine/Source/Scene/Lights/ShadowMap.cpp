#include "pch.h"
#include "Lights.h"
#include "../Camera/Camera.h"
#include "../../Common/d3dx12.h"
#include "../../Common/DirectXHelper.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/RenderPass/RenderPass.h"
#include "../../Renderer/DeviceUtils/D3D12Device/Builder.h"
#include "../../Renderer/DeviceUtils/D3D12Device/Interop.h"
#include "../../Renderer/DeviceUtils/RootSignature/RootSignature.h"
#include "../../Renderer/DeviceUtils/PipelineState/PipelineState.h"
#if defined(_EDITOR)
#include "../../Editor/Editor.h"
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

	void Light::CreateDirectionalLightShadowMap()
	{
		shadowMapScissorRect.clear();
		shadowMapViewport.clear();

		shadowMapProjectionMatrix = XMMatrixOrthographicRH(json.at("viewWidth"), json.at("viewHeight"), json.at("nearZ"), json.at("farZ"));
		shadowMapScissorRect.push_back({ 0, 0, static_cast<long>(json.at("shadowMapWidth")), static_cast<long>(json.at("shadowMapHeight")) });
		shadowMapViewport.push_back({ 0.0f , 0.0f, static_cast<float>(json.at("shadowMapWidth")), static_cast<float>(json.at("shadowMapHeight")), 0.0f, 1.0f });
		shadowMapTexelInvSize = { 1.0f / static_cast<float>(json.at("shadowMapWidth")), 1.0f / static_cast<float>(json.at("shadowMapHeight")) };

		auto camera = CreateCamera({
			{ "uuid", uuid() + "-cam"},
			{ "name", name() + ".cam"},
			{ "projectionType", "Orthographic" },
			{ "orthographic", {
				{ "nearZ", json.at("nearZ") },
				{ "farZ", json.at("farZ") },
				{ "width", json.at("viewWidth") },
				{ "height", json.at("viewHeight") },
			}},
			{ "rotation", { rotation().x, rotation().y, 0.0f}},
			{ "light", uuid()}
			}
		);
		XMVECTOR camPos = XMVectorScale(XMVector3Normalize(camera->CameraFw()), -directionalDistance());
		camera->position(*(XMFLOAT3*)camPos.m128_f32);
		camera->orthographic.updateProjectionMatrix(json.at("viewWidth"), json.at("viewHeight"));
		shadowMapCameras.push_back(camera);
	}

	void Light::CreateSpotLightShadowMap()
	{
		shadowMapScissorRect.clear();
		shadowMapViewport.clear();

		shadowMapProjectionMatrix = XMMatrixPerspectiveFovRH(coneAngle() * 2.0f, 1.0f, json.at("nearZ"), json.at("farZ"));
		shadowMapScissorRect.push_back({ 0, 0, static_cast<long>(json.at("shadowMapWidth")), static_cast<long>(json.at("shadowMapHeight")) });
		shadowMapViewport.push_back({ 0.0f , 0.0f, static_cast<float>(json.at("shadowMapWidth")), static_cast<float>(json.at("shadowMapHeight")), 0.0f, 1.0f });
		shadowMapTexelInvSize = { 1.0f / static_cast<float>(json.at("shadowMapWidth")), 1.0f / static_cast<float>(json.at("shadowMapHeight")) };

		auto camera = CreateCamera({
			{ "uuid", uuid() + "-cam"},
			{ "name", name() + ".cam"},
			{ "projectionType", "Perspective" },
			{ "perspective", {
				{ "fovAngleY", coneAngle() * 2.0f },
				{ "width", json.at("viewWidth") },
				{ "height", json.at("viewHeight") },
			}},
			{ "position", { position().x, position().y, position().z}},
			{ "rotation", { rotation().x, rotation().y, 0.0f }},
			{ "light", uuid()}
			});
		camera->perspective.updateProjectionMatrix(json.at("viewWidth"), json.at("viewHeight"));
		shadowMapCameras.push_back(camera);
	}

	void Light::CreatePointLightShadowMap()
	{
		shadowMapScissorRect.clear();
		shadowMapViewport.clear();
		for (unsigned int i = 0U; i < 6U; i++)
		{
			shadowMapScissorRect.push_back({ 0, static_cast<long>(i) * static_cast<long>(json.at("shadowMapHeight")), static_cast<long>(json.at("shadowMapWidth")), static_cast<long>(i + 1U) * static_cast<long>(json.at("shadowMapHeight")) });
			shadowMapViewport.push_back({ 0.0f, static_cast<float>(i) * static_cast<float>(json.at("shadowMapHeight")), static_cast<float>(json.at("shadowMapWidth")), static_cast<float>(json.at("shadowMapHeight")), 0.0f, 1.0f });
		}
		shadowMapClearScissorRect = { 0, 0, static_cast<long>(json.at("shadowMapWidth")), 6L * static_cast<long>(json.at("shadowMapHeight")) };
		shadowMapClearViewport = { 0.0f, 0.0f , static_cast<float>(json.at("shadowMapWidth")), 6L * static_cast<float>(json.at("shadowMapHeight")), 0.0f, 1.0f };
		shadowMapProjectionMatrix = XMMatrixPerspectiveFovRH(DirectX::XM_PIDIV2, 1.0f, json.at("nearZ"), json.at("farZ"));

		for (unsigned int i = 0U; i < 6U; i++)
		{
			auto camera = CreateCamera(
				{
					{ "name", name() + ".cam." + std::to_string(i)},
					{ "uuid", uuid() + "-cam-" + std::to_string(i)},
					{ "projectionType", "Perspective" },
					{ "perspective",
					{
						{ "fovAngleY", DirectX::XM_PIDIV2 },
						{ "width", static_cast<float>(json.at("shadowMapWidth")) },
						{ "height", static_cast<float>(json.at("shadowMapHeight")) },
					}
					},
					{ "position", { position().x, position().y, position().z}},
					{ "light", uuid()}
				}
			);
			camera->perspective.updateProjectionMatrix(static_cast<float>(json.at("shadowMapWidth")), static_cast<float>(json.at("shadowMapHeight")));

			shadowMapCameras.push_back(camera);
		}
	}

	void Light::CreateShadowMapDepthStencilResource()
	{
		unsigned int w = static_cast<unsigned int>(json.at("shadowMapWidth"));
		unsigned int h = static_cast<unsigned int>(json.at("shadowMapHeight")) * ((lightType() == LT_Point) ? 6U : 1U);
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
	void Light::CreateShadowMapMinMaxChain()
	{
		//pick the gpu handles for the final shadowmap and copies for the min/max chain initial calculation
		CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle = GetShadowMapGpuDescriptorHandle(shadowMapIndex);
		CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle1 = shadowMapChainGpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle2 = shadowMapChainGpuHandle;

		float texWidth = static_cast<float>(json.at("shadowMapWidth"));
		float texHeight = static_cast<float>(json.at("shadowMapHeight")) * ((lightType() == LT_Point) ? 6.0f : 1.0f);

		//calculate the width/height of the texture and the TexelInvSize of the shadow map texture for the current pass
		unsigned int width = static_cast<unsigned int>(texWidth) >> 1;
		unsigned int height = static_cast<unsigned int>(texHeight) >> 1;
		XMFLOAT2 TexelInvSize = { 1.0f / static_cast<float>(texWidth), 1.0f / static_cast<float>(texHeight) };

		do
		{
			//push a render pass for the current chain depth
			std::string ShadowMinMaxChainRenderPassName = "ShadowMapMinMaxChainRenderPass[" + std::to_string(max(2U, width)) + "," + std::to_string(max(2U, height)) + "]";
			shadowMapMinMaxChainRenderPass.push_back(
				CreateRenderPass(
					ShadowMinMaxChainRenderPassName,
					{ DXGI_FORMAT_R32_FLOAT , DXGI_FORMAT_R32_FLOAT },
					DXGI_FORMAT_UNKNOWN,
					max(2U, width),
					max(2U, height)
				)
			);

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

			//write the TexelInvSize to constants buffers
			for (unsigned int i = 0; i < renderer->numFrames; i++)
			{
				shadowMapMinMaxChainRenderable.back()->WriteConstantsBuffer("texelInvSize", TexelInvSize, i);
			}

			//get the material instance of the renderable and push the gpu handles 1&2 of the chain
			std::shared_ptr<MaterialInstance>& shadowMapMinMaxChainMaterial = shadowMapMinMaxChainRenderable.back()->meshMaterials.begin()->second;

			std::string ShadowMapMinMaxChainMat1 = "ShadowMapMinMaxChainMat1[" + std::to_string(max(2U, width)) + "," + std::to_string(max(2U, height)) + "]";
			std::string ShadowMapMinMaxChainMat2 = "ShadowMapMinMaxChainMat2[" + std::to_string(max(2U, width)) + "," + std::to_string(max(2U, height)) + "]";
			shadowMapMinMaxChainMaterial->textures.insert_or_assign(TextureType_MinTexture, GetTextureFromGPUHandle(ShadowMapMinMaxChainMat1, shadowMapChainGpuHandle1));
			shadowMapMinMaxChainMaterial->textures.insert_or_assign(TextureType_MaxTexture, GetTextureFromGPUHandle(ShadowMapMinMaxChainMat2, shadowMapChainGpuHandle1));

			//get the new gpu handles 1&2 for the next chain
			std::shared_ptr<RenderToTexturePass>& pass = shadowMapMinMaxChainRenderPass.back();
			shadowMapChainGpuHandle1 = pass->renderToTexture[0]->gpuTextureHandle;
			shadowMapChainGpuHandle2 = pass->renderToTexture[1]->gpuTextureHandle;

			//calculate the next TexelInvSize
			TexelInvSize = { 1.0f / static_cast<float>(max(2U, width)), 1.0f / static_cast<float>(max(2U, height)) };

			//calculate the next width and height
			width = max(1U, width >> 1);
			height = max(1U, height >> 1);
		} while (width != 1U || height != 1U);

		//create the end result render pass
		shadowMapMinMaxChainResultRenderPass = CreateRenderPass(
			"ShadowMinMaxChainRenderPassResult",
			{ DXGI_FORMAT_R8G8B8A8_UNORM },
			DXGI_FORMAT_UNKNOWN,
			static_cast<unsigned int>(texWidth),
			static_cast<unsigned int>(texHeight)
		);

		//create the end result renderable
		shadowMapMinMaxChainResultRenderable = CreateRenderable(
			{
				{ "meshMaterials" ,
					{
						{
							{ "material", FindMaterialUUIDByName("DepthMinMaxToRGBA") },
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

		std::shared_ptr<RenderToTexturePass>& lastMinMaxPass = shadowMapMinMaxChainRenderPass.back();
		std::shared_ptr<MaterialInstance>& shadowMapMinMaxChainResultMaterial = shadowMapMinMaxChainResultRenderable->meshMaterials.begin()->second;

		std::string ShadowMapResultChainMat1 = "ShadowMapMinMaxResult1";
		std::string ShadowMapResultChainMat2 = "ShadowMapMinMaxResult2";
		std::string ShadowMapResultChainMat3 = "ShadowMapMinMaxResult3";
		shadowMapMinMaxChainResultMaterial->textures.insert_or_assign(TextureType_DepthTexture, GetTextureFromGPUHandle(ShadowMapResultChainMat1, shadowMapChainGpuHandle));
		shadowMapMinMaxChainResultMaterial->textures.insert_or_assign(TextureType_MinTexture, GetTextureFromGPUHandle(ShadowMapResultChainMat2, lastMinMaxPass->renderToTexture[0]->gpuTextureHandle));
		shadowMapMinMaxChainResultMaterial->textures.insert_or_assign(TextureType_MaxTexture, GetTextureFromGPUHandle(ShadowMapResultChainMat3, lastMinMaxPass->renderToTexture[1]->gpuTextureHandle));
	}
#endif

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

	void UpdateConstantsBufferShadowMapAttributes(const std::shared_ptr<Light>& light, unsigned int backbufferIndex, unsigned int shadowMapIndex)
	{
		if (!light->hasShadowMaps()) return;

		ShadowMapAttributes atts{};
		ZeroMemory(&atts, sizeof(atts));

		switch (light->lightType()) {
		case LT_Directional:
		{
			XMMATRIX view = light->shadowMapCameras[0]->ViewMatrix();
			XMMATRIX projection = light->shadowMapCameras[0]->orthographic.projectionMatrix;
			atts.atts0 = XMMatrixMultiply(view, projection);
			atts.atts6 = { 0.0002f, light->shadowMapTexelInvSize.x, light->shadowMapTexelInvSize.y, 0.0f }; //ZBias, TexelInvSize
		}
		break;
		case LT_Spot:
		{
			XMMATRIX view = light->shadowMapCameras[0]->ViewMatrix();
			XMMATRIX projection = light->shadowMapCameras[0]->perspective.projectionMatrix;
			atts.atts0 = XMMatrixMultiply(view, projection);
			atts.atts6 = { 0.0001f, light->shadowMapTexelInvSize.x, light->shadowMapTexelInvSize.y, 0.0f }; //ZBias, TexelInvSize
		}
		break;
		case LT_Point:
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
		{
			assert(light->lightType() != LT_Directional || light->lightType() != LT_Spot || light->lightType() != LT_Point);
		}
		break;
		}

		size_t offset = shadowMapsCbv->alignedConstantBufferSize * backbufferIndex;
		offset += sizeof(ShadowMapAttributes) * shadowMapIndex;
		memcpy(shadowMapsCbv->mappedConstantBuffer + offset, &atts, sizeof(atts));
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
			cam->light = nullptr;
			cam->this_ptr = nullptr;
			DestroyCamera(cam);
		}
		shadowMapCameras.clear();
	}

#if defined(_EDITOR)
	void Light::DestroyShadowMapMinMaxChain()
	{
		shadowMapMinMaxChainRenderPass.clear();
		for (auto& renderable : shadowMapMinMaxChainRenderable)
		{
			DestroyRenderable(renderable);
		}
		shadowMapMinMaxChainRenderable.clear();
		shadowMapMinMaxChainResultRenderPass = nullptr;
		DestroyRenderable(shadowMapMinMaxChainResultRenderable);
	}
#endif

	//RENDER

	std::set<LightType> onePassTypes = { LT_Directional, LT_Spot };
	void Light::RenderShadowMap(std::function<void(size_t passHash, unsigned int)> renderScene)
	{
		auto& commandList = renderer->commandList;

		shadowMapRenderPass->Pass([this, renderScene, &commandList](size_t passHash)
			{
				if (onePassTypes.contains(lightType()))
				{
					renderScene(passHash, 0);
				}
				else
				{
					for (unsigned int i = 0; i < 6; i++)
					{
						commandList->RSSetViewports(1, &shadowMapViewport.at(i));
						commandList->RSSetScissorRects(1, &shadowMapScissorRect.at(i));
						renderScene(passHash, i);
					}
				}
			}
		);
	}

#if defined(_EDITOR)
	void Light::RenderShadowMapMinMaxChain()
	{
#if defined(_DEVELOPMENT)
		auto& commandList = renderer->commandList;

		PIXBeginEvent(commandList.p, 0, "ShadowMapMinMaxChain");
#endif

		for (unsigned int i = 0; i < shadowMapMinMaxChainRenderPass.size(); i++)
		{
			std::shared_ptr<RenderToTexturePass>& renderPass = shadowMapMinMaxChainRenderPass[i];
			std::shared_ptr<Renderable>& quad = shadowMapMinMaxChainRenderable[i];

#if defined(_DEVELOPMENT)
			PIXBeginEvent(commandList.p, 0, renderPass->name.c_str());
#endif
			renderPass->Pass([&quad](size_t passHash)
				{
					quad->visible(true);
					quad->Render(passHash);
					quad->visible(false);
				}
			);
#if defined(_DEVELOPMENT)
			PIXEndEvent(commandList.p);
#endif
		}

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, "ShadowMapMinMaxChainResult");
#endif
		shadowMapMinMaxChainResultRenderPass->Pass([this](size_t passHash)
			{
				shadowMapMinMaxChainResultRenderable->visible(true);
				shadowMapMinMaxChainResultRenderable->Render(passHash);
				shadowMapMinMaxChainResultRenderable->visible(false);
			}
		);

#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
		PIXEndEvent(commandList.p);
#endif
	}

	void Light::ImDrawDirectionalShadowMap()
	{
		std::string tableName = "light-directional-shadowmap";

		bool shadowMap = hasShadowMaps();

		if (ImGui::Checkbox("Shadow Map", &shadowMap))
		{
			hasShadowMaps(shadowMap);
			if (shadowMap)
			{
				shadowMapUpdateFlags |= ShadowMapUpdateFlags_CreateBoth;
			}
			else
			{
				shadowMapUpdateFlags |= ShadowMapUpdateFlags_DestroyBoth;
			}
		}

		if (shadowMap && !shadowMapUpdateFlags)
		{
			unsigned int shadowMapWidth = static_cast<unsigned int>(json.at("shadowMapWidth"));
			unsigned int shadowMapHeight = static_cast<unsigned int>(json.at("shadowMapHeight"));
			float viewWidth = static_cast<float>(json.at("viewWidth"));
			float viewHeight = static_cast<float>(json.at("viewHeight"));
			float nearZ = static_cast<float>(json.at("nearZ"));
			float farZ = static_cast<float>(json.at("farZ"));

			static std::vector<std::string> shadowMapSizes = { "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384" };

			if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				std::string selectecShadowMapWidth = std::to_string(shadowMapWidth);
				DrawComboSelection(selectecShadowMapWidth, shadowMapSizes, [this](std::string value)
					{
						json.at("shadowMapWidth") = std::stof(value);
						shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
					}, "TexWidth"
				);
				ImGui::TableSetColumnIndex(1);
				std::string selectecShadowMapHeight = std::to_string(shadowMapHeight);
				DrawComboSelection(selectecShadowMapHeight, shadowMapSizes, [this](std::string value)
					{
						json.at("shadowMapHeight") = std::stof(value);
						shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
					}, "TexHeight"
				);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::InputFloat("ViewWidth", &viewWidth))
				{
					json.at("viewWidth") = viewWidth;
					shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
				}
				ImGui::TableSetColumnIndex(1);
				if (ImGui::InputFloat("ViewHeight", &viewHeight))
				{
					json.at("viewHeight") = viewHeight;
					shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::InputFloat("NearZ", &nearZ))
				{
					json.at("nearZ") = nearZ;
					shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
				}
				ImGui::TableSetColumnIndex(1);
				if (ImGui::InputFloat("FarZ", &farZ))
				{
					json.at("farZ") = farZ;
					shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
				}

				ImGui::EndTable();
			}

			using namespace Editor;
			ImDrawShadowMapMinMaxChain();
		}
	}

	void Light::ImDrawSpotShadowMap()
	{
		std::string tableName = "light-spot-shadowmap";

		bool shadowMap = hasShadowMaps();

		if (ImGui::Checkbox("Shadow Map", &shadowMap))
		{
			hasShadowMaps(shadowMap);
			if (shadowMap)
			{
				shadowMapUpdateFlags |= ShadowMapUpdateFlags_CreateBoth;
			}
			else
			{
				shadowMapUpdateFlags |= ShadowMapUpdateFlags_DestroyBoth;
			}
		}

		if (shadowMap && !shadowMapUpdateFlags)
		{
			unsigned int shadowMapWidth = static_cast<unsigned int>(json.at("shadowMapWidth"));
			unsigned int shadowMapHeight = static_cast<unsigned int>(json.at("shadowMapHeight"));
			float viewWidth = static_cast<float>(json.at("viewWidth"));
			float viewHeight = static_cast<float>(json.at("viewHeight"));
			float nearZ = static_cast<float>(json.at("nearZ"));
			float farZ = static_cast<float>(json.at("farZ"));

			static std::vector<std::string> shadowMapSizes = { "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384" };

			if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				std::string selectecShadowMapWidth = std::to_string(shadowMapWidth);
				DrawComboSelection(selectecShadowMapWidth, shadowMapSizes, [this](std::string value)
					{
						json.at("shadowMapWidth") = std::stof(value);
						shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
					}, "TexWidth"
				);
				ImGui::TableSetColumnIndex(1);
				std::string selectecShadowMapHeight = std::to_string(shadowMapHeight);
				DrawComboSelection(selectecShadowMapHeight, shadowMapSizes, [this](std::string value)
					{
						json.at("shadowMapHeight") = std::stof(value);
						shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
					}, "TexHeight"
				);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::InputFloat("ViewWidth", &viewWidth))
				{
					json.at("viewWidth") = viewWidth;
					shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
				}
				ImGui::TableSetColumnIndex(1);
				if (ImGui::InputFloat("ViewHeight", &viewHeight))
				{
					json.at("viewHeight") = viewHeight;
					shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::InputFloat("NearZ", &nearZ))
				{
					json.at("nearZ") = nearZ;
					shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
				}
				ImGui::TableSetColumnIndex(1);
				if (ImGui::InputFloat("FarZ", &farZ))
				{
					json.at("farZ") = farZ;
					shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
				}

				ImGui::EndTable();
			}

			using namespace Editor;
			ImDrawShadowMapMinMaxChain();
		}
	}

	void Light::ImDrawPointShadowMap()
	{
		std::string tableName = "light-point-shadowmap";

		bool shadowMap = hasShadowMaps();

		if (ImGui::Checkbox("Shadow Map", &shadowMap))
		{
			hasShadowMaps(shadowMap);
			if (shadowMap)
			{
				shadowMapUpdateFlags |= ShadowMapUpdateFlags_CreateBoth;
			}
			else
			{
				shadowMapUpdateFlags |= ShadowMapUpdateFlags_DestroyBoth;
			}
		}

		if (shadowMap && !shadowMapUpdateFlags)
		{
			unsigned int shadowMapWidth = static_cast<unsigned int>(json.at("shadowMapWidth"));
			float nearZ = static_cast<float>(json.at("nearZ"));
			float farZ = static_cast<float>(json.at("farZ"));

			static std::vector<std::string> shadowMapSizes = { "64", "128", "256", "512", "1024", "2048" };

			if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				std::string selectecShadowMapWidth = std::to_string(shadowMapWidth);
				DrawComboSelection(selectecShadowMapWidth, shadowMapSizes, [this](std::string value)
					{
						json.at("shadowMapHeight") = std::stof(value);
						json.at("shadowMapWidth") = std::stof(value);
						shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
					}, "Dimension"
				);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::InputFloat("NearZ", &nearZ))
				{
					json.at("nearZ") = nearZ;
					shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
				}
				ImGui::TableSetColumnIndex(1);
				if (ImGui::InputFloat("FarZ", &farZ))
				{
					json.at("farZ") = farZ;
					shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
				}

				ImGui::EndTable();
			}

			using namespace Editor;
			ImDrawShadowMapMinMaxChain();
		}
	}

	void Light::ImDrawShadowMapMinMaxChain()
	{
		if (shadowMapUpdateFlags) return;

		float texWidth = static_cast<float>(json.at("shadowMapWidth"));
		float texHeight = static_cast<float>(json.at("shadowMapHeight")) * ((lightType() == LT_Point) ? 6.0f : 1.0f);

		ImDrawTextureImage((ImTextureID)shadowMapMinMaxChainResultRenderPass->renderToTexture[0]->gpuTextureHandle.ptr,
			static_cast<unsigned int>(texWidth),
			static_cast<unsigned int>(texHeight)
		);
	}

#endif

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