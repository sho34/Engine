#include "pch.h"
#include "Lights.h"
#include "../Camera/Camera.h"
#include "../../Common/d3dx12.h"
#include "../../Common/DirectXHelper.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/DeviceUtils/D3D12Device/Builder.h"
#include "../../Renderer/DeviceUtils/D3D12Device/Interop.h"
#include "../../Renderer/DeviceUtils/RootSignature/RootSignature.h"
#include "../../Renderer/DeviceUtils/PipelineState/PipelineState.h"
#if defined(_EDITOR)
#include "../../Editor/Editor.h"
//#include "../../Animation/Effects/Effects.h"
#include <imgui.h>
#include <ImEditor.h>
#endif
#include <Json.h>
#include <NoStd.h>

extern std::shared_ptr<Renderer> renderer;

namespace Scene {

	using namespace DeviceUtils;
#if defined(_EDITOR)
	using namespace Editor;
#endif

	static std::map<std::string, std::shared_ptr<Light>> lightsByUUID;
	static std::vector<std::shared_ptr<Light>> lights;
	static std::shared_ptr<ConstantsBuffer> lightsCbv = nullptr; //CBV for lights pool
#if defined(_EDITOR)
	static std::vector<std::shared_ptr<Light>> lightsToDestroy;
	nlohmann::json Light::creationJson;
	unsigned int Light::popupModalId = 0U;
#endif

	//CREATE
	void CreateLightsResources() {
		lightsCbv = CreateConstantsBuffer(sizeof(LightPool), "lightsCbv");
	}

