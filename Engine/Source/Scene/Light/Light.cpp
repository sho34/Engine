#include "pch.h"
#include "Light.h"
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

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectLight(std::shared_ptr<Light> light);
}
#endif

namespace Scene {
#include <Editor/JDrawersDef.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <TrackUUID/JDef.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <LightAtt.h>
#include <JEnd.h>

	using namespace DeviceUtils;

	static std::shared_ptr<ConstantsBuffer> lightsCbv = nullptr; //CBV for lights pool

	//CREATE
	void CreateLightsResources() {
		lightsCbv = CreateConstantsBuffer(sizeof(LightPool), "lightsCbv");
	}

	Light::Light(nlohmann::json json) : SceneObject(json)
	{
#include <Attributes/JInit.h>
#include <LightAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <LightAtt.h>
#include <JEnd.h>
	}

	void Light::Initialize()
	{
#include <TrackUUID/JInsert.h>
#include <LightAtt.h>
#include <JEnd.h>

		if (hasShadowMaps())
		{
			CreateShadowMap();
		}
#if defined(_EDITOR)
		CreateLightBillboard();
#endif
	}

	void Light::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <LightAtt.h>
#include <JEnd.h>

		BindRenderablesToShadowMapCamera();

#if defined(_EDITOR)
		BindLightBillboardToScene();
#endif
	}

	void Light::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <LightAtt.h>
#include <JEnd.h>

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
		std::set<std::shared_ptr<Light>> lightsToDestroySMChain;
		std::set<std::shared_ptr<Light>> lightsToDelete;

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
			l->UpdateLightBillboard();

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

			//if destroying SMChain
			if (l->destroySMChain)
			{
				lightsToDestroySMChain.insert(l);
				l->destroySMChain = false;
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

			if (l->markedForDelete) {
				lightsToDelete.insert(l);
				if (l->hasShadowMaps())
				{
					lightsToDestroyShadowMaps.insert(l);
				}
			}
		}

		bool criticalFrame = !!lightsToDestroyShadowMaps.size() || !!lightsToCreateShadowMaps.size() || !!lightsToDestroySMChain.size() || !!lightsToDelete.size();

		if (criticalFrame)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([
				&lightsToDestroyShadowMaps,
				&lightsToCreateShadowMaps,
				&lightsToDestroySMChain,
				&lightsToDelete
			]
				{
					for (auto& l : lightsToDestroyShadowMaps)
					{
						l->UnbindRenderablesFromShadowMapCameras();
						l->DestroyShadowMapMinMaxChain();
						l->DestroyShadowMap();
					}
					for (auto& l : lightsToCreateShadowMaps)
					{
						l->CreateShadowMap();
						l->CreateShadowMapMinMaxChain();
						l->BindRenderablesToShadowMapCamera();
					}
					for (auto& l : lightsToDestroySMChain)
					{
						l->DestroyShadowMapMinMaxChain();
					}
					for (auto& l : lightsToDelete)
					{
						EraseLightFromLights(l);
						EraseLightFromShadowMapLights(l);
						std::shared_ptr<Light> light = l;
						SafeDeleteSceneObject(light);
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

	void DestroyLights()
	{
		auto tmp = Lights;
		for (auto& [_, l] : tmp)
		{
			SafeDeleteSceneObject(l);
		}
#include <TrackUUID/JClear.h>
#include <LightAtt.h>
#include <JEnd.h>
	}

	void DeleteLight(std::string uuid)
	{
		std::shared_ptr<Light> l = FindInLights(uuid);
#if defined(_EDITOR)
		if (l->lightBillboard)
		{
			DeleteSceneObject(l->lightBillboard->uuid());
			l->lightBillboard->OnPick = [] {};
			l->lightBillboard = nullptr;
		}
#endif
		l->markedForDelete = true;
	}

	void ResetConstantsBufferLightAttributes(unsigned int backbufferIndex)
	{
		size_t offset = lightsCbv->alignedConstantBufferSize * backbufferIndex;
		LightPool* lpool = (LightPool*)(lightsCbv->mappedConstantBuffer + offset);
		ZeroMemory(lpool, sizeof(LightPool));
	}

	//EDITOR
#if defined(_EDITOR)
	static std::map<LightType, nlohmann::json> defaultShadowMapParameters = {
		{ LT_Directional, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"viewWidth", 32.0f}, {"viewHeight",32.0f},{"nearZ",0.01f}, {"farZ",1000.0f}}},
		{ LT_Spot, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"viewWidth", 32.0f}, {"viewHeight",32.0f},{"nearZ",0.01f}, {"farZ",100.0f}} },
		{ LT_Point, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"nearZ",0.01f}, {"farZ",20.0f}} },
	};

	static std::vector<std::string> shadowMapJsonAttributes = {
		"shadowMapWidth", "shadowMapHeight", "viewWidth", "viewHeight", "nearZ", "farZ",
	};

	void WriteLightsJson(nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <LightAtt.h>
#include <JEnd.h>
	}

#endif

#if defined(_EDITOR)
	void Light::EditorPreview(size_t flags)
	{
		if (flags & (1 << Light::Update_hasShadowMaps))
		{
			if (hasShadowMaps())
				CreateShadowMapMinMaxChain();
		}
		switch (lightType())
		{
		case LT_Directional:
		case LT_Spot:
		case LT_Point:
		{
			//Editor::SelectSceneObject(uuid());
		}
		break;
		}
	}

	void Light::DestroyEditorPreview()
	{
		destroySMChain = true;
		switch (lightType())
		{
		case LT_Directional:
		{
			//Editor::DeselectSceneObject(uuid());
		}
		break;
		}
	}

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

	void Light::CreateLightBillboard()
	{
		if (lightType() == LT_Ambient) return;

		nlohmann::json jbillboard = nlohmann::json(
			{
				{ "meshMaterials",
					{
						{
							{ "material", FindMaterialUUIDByName("LightBulb") },
							{ "mesh", "7dec1229-075f-4599-95e1-9ccfad0d48b1" }
						}
					}
				},
				{ "castShadows", false },
				{ "shadowed", false },
				{ "name" , uuid() + "-billboard" },
				{ "uuid" , getUUID() },
				{ "position" , { 0.0f, 0.0f, 0.0f} },
				{ "topology", "TRIANGLELIST"},
				{ "rotation" , { 0.0, 0.0, 0.0 } },
				{ "scale" , { 1.0f, 1.0f, 1.0f } },
				{ "skipMeshes" , {}},
				{ "visible" , true},
				{ "hidden" , true},
				{ "cameras", { GetMouseCameras().at(0)->uuid()}},
				{ "passMaterialOverrides",
					{
						{
							{ "meshIndex", 0 },
							{ "renderPass", FindRenderPassUUIDByName("PickingPass") },
							{ "material", FindMaterialUUIDByName("LightBulbPicking") }
						}
					}
				}
			}
		);
		lightBillboard = CreateSceneObjectFromJson<Renderable>(jbillboard);
		lightBillboard->OnPick = [this] {Editor::SelectLight(this_ptr); };
	}

	void Light::DestroyLightBillboard()
	{
	}

	void Light::UpdateLightBillboard()
	{
		if (!lightBillboard) return;
		lightBillboard->position(position());
		XMFLOAT3 baseColor = color();
		lightBillboard->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, renderer->backBufferIndex);
	}

	void Light::BindLightBillboardToScene()
	{
		if (lightType() == LT_Ambient) return;

		lightBillboard->BindToScene();
	}

	BoundingBox Light::GetBoundingBox()
	{
		return BoundingBox(position(), { 0.1f,0.1f,0.1f });
	}
#endif
}