#include "pch.h"
#include "Lights.h"
#include <Camera/Camera.h>
#include <d3dx12.h>
#include <DirectXHelper.h>
#include <Renderer.h>
#include <DeviceUtils/D3D12Device/Builder.h>
#include <DeviceUtils/D3D12Device/Interop.h>
#include <DeviceUtils/RootSignature/RootSignature.h>
#include <DeviceUtils/PipelineState/PipelineState.h>
#if defined(_EDITOR)
#include <imgui.h>
#include <Scene.h>
#endif
#include <Json.h>
#include <NoStd.h>

extern std::shared_ptr<Renderer> renderer;

namespace Scene {
#include <JExposeAttDrawersDef.h>
#include <LightsAtt.h>
#include <JExposeEnd.h>

#include <JExposeTrackUUIDDef.h>
#include <LightsAtt.h>
#include <JExposeEnd.h>

	using namespace DeviceUtils;

	static std::shared_ptr<ConstantsBuffer> lightsCbv = nullptr; //CBV for lights pool

	//CREATE
	void CreateLightsResources() {
		lightsCbv = CreateConstantsBuffer(sizeof(LightPool), "lightsCbv");
	}

	Light::Light(nlohmann::json json) : SceneObject(json)
	{
#include <JExposeInit.h>
#include <LightsAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttUpdate.h>
#include <LightsAtt.h>
#include <JExposeEnd.h>
	}

	void Light::BindToScene()
	{
		Lights.insert_or_assign(uuid(), this_ptr);
		if (hasShadowMaps())
		{
			CreateShadowMap();
			BindRenderablesToShadowMapCamera();
		}
	}

	void Light::UnbindFromScene()
	{
		Lights.erase(uuid());

		if (!hasShadowMaps()) return;

#if defined(_EDITOR)
		DestroyShadowMapMinMaxChain();
#endif
		DestroyShadowMap();
	}

	void Light::Destroy()
	{
		DestroyEditorPreview();
	}

	XMMATRIX Light::world()
	{
		XMFLOAT3 posV = position();
		XMFLOAT3 rotV = rotation();
		float roll, pitch, yaw;
		pitch = rotV.x; yaw = rotV.y; roll = rotV.z;
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
		XMMATRIX rotationM = XMMatrixRotationQuaternion(rotQ);
		XMMATRIX positionM = XMMatrixTranslationFromVector({ posV.x, posV.y, posV.z });
		return XMMatrixMultiply(rotationM, positionM);
	}

	XMVECTOR Light::fw()
	{
		FXMVECTOR dir = { 0.0f, 0.0f, 1.0f,0.0f };
		XMFLOAT3 rotV = rotation();
		float roll, pitch, yaw;
		pitch = rotV.x; yaw = rotV.y; roll = rotV.z;
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
		XMVECTOR fw = XMVector3Normalize(XMVector3Rotate(dir, rotQ));
		return fw;
	}

	//READ&GET
	std::shared_ptr<ConstantsBuffer> GetLightsConstantsBuffer() {
		return lightsCbv;
	}