	std::shared_ptr<Light> CreateLight(nlohmann::json lightj) {

		std::shared_ptr<Light> light = std::make_shared<Light>();
		light->this_ptr = light;
		light->json = lightj;
		SetIfMissingJson(light->json, "lightType", LightTypeToStr.at(LT_Ambient));
		SetIfMissingJson(light->json, "hasShadowMaps", false);
		SetIfMissingJson(light->json, "position", XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetIfMissingJson(light->json, "attenuation", XMFLOAT3(0.0f, 0.01f, 0.02f));
		SetIfMissingJson(light->json, "color", XMFLOAT3(1.0f, 1.0f, 1.0f));
		SetIfMissingJson(light->json, "brightness", 1.0f);
		SetIfMissingJson(light->json, "rotation", XMFLOAT3(0.0f, 0.0f, 0.0f));

#if defined(_EDITOR)
		switch (light->lightType())
		{
		case LT_Ambient:
		{
			light->editorAmbient = light->json;
		}
		break;
		case LT_Directional:
		{
			light->editorDirectional = light->json;
			SetIfMissingJson(light->json, "zBias", 0.0002f);
		}
		break;
		case LT_Spot:
		{
			light->editorSpot = light->json;
			SetIfMissingJson(light->json, "zBias", 0.0001f);
			SetIfMissingJson(light->json, "coneAngle", 45.0f);
		}
		break;
		case LT_Point:
		{
			light->editorPoint = light->json;
			SetIfMissingJson(light->json, "zBias", 0.0001f);
		}
		break;
		}
#endif

		lights.push_back(light);
		lightsByUUID[light->uuid()] = light;

		if (light->hasShadowMaps()) light->CreateShadowMap();

		return light;
	}

	std::string Light::uuid()
	{
		return json.at("uuid");
	}

	void Light::uuid(std::string uuid)
	{
		json.at("uuid") = uuid;
	}

	std::string Light::name()
	{
		return json.at("name");
	}

	void Light::name(std::string name)
	{
		json.at("name") = name;
	}

	LightType Light::lightType()
	{
		return StrToLightType.at(json.at("lightType"));
	}

	void Light::lightType(LightType lightType)
	{
		json.at("lightType") = LightTypeToStr.at(lightType);
	}

	XMFLOAT3 Light::position()
	{
		return XMFLOAT3(json.at("position").at(0), json.at("position").at(1), json.at("position").at(2));
	}

	void Light::position(XMFLOAT3 f3)
	{
		nlohmann::json& j = json.at("position");
		j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
	}

	void Light::position(nlohmann::json f3)
	{
		json.at("position") = f3;
	}

	XMFLOAT3 Light::rotation()
	{
		return XMFLOAT3(json.at("rotation").at(0), json.at("rotation").at(1), 0.0f);
	}

	void Light::rotation(XMFLOAT3 f3)
	{
		nlohmann::json& j = json.at("rotation");
		j.at(0) = f3.x; j.at(1) = f3.y;
	}

	void Light::rotation(nlohmann::json f3)
	{
		json.at("rotation") = { f3.at(0), f3.at(0) };
	}

	XMFLOAT3 Light::color()
	{
		return XMFLOAT3(json.at("color").at(0), json.at("color").at(1), json.at("color").at(2));
	}

	void Light::color(XMFLOAT3 f3)
	{
		nlohmann::json& j = json.at("color");
		j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
	}

	void Light::color(nlohmann::json f3)
	{
		json.at("color") = f3;
	}

	float Light::brightness()
	{
		return json.at("brightness");
	}

	void Light::brightness(float brightness)
	{
		json.at("brightness") = brightness;
	}

	XMFLOAT3 Light::attenuation()
	{
		return XMFLOAT3(json.at("attenuation").at(0), json.at("attenuation").at(1), json.at("attenuation").at(2));
	}

	void Light::attenuation(XMFLOAT3 f3)
	{
		nlohmann::json& j = json.at("attenuation");
		j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
	}

	void Light::attenuation(nlohmann::json f3)
	{
		json.at("attenuation") = f3;
	}

	float Light::coneAngle()
	{
		return json.at("coneAngle");
	}

	void Light::coneAngle(float coneAngle)
	{
		json.at("coneAngle") = coneAngle;
	}

	bool Light::hasShadowMaps()
	{
		return json.at("hasShadowMaps");
	}

	void Light::hasShadowMaps(bool hasShadowMaps)
	{
		json.at("hasShadowMaps") = hasShadowMaps;
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

	unsigned int Light::shadowMapWidth()
	{
		return static_cast<unsigned int>(json.at("shadowMapWidth"));
	}

	void Light::shadowMapWidth(unsigned int shadowMapWidth)
	{
		json.at("shadowMapWidth") = shadowMapWidth;
	}

	unsigned int Light::shadowMapHeight()
	{
		return static_cast<unsigned int>(json.at("shadowMapHeight"));
	}

	void Light::shadowMapHeight(unsigned int shadowMapHeight)
	{
		json.at("shadowMapHeight") = shadowMapHeight;
	}

	float Light::viewWidth()
	{
		return static_cast<float>(json.at("viewHeight"));
	}

	void Light::viewWidth(float viewWidth)
	{
		json.at("viewHeight") = viewWidth;
	}

	float Light::viewHeight()
	{
		return static_cast<float>(json.at("viewHeight"));
	}

	void Light::viewHeight(float viewHeight)
	{
		json.at("viewHeight") = viewHeight;
	}

	float Light::nearZ()
	{
		return static_cast<float>(json.at("nearZ"));
	}

	void Light::nearZ(float nearZ)
	{
		json.at("nearZ") = nearZ;
	}

	float Light::farZ()
	{
		return static_cast<float>(json.at("farZ"));
	}

	void Light::farZ(float farZ)
	{
		json.at("farZ") = farZ;
	}

	float Light::zBias()
	{
		return json.at("zBias");
	}

	void Light::zBias(float zBias)
	{
		json.at("zBias") = zBias;
	}

	//READ&GET
	std::vector<std::shared_ptr<Light>> GetLights() {
		return lights;
	}

	std::shared_ptr<Light> GetLight(std::string uuid) {
		return lightsByUUID.at(uuid);
	}

	std::vector<std::string> GetLightsNames() {
		std::vector<std::string> lightsNames;
		std::transform(lights.begin(), lights.end(), std::back_inserter(lightsNames), [](auto& l)
			{
				return l->name();
			}
		);
		return lightsNames;
	}

#if defined(_EDITOR)
	std::vector<UUIDName> GetLightsUUIDNames()
	{
		return GetSceneObjectsUUIDsNames(lightsByUUID);
	}
#endif

	std::shared_ptr<ConstantsBuffer> GetLightsConstantsBuffer() {
		return lightsCbv;
	}

#if defined(_EDITOR)
	std::string GetLightName(std::string uuid)
	{
		return lightsByUUID.at(uuid)->name();
	}
#endif

	//UPDATE
	void LightsStep()
	{
#if defined(_EDITOR)
		std::vector<std::shared_ptr<Light>> lightsToDestroyShadowMaps;
		std::vector<std::shared_ptr<Light>> lightsToDestroyShadowMapsMinMaxChain;
		std::vector<std::shared_ptr<Light>> lightsToCreateShadowMaps;
		std::vector<std::shared_ptr<Light>> lightsToCreateShadowMapsMinMaxChain;

		if (lightsToDestroy.size() > 0ULL)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([]
				{
					for (auto& l : lightsToDestroy)
					{
						nostd::vector_erase(lights, l);
						lightsByUUID.erase(l->uuid());

						if (l->hasShadowMaps())
						{
							l->DestroyShadowMap();
#if defined(_EDITOR)
							l->DestroyShadowMapMinMaxChain();
#endif
						}
						l->this_ptr = nullptr;
					}

					lightsToDestroy.clear();
				}
			);
		}

		bool criticalFrame = false;
		for (auto& light : GetLights())
		{
			if (light->shadowMapUpdateFlags & ShadowMapUpdateFlags_DestroyShadowMap)
			{
				lightsToDestroyShadowMaps.push_back(light);
				criticalFrame |= true;
			}
			if (light->shadowMapUpdateFlags & ShadowMapUpdateFlags_DestroyShadowMapMinMaxChain)
			{
				lightsToDestroyShadowMapsMinMaxChain.push_back(light);
				criticalFrame |= true;
			}
			if (light->shadowMapUpdateFlags & ShadowMapUpdateFlags_CreateShadowMap)
			{
				lightsToCreateShadowMaps.push_back(light);
				criticalFrame |= true;
			}
			if (light->shadowMapUpdateFlags & ShadowMapUpdateFlags_CreateShadowMapMinMaxChain)
			{
				lightsToCreateShadowMapsMinMaxChain.push_back(light);
				criticalFrame |= true;
			}
		}

		if (criticalFrame)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([&lightsToDestroyShadowMapsMinMaxChain, &lightsToDestroyShadowMaps, &lightsToCreateShadowMaps, &lightsToCreateShadowMapsMinMaxChain]
				{
					for (auto& light : lightsToDestroyShadowMapsMinMaxChain)
					{
						light->DestroyShadowMapMinMaxChain();
						light->shadowMapUpdateFlags &= ~ShadowMapUpdateFlags_DestroyShadowMapMinMaxChain;
					}

					for (auto& light : lightsToDestroyShadowMaps)
					{
						light->DestroyShadowMap();
						light->shadowMapUpdateFlags &= ~ShadowMapUpdateFlags_DestroyShadowMap;
					}
					for (auto& light : lightsToCreateShadowMaps)
					{
						light->CreateShadowMap();
						light->shadowMapUpdateFlags &= ~ShadowMapUpdateFlags_CreateShadowMap;
					}

					for (auto& light : lightsToCreateShadowMapsMinMaxChain)
					{
						light->CreateShadowMapMinMaxChain();
						light->shadowMapUpdateFlags &= ~ShadowMapUpdateFlags_CreateShadowMapMinMaxChain;
					}
				}
			);
		}
#endif
	}

