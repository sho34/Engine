#pragma once

#include <ppltasks.h>
#include <Renderable/Renderable.h>
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include "ShadowMap.h"
#include <SceneObjectDecl.h>
#include <Json.h>
#include <SceneObject.h>
#include <JExposeTypes.h>

//let's explain a little bit here
// 1.- One CVB of MaxLights Descriptors is created to write the data used
// by the shaders rendering(hence it needs to have NumFrames times the size)
// I also think that this should be moved to the "RenderPass" once this makes it here
// 2.- each light creates a DSV Heap + DepthStencil resource
// 3.- these DepthStencil resources for each light are then used to be mapped into
// a CBV descriptor as SRV unbounded slots

namespace Scene { struct Camera; };
namespace DeviceUtils { struct RenderToTexturePass; };

namespace Scene {

	using namespace DirectX;
	using namespace DeviceUtils;

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
	inline static const std::string ShadowMapLightsShaderResourceViewName = TextureShaderUsageToString.at(TextureShaderUsage_ShadowMaps);
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

	static std::map<LightType, std::string> LightTypeToString = {
		{ LT_Ambient, "Ambient" },
		{ LT_Directional, "Directional" },
		{ LT_Spot, "Spot" },
		{ LT_Point, "Point" }
	};

	static std::map<std::string, LightType> StringToLightType = {
		{ "Ambient",	LT_Ambient },
		{ "Directional",	LT_Directional },
		{ "Spot", LT_Spot },
		{ "Point",	LT_Point }
	};

#if defined(_EDITOR)

	static const std::map<LightType, nlohmann::json> editorDefaultLightProperties = {
		{ LT_Ambient, {
			{ "color", { 0.3f, 0.3f, 0.3f} },
		}},
		{
			LT_Directional, {
			{ "color", { 1.0f, 1.0f, 1.0f } },
			{ "rotation", { -90.0f, 0.0f, 0.0 } },
			{ "shadowMapWidth", 1024 },
			{ "shadowMapHeight", 1024 },
			{ "zBias", 0.0002 },
			{ "hasShadowMaps", false },
		}},
		{	LT_Spot, {
			{ "color", { 1.0f, 1.0f, 1.0f } },
			{ "position", { 0.0f, 10.0f, 0.0f } },
			{ "rotation", { -90.0f, 0.0, 0.0 } },
			{ "coneAngle", 45.0f },
			{ "attenuation" , { 0.0f, 0.001f, 0.0001f } },
			{ "shadowMapWidth", 1024 },
			{ "shadowMapHeight", 1024 },
			{ "zBias", 0.000002 },
			{ "hasShadowMaps", false },
		}},
		{
			LT_Point, {
			{ "color", { 1.0f, 1.0f, 1.0f } },
			{ "position", { 0.0f, 10.0f, 0.0f } },
			{ "attenuation" , { 0.0f, 0.001f, 0.0001f } },
			{ "shadowMapWidth", 1024 },
			{ "shadowMapHeight", 1024 },
			{ "zBias", 0.000002 },
			{ "hasShadowMaps", false },
		}}
	};

	static inline std::map<LightType, std::vector<std::string>> shadowMapSizes =
	{
		{ LT_Directional, { "32","64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384" }},
		{ LT_Spot, { "32","64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384" }},
		{ LT_Point, { "32","64", "128", "256", "512", "1024", "2048" }}
	};

#endif

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

#include <JExposeTrackUUIDDecl.h>
#include <LightsAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttOrder.h>
#include <LightsAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttDrawersDecl.h>
#include <LightsAtt.h>
#include <JExposeEnd.h>

	struct Light : SceneObject
	{
		SCENEOBJECT_DECL(Light);

#include <JExposeAttFlags.h>
#include <LightsAtt.h>
#include <JExposeEnd.h>

#include <JExposeDecl.h>
#include <LightsAtt.h>
#include <JExposeEnd.h>

		void Destroy();

		XMMATRIX world();
		XMVECTOR fw();

		std::vector<D3D12_RECT> shadowMapScissorRect;
		std::vector<D3D12_VIEWPORT> shadowMapViewport;
		XMFLOAT2 shadowMapTexelInvSize;

		std::shared_ptr<RenderToTexturePass> shadowMapRenderPass;
		unsigned int shadowMapIndex = 0xFFFFFFFF;

		//Camera
		std::vector<std::shared_ptr<Scene::Camera>> shadowMapCameras;

		//CREATE
		void CreateShadowMap();
		nlohmann::json CreateDirectionalShadowMapCameraJson();
		void CreateDirectionalLightShadowMap();
		nlohmann::json CreateSpotShadowMapCameraJson();
		void CreateSpotLightShadowMap();
		nlohmann::json CreatePointShadowMapCameraJson(unsigned camIndex);
		void CreatePointLightShadowMap();
		void CreateShadowMapDepthStencilResource();
		void CreateShadowMapShaderResourceView();
#if defined(_EDITOR)
		virtual void EditorPreview(size_t flags);
		virtual void DestroyEditorPreview();
		std::vector<std::shared_ptr<RenderPassInstance>> shadowMapMinMaxChainRenderPass;
		std::shared_ptr<RenderPassInstance> shadowMapMinMaxChainResultRenderPass;
		void CreateShadowMapMinMaxChain();
		void DestroyShadowMapMinMaxChain();
		void RenderShadowMapMinMaxChain();
#endif
		void BindRenderablesToShadowMapCamera();

		//UPDATE
		void UpdateShadowMapCameraProperties();
		void UpdateDirectionalShadowMapCameraProperties();
		void UpdateSpotShadowMapCameraProperties();
		void UpdatePointShadowMapCameraProperties();
		void UpdateShadowMapCameraTransformation();
		void UpdateDirectionalShadowMapCameraTransformation();
		void UpdateSpotShadowMapCameraTransformation();
		void UpdatePointShadowMapCameraTransformation();

		//DESTROY
		void DestroyShadowMap();
		void DestroyShadowMapCameras();

		//RENDER
		void RenderShadowMap(std::function<void(unsigned int)> renderScene);

		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);
	};

	//CREATE
	void CreateLightsResources();

	//READ&GET
	std::shared_ptr<ConstantsBuffer> GetLightsConstantsBuffer();

	void LightsStep();
	void WriteConstantsBufferNumLights(unsigned int backbufferIndex, unsigned int numLights);
	void WriteConstantsBufferLightAttributes(const std::shared_ptr<Light>& light, unsigned int backbufferIndex, unsigned int lightIndex, unsigned int shadowMapIndex = 0U);

	//DELETE
	void DestroyLightsResources();
	void DestroyLight(std::shared_ptr<Light>& light);
	void DestroyLights();
	void ResetConstantsBufferLightAttributes(unsigned int backbufferIndex);
	void ResetConstantsBufferShadowMapAttributes(unsigned int backbufferIndex);

	//EDITOR
#if defined(_EDITOR)
	void WriteLightsJson(nlohmann::json& json);
	void RenderShadowMapMinMaxChain();
#endif
};