	//UPDATE
	void LightsStep()
	{
#if defined(_EDITOR)
		std::set<std::shared_ptr<Light>> lightsToDestroyShadowMaps;
		std::set<std::shared_ptr<Light>> lightsToCreateShadowMaps;
		std::set<std::shared_ptr<Light>> lightsToUpdateCamAttributes;
		std::set<std::shared_ptr<Light>> lightsToUpdateTransformation;

		std::set<Light::Light_UpdateFlags> smCamAttributes =
		{
			Light::Update_coneAngle, Light::Update_shadowMapWidth, Light::Update_shadowMapHeight,
			Light::Update_viewWidth, Light::Update_viewHeight, Light::Update_nearZ, Light::Update_farZ
		};
		std::set<Light::Light_UpdateFlags> smCamTransformations =
		{
			Light::Update_position, Light::Update_rotation, Light::Update_dirDist
		};

		for (auto& [uuid, l] : Lights)
		{
			//if the light type changed
			if (l->dirty(Light::Update_lightType))
			{
				//use default attributes depending of the light type
				l->JUpdate(editorDefaultLightProperties.at(l->lightType()));

				//we deactivate shadowmaps always a light type is converted
				if (l->hasShadowMaps())
				{
					lightsToDestroyShadowMaps.insert(l);
					l->hasShadowMaps(false);
				}

				l->clean(Light::Update_lightType);
			}

			if (l->dirty(Light::Update_hasShadowMaps))
			{
				if (l->hasShadowMaps())
				{
					lightsToCreateShadowMaps.insert(l);
				}
				else
				{
					lightsToDestroyShadowMaps.insert(l);
				}
				l->clean(Light::Update_hasShadowMaps);
			}

			//if resizing
			if (l->dirty(Light::Update_shadowMapWidth) || l->dirty(Light::Update_shadowMapHeight))
			{
				//verify first if the light has shadowmaps(it should)
				if (l->hasShadowMaps())
				{
					lightsToDestroyShadowMaps.insert(l);
					lightsToCreateShadowMaps.insert(l);
				}
				l->clean(Light::Update_shadowMapWidth);
				l->clean(Light::Update_shadowMapHeight);
			}

			if (std::any_of(smCamAttributes.begin(), smCamAttributes.end(), [l](auto flag) { return l->dirty(flag); }))
			{
				lightsToUpdateCamAttributes.insert(l);
				std::for_each(smCamAttributes.begin(), smCamAttributes.end(), [l](auto flag) { l->clean(flag); });
			}
			if (std::any_of(smCamTransformations.begin(), smCamTransformations.end(), [l](auto flag) { return l->dirty(flag); }))
			{
				lightsToUpdateTransformation.insert(l);
				std::for_each(smCamTransformations.begin(), smCamTransformations.end(), [l](auto flag) { l->clean(flag); });
			}
		}

		bool criticalFrame = !!lightsToDestroyShadowMaps.size() || !!lightsToCreateShadowMaps.size();

		if (criticalFrame)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([
				&lightsToDestroyShadowMaps,
				&lightsToCreateShadowMaps
			]
				{
					for (auto& l : lightsToDestroyShadowMaps)
					{
						l->DestroyShadowMapMinMaxChain();
						l->DestroyShadowMap();
					}
					for (auto& l : lightsToCreateShadowMaps)
					{
						l->CreateShadowMap();
						l->CreateShadowMapMinMaxChain();
						l->BindRenderablesToShadowMapCamera();
					}
				}
			);
		}

		for (auto& l : lightsToUpdateCamAttributes)
		{
			l->UpdateShadowMapCameraProperties();
		}
		for (auto& l : lightsToUpdateTransformation)
		{
			l->UpdateShadowMapCameraTransformation();
		}
#endif
	}

	void WriteConstantsBufferNumLights(unsigned int backbufferIndex, unsigned int numLights)
	{
		size_t offset = lightsCbv->alignedConstantBufferSize * backbufferIndex;
		LightPool* lpool = (LightPool*)(lightsCbv->mappedConstantBuffer + offset);
		lpool->numLights.D[0] = numLights;
	}

	void WriteConstantsBufferLightAttributes(const std::shared_ptr<Light>& light, unsigned int backbufferIndex, unsigned int lightIndex, unsigned int shadowMapIndex)
	{
		LightAttributes atts{};
		ZeroMemory(&atts, sizeof(atts));
		atts.lightType = light->lightType();

		switch (light->lightType())
		{
		case LT_Ambient:
		{
			atts.lightColor = light->color() * light->brightness();
		}
		break;
		case LT_Directional:
		{
			atts.lightColor = light->color() * light->brightness();
			XMVECTOR lightDir = light->fw();
			atts.atts1 = *((XMFLOAT4*)&lightDir.m128_f32[0]);
			atts.hasShadowMap = light->hasShadowMaps();
			atts.shadowMapIndex = light->shadowMapIndex;
		}
		break;
		case LT_Spot:
		{
			XMFLOAT3 pos = light->position();
			XMFLOAT3 atte = light->attenuation();
			atts.lightColor = light->color() * light->brightness();
			atts.atts1 = { pos.x, pos.y, pos.z, 0.0f };
			XMVECTOR lightDir = light->fw();
			atts.atts2 = *((XMFLOAT4*)&lightDir.m128_f32[0]);
			atts.atts2.w = cosf(XMConvertToRadians(light->coneAngle()));
			atts.atts3 = { atte.x, atte.y, atte.z };
			atts.hasShadowMap = light->hasShadowMaps();
			atts.shadowMapIndex = light->shadowMapIndex;
		}
		break;
		case LT_Point:
		{
			XMFLOAT3 pos = light->position();
			XMFLOAT3 atte = light->attenuation();
			atts.lightColor = light->color() * light->brightness();
			atts.atts1 = { pos.x, pos.y, pos.z, 0.0f };
			atts.atts2 = { atte.x, atte.y, atte.z, 0.0f };
			atts.hasShadowMap = light->hasShadowMaps();
			atts.shadowMapIndex = light->shadowMapIndex;
		}
		break;
		}

		size_t offset = lightsCbv->alignedConstantBufferSize * backbufferIndex;
		offset += sizeof(LightPool::numLights);
		offset += sizeof(atts) * lightIndex;
		memcpy(lightsCbv->mappedConstantBuffer + offset, &atts, sizeof(atts));
	}

	//DESTROY
	void DestroyLightsResources()
	{
		DestroyConstantsBuffer(lightsCbv);
	}

	void DestroyLight(std::shared_ptr<Light>& light)
	{
		if (light == nullptr) return;
		DEBUG_PTR_COUNT_JSON(light);

		light->UnbindFromScene();
		light->this_ptr = nullptr;
		light = nullptr;
	}

	void DestroyLights()
	{
		auto tmp = Lights;
		for (auto& [_, l] : tmp)
		{
			DestroyLight(l);
		}
		Lights.clear();
	}

	void ResetConstantsBufferLightAttributes(unsigned int backbufferIndex)
	{
		size_t offset = lightsCbv->alignedConstantBufferSize * backbufferIndex;
		LightPool* lpool = (LightPool*)(lightsCbv->mappedConstantBuffer + offset);
		ZeroMemory(lpool, sizeof(LightPool));
	}

	//EDITOR

