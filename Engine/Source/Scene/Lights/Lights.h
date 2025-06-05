#pragma once

#include <ppltasks.h>
#include "../Renderable/Renderable.h"
#include "../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include "ShadowMap.h"

//let's explain a little bit here
// 1.- One CVB of MaxLights Descriptors is created to write the data used
// by the shaders rendering(hence it needs to have NumFrames times the size)
// I also think that this should be moved to the "RenderPass" once this makes it here
// 2.- each light creates a DSV Heap + DepthStencil resource
// 3.- these DepthStencil resources for each light are then used to be mapped into
// a CBV descriptor as SRV unbounded slots

namespace Scene { struct Camera; };
namespace RenderPass { struct RenderToTexturePass; };

namespace Scene {

	using namespace DirectX;
	using namespace DeviceUtils;
	using namespace RenderPass;

#if defined(_EDITOR)
	enum Light_PopupModal
	{
		LightPopupModal_CannotDelete = 1,
		LightPopupModal_CreateNew = 2
	};

	static const nlohmann::json editorDefaultAmbient = {
		{ "color", { 0.3f, 0.3f, 0.3f} }
	};

	static const nlohmann::json editorDefaultDirectional = {
		{ "color", { 1.0f, 1.0f, 1.0f } },
		{ "rotation", { 0.0f, 3.141592654f } }
	};

	static const nlohmann::json editorDefaultSpot = {
		{ "color", { 1.0f, 1.0f, 1.0f } },
		{ "position", { 0.0f, 10.0f, 0.0f } },
		{ "rotation", { 0.0f, 3.141592654f } },
		{ "coneAngle", 0.523599 },
		{ "attenuation" , { 0.0f, 0.001f, 0.0001f } }
	};

	static const nlohmann::json editorDefaultPoint = {
		{ "color", { 1.0f, 1.0f, 1.0f } },
		{ "position", { 0.0f, 10.0f, 0.0f } },
		{ "attenuation" , { 0.0f, 0.001f, 0.0001f } }
	};

#endif

	static constexpr XMVECTOR PointLightDirection[] = {
	{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f,-1.0f, 0.0f, 0.0f },
	{ 1.0f, 0.0f, 0.0f, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f },
	};
	static constexpr XMVECTOR PointLightUp[] = {
		{ 0.0f, 0.0f,-1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f },
	};

	inline static const std::string LightConstantBufferName = "lights";
	inline static const std::string ShadowMapConstantBufferName = "shadowMaps";
	inline static const std::string ShadowMapLightsShaderResourceViewName = textureShaderUsageToStr.at(TextureShaderUsage_ShadowMaps);
	inline static const unsigned int MaxLights = 100U;

	enum LightType {
		LT_Ambient,
		LT_Directional,
		LT_Spot,
		LT_Point
	};

	static std::vector<std::string> LightTypesStr = {
		"Ambient",
		"Directional",
		"Spot",
		"Point"
	};

	static std::map<LightType, std::string> LightTypeToStr = {
		{ LT_Ambient, "Ambient" },
		{ LT_Directional, "Directional" },
		{ LT_Spot, "Spot" },
		{ LT_Point, "Point" }
	};

	static std::map<std::string, LightType> StrToLightType = {
		{ "Ambient",	LT_Ambient },
		{ "Directional",	LT_Directional },
		{ "Spot", LT_Spot },
		{ "Point",	LT_Point }
	};

	struct LightAttributes {
		LightType lightType; //8
		XMFLOAT3 lightColor; //32
		XMFLOAT4 atts1; //64
		XMFLOAT4 atts2; //96
		XMFLOAT3 atts3; //120
		bool hasShadowMap; //128
		unsigned int shadowMapIndex; //136
		XMFLOAT3 _pad; //160
	};

	struct LightPool {
		uint128_t numLights;
		LightAttributes lights[MaxLights];
	};

	struct Light
	{
		Light() {
#if defined(_EDITOR)
			editorAmbient = editorDefaultAmbient;
			editorDirectional = editorDefaultDirectional;
			editorSpot = editorDefaultSpot;
			editorPoint = editorDefaultPoint;
#endif
		}

		~Light() {}

		std::shared_ptr<Light> this_ptr = nullptr; //dumb but efective
		nlohmann::json json;

		std::string uuid();
		void uuid(std::string uuid);

		std::string name();
		void name(std::string name);

		LightType lightType();
		void lightType(LightType lightType);

		XMFLOAT3 position();
		void position(XMFLOAT3 f3);
		void position(nlohmann::json f3);

		XMFLOAT3 rotation();
		void rotation(XMFLOAT3 f3);
		void rotation(nlohmann::json f3);

		XMFLOAT3 color();
		void color(XMFLOAT3 f3);
		void color(nlohmann::json f3);