	void UpdateConstantsBufferNumLights(unsigned int backbufferIndex, unsigned int numLights)
	{
		size_t offset = lightsCbv->alignedConstantBufferSize * backbufferIndex;
		LightPool* lpool = (LightPool*)(lightsCbv->mappedConstantBuffer + offset);
		lpool->numLights.D[0] = numLights;
	}

	void UpdateConstantsBufferLightAttributes(const std::shared_ptr<Light>& light, unsigned int backbufferIndex, unsigned int lightIndex, unsigned int shadowMapIndex)
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
		for (auto& l : lights)
		{
			if (l->hasShadowMaps())
			{
				l->DestroyShadowMap();
#if defined(_EDITOR)
				l->DestroyShadowMapMinMaxChain();
#endif
			}
			l->this_ptr = nullptr;
		}
		lights.clear();
		lightsByUUID.clear();
	}

	void ResetConstantsBufferLightAttributes(unsigned int backbufferIndex)
	{
		size_t offset = lightsCbv->alignedConstantBufferSize * backbufferIndex;
		LightPool* lpool = (LightPool*)(lightsCbv->mappedConstantBuffer + offset);
		ZeroMemory(lpool, sizeof(LightPool));
	}

	//EDITOR

#if defined(_EDITOR)
	void Light::DrawEditorInformationAttributes()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "light-information-atts";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string currentName = name();
			if (ImGui::InputText("name", &currentName))
			{
				name(currentName);
			}
			ImGui::EndTable();
		}
	}

	void Light::DrawAmbientLightPanel()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		XMFLOAT3 col = color();
		ImDrawColorEdit3<XMFLOAT3>("light-ambient-color", col, [this](XMFLOAT3 col)
			{
				editorAmbient["color"] = { col.x, col.y, col.z };
				color(col);
			}
		);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		float bright = brightness();
		if (ImGui::InputFloat("Brightness", &bright))
		{
			brightness(bright);
		}
	}

	void Light::DrawDirectionalLightPanel()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		XMFLOAT3 col = color();
		ImDrawColorEdit3<XMFLOAT3>("light-directional-color", col, [this](XMFLOAT3 col)
			{
				editorDirectional["color"] = { col.x, col.y, col.z };
				color(col);
			}
		);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		float bright = brightness();
		if (ImGui::InputFloat("Brightness", &bright))
		{
			brightness(bright);
		}

		XMFLOAT3 rot3 = rotation();
		XMFLOAT2 rot2 = { rot3.x, rot3.y };
		ImDrawDegreesValues<XMFLOAT2>("light-directional-rotation", { "pitch","yaw" }, rot2, [this](XMFLOAT2 rot)
			{
				editorDirectional["rotation"] = { rot.x, rot.y, 0.0f };
				rotation(XMFLOAT3({ rot.x, rot.y, 0.0f }));
				if (hasShadowMaps())
				{
					shadowMapCameras[0]->rotation(XMFLOAT3({ rot.x, rot.y, 0.0f }));
				}
			}
		);

		ImDrawDirectionalShadowMap();
	}

	void Light::DrawSpotLightPanel()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		XMFLOAT3 col = color();
		ImDrawColorEdit3<XMFLOAT3>("light-spot-color", col, [this](XMFLOAT3 col)
			{
				editorSpot["color"] = { col.x, col.y, col.z };
				color(col);
			}
		);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		float bright = brightness();
		if (ImGui::InputFloat("Brightness", &bright))
		{
			brightness(bright);
		}

		XMFLOAT3 pos = position();
		ImDrawFloatValues<XMFLOAT3>("light-spot-position", { "x","y","z" }, pos, [this](XMFLOAT3 pos)
			{
				position(pos);
				editorSpot["position"] = { pos.x, pos.y, pos.z };
				if (hasShadowMaps())
				{
					shadowMapCameras[0]->position(pos);
				}
			}
		);

		XMFLOAT3 rot3 = rotation();
		XMFLOAT2 rot2 = { rot3.x, rot3.y };
		ImDrawDegreesValues<XMFLOAT2>("light-spot-rotation", { "pitch","yaw" }, rot2, [this](XMFLOAT2 rot)
			{
				editorSpot["rotation"] = { rot.x, rot.y, 0.0f };
				rotation(XMFLOAT3({ rot.x, rot.y, 0.0f }));
				if (hasShadowMaps())
				{
					shadowMapCameras[0]->rotation(XMFLOAT3({ rot.x, rot.y, 0.0f }));
				}
			}
		);

		float coneA = coneAngle();
		ImDrawDegreesValues<float>("light-spot-coneangle", { "Cone Angle" }, coneA, [this](float coneA)
			{
				editorSpot["coneAngle"] = coneA;
				coneAngle(coneA);
				if (hasShadowMaps())
				{
					UpdateShadowMapCameraProperties();
				}
			}, 10.0f, 150.0f
		);

		XMFLOAT3 atte = attenuation();
		ImDrawFloatValues<XMFLOAT3>("light-spot-attenuation", { "C","X","X\xC2\xB2" }, atte, [this](XMFLOAT3 atte)
			{
				editorSpot["attenuation"] = { atte.x, atte.y, atte.z };
				attenuation(atte);
			}
		);

		ImDrawSpotShadowMap();
	}

	void Light::DrawPointLightPanel()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		XMFLOAT3 col = color();
		ImDrawColorEdit3<XMFLOAT3>("light-point-color", col, [this](XMFLOAT3 col)
			{
				editorPoint["color"] = { col.x, col.y, col.z };
				color(col);
			}
		);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		float bright = brightness();
		if (ImGui::InputFloat("Brightness", &bright))
		{
			brightness(bright);
		}

		XMFLOAT3 pos = position();
		ImDrawFloatValues<XMFLOAT3>("light-point-position", { "x","y","z" }, pos, [this](XMFLOAT3 pos)
			{
				position(pos);
				editorPoint["position"] = { pos.x, pos.y, pos.z };
				if (hasShadowMaps())
				{
					for (auto& c : shadowMapCameras)
					{
						c->position(pos);
					}
				}
			}
		);

		XMFLOAT3 atte = attenuation();
		ImDrawFloatValues<XMFLOAT3>("light-point-attenuation", { "C","X","X\xC2\xB2" }, atte, [this](XMFLOAT3 atte)
			{
				editorPoint["attenuation"] = { atte.x, atte.y, atte.z };
				attenuation(atte);
			}
		);

		ImDrawPointShadowMap();
	}

	void SelectLight(std::string uuid, std::string& edSO)
	{
		std::shared_ptr<Light> light = lightsByUUID.at(uuid);

		if (light->hasShadowMaps())
		{
			light->shadowMapUpdateFlags |= ShadowMapUpdateFlags_CreateShadowMapMinMaxChain;
		}
		edSO = uuid;
	}

	void DeSelectLight(std::string& edSO)
	{
		std::shared_ptr<Light> light = lightsByUUID.at(edSO);
		if (light->hasShadowMaps())
		{
			light->shadowMapUpdateFlags |= ShadowMapUpdateFlags_DestroyShadowMapMinMaxChain;
		}
		edSO = "";
	}

	static void ReplaceLightsJsonAttributes(nlohmann::json& dst, nlohmann::json src, nlohmann::json attributesToAdd, nlohmann::json attributesToDelete)
	{
		for (auto it = attributesToDelete.begin(); it != attributesToDelete.end(); it++)
		{
			dst.erase(it.key());
		}
		for (auto it = attributesToAdd.begin(); it != attributesToAdd.end(); it++)
		{
			dst[it.key()] = src[it.key()];
		}
	}

	static const std::map<LightType, nlohmann::json> lightMapJsonAttributes = {
		{ LT_Ambient, editorDefaultAmbient },
		{ LT_Directional, editorDefaultDirectional },
		{ LT_Spot, editorDefaultSpot },
		{ LT_Point, editorDefaultPoint },
	};

	static std::map<LightType, nlohmann::json> defaultShadowMapParameters = {
		{ LT_Directional, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"viewWidth", 32.0f}, {"viewHeight",32.0f},{"nearZ",0.01f}, {"farZ",1000.0f}}},
		{ LT_Spot, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"viewWidth", 32.0f}, {"viewHeight",32.0f},{"nearZ",0.01f}, {"farZ",100.0f}} },
		{ LT_Point, {{ "shadowMapWidth",1024}, {"shadowMapHeight",1024}, {"nearZ",0.01f}, {"farZ",20.0f}} },
	};

	static std::vector<std::string> shadowMapJsonAttributes = {
		"shadowMapWidth", "shadowMapHeight", "viewWidth", "viewHeight", "nearZ", "farZ",
	};

	void DrawLightPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop)
	{
		std::string tableName = "light-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			std::shared_ptr<Light> light = lightsByUUID.at(uuid);
			light->DrawEditorInformationAttributes();

			std::vector<std::string> selectables = LightTypesStr;
			std::string selected = light->json.at("lightType");
			nlohmann::json lightAttributes = lightMapJsonAttributes.at(light->lightType());

			DrawComboSelection(selected, selectables, [light, lightAttributes](std::string lightTypeStr)
				{
					light->lightType(StrToLightType.at(lightTypeStr));
					RemoveJsonAttributes(light->json, shadowMapJsonAttributes);
					switch (light->lightType())
					{
					case LT_Ambient:
					{
						ReplaceLightsJsonAttributes(light->json, light->editorAmbient, editorDefaultAmbient, lightAttributes);
					}
					break;
					case LT_Directional:
					{
						ReplaceLightsJsonAttributes(light->json, light->editorDirectional, editorDefaultDirectional, lightAttributes);
						if (light->hasShadowMaps())
						{
							light->json.merge_patch(defaultShadowMapParameters.at(LT_Directional));
							light->shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
						}
					}
					break;
					case LT_Spot:
					{
						ReplaceLightsJsonAttributes(light->json, light->editorSpot, editorDefaultSpot, lightAttributes);
						if (light->hasShadowMaps())
						{
							light->json.merge_patch(defaultShadowMapParameters.at(LT_Spot));
							light->shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
						}
					}
					break;
					case LT_Point:
					{
						ReplaceLightsJsonAttributes(light->json, light->editorPoint, editorDefaultPoint, lightAttributes);
						if (light->hasShadowMaps())
						{
							light->json.merge_patch(defaultShadowMapParameters.at(LT_Point));
							light->shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
						}
					}
					break;
					}
				},
				"lightType"
			);

			switch (light->lightType())
			{
			case LT_Ambient:
			{
				light->DrawAmbientLightPanel();
			}
			break;
			case LT_Directional:
			{
				light->DrawDirectionalLightPanel();
			}
			break;
			case LT_Spot:
			{
				light->DrawSpotLightPanel();
			}
			break;
			case LT_Point:
			{
				light->DrawPointLightPanel();
			}
			break;
			}

			ImGui::EndTable();
		}
	}

	void CreateNewLight()
	{
		Light::popupModalId = LightPopupModal_CreateNew;
		Light::creationJson = nlohmann::json(
			{
			{ "name" , "" },
			{ "lightType", LightTypeToStr.at(LT_Ambient) }
			}
		);
	}

	void DeleteLight(std::string uuid)
	{
		lightsToDestroy.push_back(lightsByUUID.at(uuid));
	}

	void DrawLightsPopups()
	{
		Editor::DrawCreateWindow(Light::popupModalId, LightPopupModal_CreateNew, "New Light", [](auto OnCancel)
			{
				ImGui::PushID("light-name");
				{
					ImGui::Text("Name");
					ImDrawJsonInputText(Light::creationJson, "name");
				}
				ImGui::PopID();

				int current_item = StrToLightType.at(Light::creationJson.at("lightType"));

				ImGui::PushID("light-type-selector");
				{
					ImGui::Text("Select light type");

					DrawComboSelection(LightTypesStr[current_item], LightTypesStr, [](std::string lightType)
						{
							Light::creationJson.at("lightType") = lightType;
						}, "type"
					);
				}
				ImGui::PopID();

				if (ImGui::Button("Cancel")) { OnCancel(); }
				ImGui::SameLine();

				bool disabledCreate = (
					Light::creationJson.at("name") == "" ||
					NameCollideWithSceneObjects(lightsByUUID, Light::creationJson)
					);

				DrawItemWithEnabledState([]
					{
						if (ImGui::Button("Create"))
						{
							Light::popupModalId = 0;

							nlohmann::json r = {
								{ "uuid", getUUID() },
								{ "name", Light::creationJson.at("name") },
								{ "lightType", Light::creationJson.at("lightType") },
							};
							CreateLight(r);
						}
					}, !disabledCreate
				);
			}
		);
	}

	bool GetLightPopupIsOpen()
	{
		return !!Light::popupModalId;
	}

	void WriteLightsJson(nlohmann::json& json)
	{
		std::map<std::string, std::shared_ptr<Light>> filtered;
		std::copy_if(lightsByUUID.begin(), lightsByUUID.end(), std::inserter(filtered, filtered.end()), [](const auto& pair)
			{
				auto& [uuid, light] = pair;
				return !light->hidden();
			}
		);
		std::transform(filtered.begin(), filtered.end(), std::back_inserter(json), [](const auto& pair)
			{
				auto& [uuid, light] = pair;
				nlohmann::json ret = light->json;
				ret["uuid"] = uuid;
				return ret;
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