#if defined(_EDITOR)
	//static void ReplaceLightsJsonAttributes(nlohmann::json& dst, nlohmann::json src, nlohmann::json attributesToAdd, nlohmann::json attributesToDelete)
	//{
	//	for (auto it = attributesToDelete.begin(); it != attributesToDelete.end(); it++)
	//	{
	//		dst.erase(it.key());
	//	}
	//	for (auto it = attributesToAdd.begin(); it != attributesToAdd.end(); it++)
	//	{
	//		dst[it.key()] = src[it.key()];
	//	}
	//}

	static std::map<LightType, nlohmann::json> defaultShadowMapParameters = {
		{ LT_Directional, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"viewWidth", 32.0f}, {"viewHeight",32.0f},{"nearZ",0.01f}, {"farZ",1000.0f}}},
		{ LT_Spot, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"viewWidth", 32.0f}, {"viewHeight",32.0f},{"nearZ",0.01f}, {"farZ",100.0f}} },
		{ LT_Point, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"nearZ",0.01f}, {"farZ",20.0f}} },
	};

	static std::vector<std::string> shadowMapJsonAttributes = {
		"shadowMapWidth", "shadowMapHeight", "viewWidth", "viewHeight", "nearZ", "farZ",
	};

	bool GetLightPopupIsOpen()
	{
		return false;
	}

	void WriteLightsJson(nlohmann::json& json)
	{
		std::map<std::string, std::shared_ptr<Light>> filtered;
		std::copy_if(Lights.begin(), Lights.end(), std::inserter(filtered, filtered.end()), [](const auto& pair)
			{
				auto& [uuid, light] = pair;
				return !light->hidden();
			}
		);
		std::transform(filtered.begin(), filtered.end(), std::back_inserter(json), [](const auto& pair)
			{
				auto& [uuid, light] = pair;
				return *(static_cast<nlohmann::json*>(light.get()));
			}
		);
	}

#endif

	void Light::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox)
	{
		if (lightType() == LT_Ambient || lightType() == LT_Directional)
		{
			bbox->position(XMFLOAT3({ 0.0f ,0.0f,0.0f }));
		}
		else
		{
			bbox->position(position());
		}

		bbox->scale(XMFLOAT3({ 0.5f, 0.5f, 0.5f }));
		bbox->rotation(XMFLOAT3({ 0.0f, 0.0f, 0.0f }));
	}
}