		float brightness();
		void brightness(float brightness);

		XMFLOAT3 attenuation();
		void attenuation(XMFLOAT3 f3);
		void attenuation(nlohmann::json f3);

		float directionalDistance() { return 30.0f; }

		float coneAngle();
		void coneAngle(float coneAngle);

		bool hasShadowMaps();
		void hasShadowMaps(bool hasShadowMaps);

		bool hidden() { return false; }

		XMMATRIX world();

		//the following ones are calculated, so no need to store in json
		D3D12_RECT shadowMapClearScissorRect; //used in point lights
		D3D12_VIEWPORT shadowMapClearViewport; //used in point lights
		std::vector<D3D12_RECT> shadowMapScissorRect;
		std::vector<D3D12_VIEWPORT> shadowMapViewport;
		XMMATRIX shadowMapProjectionMatrix;
		XMFLOAT2 shadowMapTexelInvSize;

		std::shared_ptr<RenderToTexturePass> shadowMapRenderPass;
		unsigned int shadowMapIndex = 0xFFFFFFFF;
#if defined(_EDITOR)
		static nlohmann::json creationJson;
		static unsigned int popupModalId;
		unsigned int shadowMapUpdateFlags = 0U;
		std::vector<std::shared_ptr<RenderToTexturePass>> shadowMapMinMaxChainRenderPass;
		std::vector<std::shared_ptr<Renderable>> shadowMapMinMaxChainRenderable;
		std::shared_ptr<RenderToTexturePass> shadowMapMinMaxChainResultRenderPass = nullptr;
		std::shared_ptr<Renderable> shadowMapMinMaxChainResultRenderable = nullptr;
#endif

		//Camera
		std::vector<std::shared_ptr<Scene::Camera>> shadowMapCameras;

		//CREATE
		void CreateShadowMap();
		void CreateDirectionalLightShadowMap();
		void CreateSpotLightShadowMap();
		void CreatePointLightShadowMap();
		void CreateShadowMapDepthStencilResource();
		void CreateShadowMapShaderResourceView();
#if defined(_EDITOR)
		void CreateShadowMapMinMaxChain();
#endif

		//DESTROY
		void DestroyShadowMap();
		void DestroyShadowMapCameras();
#if defined(_EDITOR)
		void DestroyShadowMapMinMaxChain();
#endif

		//RENDER
		void RenderShadowMap(std::function<void(size_t, unsigned int)> renderScene);
#if defined(_EDITOR)
		void RenderShadowMapMinMaxChain();
#endif

		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);

		//EDITOR
#if defined(_EDITOR)
		nlohmann::json editorAmbient;
		nlohmann::json editorDirectional;
		nlohmann::json editorSpot;
		nlohmann::json editorPoint;

		void DrawEditorInformationAttributes();
		void DrawAmbientLightPanel();
		void DrawDirectionalLightPanel();
		void DrawSpotLightPanel();
		void DrawPointLightPanel();
		void ImDrawDirectionalShadowMap();
		void ImDrawSpotShadowMap();
		void ImDrawPointShadowMap();
		void ImDrawShadowMapMinMaxChain();
#endif
	};

	//CREATE
	void CreateLightsResources();
	std::shared_ptr<Light> CreateLight(nlohmann::json light);

	//READ&GET
	std::vector<std::shared_ptr<Light>> GetLights();
	std::shared_ptr<Light> GetLight(std::string uuid);
	std::vector<std::string> GetLightsNames();
	std::vector<UUIDName> GetLightsUUIDNames();
	std::shared_ptr<ConstantsBuffer> GetLightsConstantsBuffer();
#if defined(_EDITOR)
	std::string GetLightName(std::string uuid);
#endif

	//UPDATE
	void LightsStep();
	void UpdateConstantsBufferNumLights(unsigned int backbufferIndex, unsigned int numLights);
	void UpdateConstantsBufferLightAttributes(const std::shared_ptr<Light>& light, unsigned int backbufferIndex, unsigned int lightIndex, unsigned int shadowMapIndex = 0U);

	//DELETE
	void DestroyLightsResources();
	void DestroyLights();
	void ResetConstantsBufferLightAttributes(unsigned int backbufferIndex);
	void ResetConstantsBufferShadowMapAttributes(unsigned int backbufferIndex);

	//EDITOR
#if defined(_EDITOR)
	void SelectLight(std::string uuid, std::string& edSO);
	void DeSelectLight(std::string& edSO);
	void DrawLightPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop);
	void CreateNewLight();
	void DeleteLight(std::string uuid);
	void DrawLightsPopups();
	bool GetLightPopupIsOpen();
	void WriteLightsJson(nlohmann::json& json);
#endif
};

