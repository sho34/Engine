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
#endif

extern std::shared_ptr<Renderer> renderer;

namespace Scene {

	using namespace DeviceUtils;
#if defined(_EDITOR)
	using namespace Editor;
#endif

	static std::map<std::string, std::shared_ptr<Light>> lightsByName;
	static std::vector<std::shared_ptr<Light>> lights;
	static std::shared_ptr<ConstantsBuffer> lightsCbv = nullptr; //CBV for lights pool

	//CREATE
	void CreateLightsResources() {
		lightsCbv = CreateConstantsBuffer(sizeof(LightPool), "lightsCbv");
	}

	std::shared_ptr<Light> CreateLight(nlohmann::json lightj) {

		std::shared_ptr<Light> light = std::make_shared<Light>();
		light->this_ptr = light;
		light->name = lightj["name"];
		light->lightType = StrToLightType[lightj["lightType"]];

		switch (light->lightType)
		{
		case LT_Ambient:
		{
			light->ambient = {
				.color = JsonToFloat3(lightj["ambient"]["color"])
			};
#if defined(_EDITOR)
			light->editorAmbient = light->ambient;
#endif
		}
		break;
		case LT_Directional:
		{
			ReplaceFromJson(light->hasShadowMaps, lightj["directional"], "hasShadowMaps");
			light->directional = {
				.color = JsonToFloat3(lightj["directional"]["color"]),
				.distance = lightj["directional"]["distance"],
				.rotation = JsonToFloat2(lightj["directional"]["rotation"])
			};
#if defined(_EDITOR)
			light->editorDirectional = light->directional;
#endif
		}
		break;
		case LT_Spot:
		{
			ReplaceFromJson(light->hasShadowMaps, lightj["spot"], "hasShadowMaps");
			light->spot = {
				.color = JsonToFloat3(lightj["spot"]["color"]),
				.position = JsonToFloat3(lightj["spot"]["position"]),
				.rotation = JsonToFloat2(lightj["spot"]["rotation"]),
				.coneAngle = lightj["spot"]["coneAngle"],
				.attenuation = JsonToFloat3(lightj["spot"]["attenuation"])
			};
#if defined(_EDITOR)
			light->editorSpot = light->spot;
#endif
		}
		break;
		case LT_Point:
		{
			ReplaceFromJson(light->hasShadowMaps, lightj["point"], "hasShadowMaps");
			light->point = {
				.color = JsonToFloat3(lightj["point"]["color"]),
				.position = JsonToFloat3(lightj["point"]["position"]),
				.attenuation = JsonToFloat3(lightj["point"]["attenuation"])
			};
#if defined(_EDITOR)
			light->editorPoint = light->point;
#endif
		}
		break;
		}

		lights.push_back(light);
		lightsByName[light->name] = light;

		if (!light->hasShadowMaps) return light;

		CreateShadowMap(light, lightj);
		return light;
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
		return l->name;
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
		atts.lightType = light->lightType;

		switch (light->lightType) {
		case LT_Ambient:
		{
			atts.lightColor = light->ambient.color;
		}
		break;
		case LT_Directional:
		{
			atts.lightColor = light->directional.color;
			atts.atts1 = {
				sinf(light->directional.rotation.x) * cosf(light->directional.rotation.y),
				sinf(light->directional.rotation.y),
				cosf(light->directional.rotation.x) * cosf(light->directional.rotation.y),
				0.0f
			};
			atts.hasShadowMap = light->hasShadowMaps;
			atts.shadowMapIndex = light->shadowMapIndex;
		}
		break;
		case LT_Spot:
		{
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
			atts.shadowMapIndex = light->shadowMapIndex;
		}
		break;
		case LT_Point:
		{
			atts.lightColor = light->point.color;
			atts.atts1 = { light->point.position.x, light->point.position.y, light->point.position.z, 0.0f };
			atts.atts2 = { light->point.constant, light->point.linear, light->point.quadratic, 0.0f };
			atts.hasShadowMap = light->hasShadowMaps;
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
			if (l->hasShadowMaps)
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
			std::string currentName = name;
			if (ImGui::InputText("name", &currentName))
			{
				if (!lightsByName.contains(currentName))
				{
					lightsByName[currentName] = lightsByName[name];
					lightsByName.erase(name);
				}
				name = currentName;
			}
			ImGui::EndTable();
		}
	}

	void Light::DrawAmbientLightPanel()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImDrawColorEdit3<XMFLOAT3>("light-ambient-color", ambient.color, [this](XMFLOAT3 color)
			{
				editorAmbient.color = color;
			}
		);
	}

	void Light::DrawDirectionalLightPanel()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImDrawColorEdit3<XMFLOAT3>("light-directional-color", directional.color, [this](XMFLOAT3 color)
			{
				editorDirectional.color = color;
			}
		);

		ImDrawDegreesValues<XMFLOAT2>("light-directional-rotation", { "azimuthal","polar" }, directional.rotation, [this](XMFLOAT2 rot)
			{
				editorDirectional.rotation = rot;
				if (hasShadowMaps)
				{
					shadowMapCameras[0]->rotation = { rot.x, rot.y, 0.0f };
				}
			}
		);

