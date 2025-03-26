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
#include <Editor.h>
#endif
#include <Json.h>

extern std::shared_ptr<Renderer> renderer;

namespace Scene {

	using namespace DeviceUtils;
#if defined(_EDITOR)
	using namespace Editor;
#endif

	static std::map<std::string, std::shared_ptr<Light>> lightsByName;
	static std::vector<std::shared_ptr<Light>> lights;
	static std::shared_ptr<ConstantsBuffer> lightsCbv = nullptr; //CBV for lights pool
#if defined(_EDITOR)
	static std::vector<std::shared_ptr<Light>> lightsToDestroy;
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
		}
		break;
		case LT_Spot:
		{
			light->editorSpot = light->json;
		}
		break;
		case LT_Point:
		{
			light->editorPoint = light->json;
		}
		break;
		}
#endif

		lights.push_back(light);
		lightsByName[light->name()] = light;

		if (light->hasShadowMaps()) light->CreateShadowMap();

		return light;
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
		return XMFLOAT3(json.at("rotation").at(0), json.at("rotation").at(1), json.at("rotation").at(2));
	}

	void Light::rotation(XMFLOAT3 f3)
	{
		nlohmann::json& j = json.at("rotation");
		j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
	}

	void Light::rotation(nlohmann::json f3)
	{
		json.at("rotation") = f3;
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

	//READ&GET
	std::vector<std::shared_ptr<Light>> GetLights() {
		return lights;
	}

	std::shared_ptr<Light> GetLight(std::string lightName) {
		return lightsByName.at(lightName);
	}

	std::vector<std::string> GetLightsNames() {
		return nostd::GetKeysFromMap(lightsByName);
	}

	std::shared_ptr<ConstantsBuffer> GetLightsConstantsBuffer() {
		return lightsCbv;
	}

#if defined(_EDITOR)
	std::string GetLightName(void* ptr)
	{
		Light* l = (Light*)ptr;
		return l->name();
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
			renderer->RenderCriticalFrame([]
				{
					for (auto& l : lightsToDestroy)
					{
						nostd::vector_erase(lights, l);
						lightsByName.erase(l->name());

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

		for (auto& light : GetLights())
		{
			if (light->shadowMapUpdateFlags & ShadowMapUpdateFlags_DestroyShadowMap)
			{
				lightsToDestroyShadowMaps.push_back(light);
			}
			if (light->shadowMapUpdateFlags & ShadowMapUpdateFlags_DestroyShadowMapMinMaxChain)
			{
				lightsToDestroyShadowMapsMinMaxChain.push_back(light);
			}
			if (light->shadowMapUpdateFlags & ShadowMapUpdateFlags_CreateShadowMap)
			{
				lightsToCreateShadowMaps.push_back(light);
			}
			if (light->shadowMapUpdateFlags & ShadowMapUpdateFlags_CreateShadowMapMinMaxChain)
			{
				lightsToCreateShadowMapsMinMaxChain.push_back(light);
			}
		}

		if (lightsToDestroyShadowMaps.size() > 0ULL || lightsToDestroyShadowMapsMinMaxChain.size() > 0ULL)
		{
			renderer->Flush();
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
			}
		}

		if (lightsToCreateShadowMaps.size() > 0ULL || lightsToCreateShadowMapsMinMaxChain.size() > 0ULL)
		{
			renderer->RenderCriticalFrame([&lightsToCreateShadowMaps, &lightsToCreateShadowMapsMinMaxChain]
				{
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
			atts.lightColor = light->color();
		}
		break;
		case LT_Directional:
		{
			XMFLOAT3 rot = light->rotation();
			atts.lightColor = light->color();
			atts.atts1 = {
				sinf(rot.x) * cosf(rot.y),
				sinf(rot.y),
				cosf(rot.x) * cosf(rot.y),
				0.0f
			};
			atts.hasShadowMap = light->hasShadowMaps();
			atts.shadowMapIndex = light->shadowMapIndex;
		}
		break;
		case LT_Spot:
		{
			XMFLOAT3 pos = light->position();
			XMFLOAT3 rot = light->rotation();
			XMFLOAT3 atte = light->attenuation();
			atts.lightColor = light->color();
			atts.atts1 = { pos.x, pos.y, pos.z, 0.0f };
			atts.atts2 = {
				sinf(rot.x) * cosf(rot.y),
				sinf(rot.y),
				cosf(rot.x) * cosf(rot.y),
				cosf(light->coneAngle())
			};
			atts.atts3 = { atte.x, atte.y, atte.z };
			atts.hasShadowMap = light->hasShadowMaps();
			atts.shadowMapIndex = light->shadowMapIndex;
		}
		break;
		case LT_Point:
		{
			XMFLOAT3 pos = light->position();
			XMFLOAT3 atte = light->attenuation();
			atts.lightColor = light->color();
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
		lightsByName.clear();
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
				if (!lightsByName.contains(currentName))
				{
					lightsByName[currentName] = lightsByName[name()];
					lightsByName.erase(name());
				}
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

		XMFLOAT3 rot3 = rotation();
		XMFLOAT2 rot2 = { rot3.x, rot3.y };
		ImDrawDegreesValues<XMFLOAT2>("light-directional-rotation", { "azimuthal","polar" }, rot2, [this](XMFLOAT2 rot)
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
		ImDrawDegreesValues<XMFLOAT2>("light-spot-rotation", { "azimuthal","polar" }, rot2, [this](XMFLOAT2 rot)
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
			}
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

		XMFLOAT3 pos = position();
		ImDrawFloatValues<XMFLOAT3>("light-point-position", { "x","y","z" }, pos, [this](XMFLOAT3 pos)
			{
				position(pos);
				editorPoint["position"] = { pos.x, pos.y, pos.z };
				if (hasShadowMaps())
				{
					shadowMapCameras[0]->position(pos);
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

	/*
	nlohmann::json Light::json() {
		nlohmann::json j = nlohmann::json({});

		j["name"] = name;
		j["lightType"] = LightTypesStr[lightType];

		auto buildJsonAmbientLight = [this](auto& j) {
			j["ambient"]["color"] = { ambient.color.x, ambient.color.y, ambient.color.z };
			};

		auto buildJsonDirectionalLight = [this](auto& j) {
			j["directional"]["color"] = { directional.color.x, directional.color.y, directional.color.z };
			j["directional"]["distance"] = directional.distance;
			j["directional"]["rotation"] = { directional.rotation.x, directional.rotation.y };
			j["directional"]["hasShadowMaps"] = hasShadowMaps;
			if (hasShadowMaps) {
				j["directional"]["directionalShadowMap"]["shadowMapWidth"] = shadowMapParams.shadowMapWidth;
				j["directional"]["directionalShadowMap"]["shadowMapHeight"] = shadowMapParams.shadowMapHeight;
				j["directional"]["directionalShadowMap"]["viewWidth"] = shadowMapParams.viewWidth;
				j["directional"]["directionalShadowMap"]["viewHeight"] = shadowMapParams.viewHeight;
				j["directional"]["directionalShadowMap"]["nearZ"] = shadowMapParams.nearZ;
				j["directional"]["directionalShadowMap"]["farZ"] = shadowMapParams.farZ;
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
				j["spot"]["spotShadowMap"]["shadowMapWidth"] = shadowMapParams.shadowMapWidth;
				j["spot"]["spotShadowMap"]["shadowMapHeight"] = shadowMapParams.shadowMapHeight;
				j["spot"]["spotShadowMap"]["viewWidth"] = shadowMapParams.viewWidth;
				j["spot"]["spotShadowMap"]["viewHeight"] = shadowMapParams.viewHeight;
				j["spot"]["spotShadowMap"]["nearZ"] = shadowMapParams.nearZ;
				j["spot"]["spotShadowMap"]["farZ"] = shadowMapParams.farZ;
			}
			};

		auto buildJsonPointLight = [this](auto& j) {
			j["point"]["color"] = { point.color.x, point.color.y, point.color.z };
			j["point"]["position"] = { point.position.x, point.position.y, point.position.z };
			j["point"]["attenuation"] = { point.attenuation.x, point.attenuation.y, point.attenuation.z };
			j["point"]["hasShadowMaps"] = hasShadowMaps;
			if (hasShadowMaps) {
				j["point"]["pointShadowMap"]["shadowMapWidth"] = shadowMapParams.shadowMapWidth;
				j["point"]["pointShadowMap"]["shadowMapHeight"] = shadowMapParams.shadowMapHeight;
				j["point"]["pointShadowMap"]["nearZ"] = shadowMapParams.nearZ;
				j["point"]["pointShadowMap"]["farZ"] = shadowMapParams.farZ;
			}
			};

		switch (lightType)
		{
		case LT_Ambient:
		{
			buildJsonAmbientLight(j);
		}
		break;
		case LT_Directional:
		{
			buildJsonDirectionalLight(j);
		}
		break;
		case LT_Spot:
		{
			buildJsonSpotLight(j);
		}
		break;
		case LT_Point:
		{
			buildJsonPointLight(j);
		}
		break;
		}

		//using namespace Animation::Effects;
		//auto effects = GetLightEffects(this_ptr);
		//if (!effects.empty()) {
		//	j["effects"] = effects;
		//}

	return j;
}
*/

	void SelectLight(std::string lightName, void*& ptr)
	{
		std::shared_ptr<Light> light = lightsByName.at(lightName);

		if (light->hasShadowMaps())
		{
			light->shadowMapUpdateFlags |= ShadowMapUpdateFlags_CreateShadowMapMinMaxChain;
		}
		ptr = light.get();
	}

	void DeSelectLight(void*& ptr)
	{
		Light* light = (Light*)ptr;
		if (light->hasShadowMaps())
		{
			light->shadowMapUpdateFlags |= ShadowMapUpdateFlags_DestroyShadowMapMinMaxChain;
		}
		ptr = nullptr;
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

	void DrawLightPanel(void*& ptr, ImVec2 pos, ImVec2 size, bool pop)
	{
		std::string tableName = "light-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			Light* light = (Light*)ptr;
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

	void DeleteLight(std::string name)
	{
		lightsToDestroy.push_back(lightsByName.at(name));
	}

	void DrawLightsPopups()
	{
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