		ImDrawDirectionalShadowMap();
	}

	void Light::DrawSpotLightPanel()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		ImDrawColorEdit3<XMFLOAT3>("light-spot-color", spot.color, [this](XMFLOAT3 color)
			{
				editorSpot.color = color;
			}
		);

		ImDrawFloatValues<XMFLOAT3>("light-spot-position", { "x","y","z" }, spot.position, [this](XMFLOAT3 pos)
			{
				editorSpot.position = pos;
				if (hasShadowMaps)
				{
					shadowMapCameras[0]->position = pos;
				}
			}
		);

		ImDrawDegreesValues<XMFLOAT2>("light-spot-rotation", { "azimuthal","polar" }, spot.rotation, [this](XMFLOAT2 rot)
			{
				editorSpot.rotation = rot;
				if (hasShadowMaps)
				{
					shadowMapCameras[0]->rotation = { rot.x, rot.y, 0.0f };
				}
			}
		);

		ImDrawDegreesValues<float>("light-spot-coneangle", { "Cone Angle" }, spot.coneAngle, [this](float coneAngle)
			{
				editorSpot.coneAngle = coneAngle;
			}
		);

		ImDrawFloatValues<XMFLOAT3>("light-spot-attenuation", { "C","X","X\xC2\xB2" }, spot.attenuation, [this](XMFLOAT3 attenuation)
			{
				editorSpot.attenuation = attenuation;
			}
		);

		ImDrawSpotShadowMap();
	}

	void Light::DrawPointLightPanel()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		ImDrawColorEdit3<XMFLOAT3>("light-spot-color", point.color, [this](XMFLOAT3 color)
			{
				editorPoint.color = color;
			}
		);

		ImDrawFloatValues<XMFLOAT3>("light-spot-position", { "x","y","z" }, spot.position, [this](XMFLOAT3 pos)
			{
				editorPoint.position = pos;
				if (hasShadowMaps)
				{
					shadowMapCameras[0]->position = pos;
				}
			}
		);

		ImDrawFloatValues<XMFLOAT3>("light-point-attenuation", { "C","X","X\xC2\xB2" }, point.attenuation, [this](XMFLOAT3 attenuation)
			{
				editorPoint.attenuation = attenuation;
			}
		);

		ImDrawPointShadowMap();
	}

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

		/*
		using namespace Animation::Effects;
		auto effects = GetLightEffects(this_ptr);
		if (!effects.empty()) {
			j["effects"] = effects;
		}
		*/

		return j;
	}

	void SelectLight(std::string lightName, void*& ptr)
	{
		std::shared_ptr<Light> light = lightsByName.at(lightName);

		if (light->hasShadowMaps)
		{
			light->shadowMapUpdateFlags |= ShadowMapUpdateFlags_CreateShadowMapMinMaxChain;
		}
		ptr = light.get();
	}

	void DeSelectLight(void*& ptr)
	{
		Light* light = (Light*)ptr;
		if (light->hasShadowMaps)
		{
			light->shadowMapUpdateFlags |= ShadowMapUpdateFlags_DestroyShadowMapMinMaxChain;
		}
		ptr = nullptr;
	}

	static std::map<LightType, ShadowMapParameters> defaultShadowMapParameters = {
		{ LT_Directional, {.shadowMapWidth = 1024.0f, .shadowMapHeight = 1024.0f,.viewWidth = 32.0f, .viewHeight = 32.0f,.nearZ = 0.01f, .farZ = 1000.0f} },
		{ LT_Spot, {.shadowMapWidth = 1024.0f, .shadowMapHeight = 1024.0f,.viewWidth = 32.0f, .viewHeight = 32.0f,.nearZ = 0.01f, .farZ = 100.0f } },
		{ LT_Point, {.shadowMapWidth = 1024.0f, .shadowMapHeight = 1024.0f,.viewWidth = 32.0f, .viewHeight = 32.0f,.nearZ = 0.01f, .farZ = 20.0f } },
	};

	void DrawLightPanel(void*& ptr, ImVec2 pos, ImVec2 size, bool pop)
	{
		std::string tableName = "light-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			Light* light = (Light*)ptr;
			light->DrawEditorInformationAttributes();

			std::vector<std::string> selectables = LightTypesStr;
			std::string selected = LightTypeToStr[light->lightType];

			DrawComboSelection(selected, selectables, [light](std::string lightTypeStr)
				{
					light->lightType = StrToLightType[lightTypeStr];
					switch (light->lightType)
					{
					case LT_Ambient:
					{
						light->ambient = light->editorAmbient;
					}
					break;
					case LT_Directional:
					{
						light->directional = light->editorDirectional;
						light->shadowMapParams = defaultShadowMapParameters.at(light->lightType);
						if (light->hasShadowMaps)
						{
							light->shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
						}
					}
					break;
					case LT_Spot:
					{
						light->spot = light->editorSpot;
						light->shadowMapParams = defaultShadowMapParameters.at(light->lightType);
						if (light->hasShadowMaps)
						{
							light->shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
						}
					}
					break;
					case LT_Point:
					{
						light->point = light->editorPoint;
						light->shadowMapParams = defaultShadowMapParameters.at(light->lightType);
						if (light->hasShadowMaps)
						{
							light->shadowMapUpdateFlags |= ShadowMapUpdateFlags_RebuildBoth;
						}
					}
					break;
					}
				},
				"lightType"
			);

			switch (light->lightType)
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

#endif

	void Light::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox)
	{
		if (lightType == LT_Ambient || lightType == LT_Directional)
		{
			bbox->position = { 0.0f ,0.0f,0.0f };
		}
		else if (lightType == LT_Spot)
		{
			bbox->position = spot.position;
		}
		else if (lightType == LT_Point)
		{
			bbox->position = point.position;
		}

		bbox->scale = { 0.5f, 0.5f, 0.5f };
		bbox->rotation = { 0.0f, 0.0f, 0.0f };
	}
}