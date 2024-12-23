#pragma once

#include <ppltasks.h>
#include "AmbientLight.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "../Renderable/Renderable.h"
#include "../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include <widemath.h>

//let's explain a little bit here
// 1.- One CVB of MaxLights Descriptors is created to write the data used
// by the shaders rendering(hence it needs to have NumFrames times the size)
// I also think that this should be moved to the "RenderPass" once this makes it here
// 2.- each light creates a DSV Heap + DepthStencil resource
// 3.- these DepthStencil resources for each light are then used to be mapped into
// a CBV descriptor as SRV unbounded slots

namespace Scene::Camera { struct Camera; };
struct Renderer;
namespace Scene::Lights {

	static const std::wstring LightConstantBufferName = L"lights";
	static const std::wstring ShadowMapConstantBufferName = L"shadowMaps";
	static const std::wstring ShadowMapLightsShaderResourveViewName = L"shadowMapsTextures";
	static const UINT MaxLights = 100U;

	enum LightType {
		Ambient,
		Directional,
		Spot,
		Point
	};

	static std::vector<std::wstring> LightTypesStr = {
		L"Ambient",
		L"Directional",
		L"Spot",
		L"Point"
	};

	static std::map<LightType, std::wstring> LightTypeToStr = {
		{ Ambient,  L"Ambient" },
		{ Directional, L"Directional" },
		{ Spot, L"Spot" },
		{ Point, L"Point" }
	};

	static std::map<std::wstring, LightType> StrToLightType = {
		{ L"Ambient",	Ambient },
		{ L"Directional",	Directional },
		{ L"Spot", Spot },
		{ L"Point",	Point }
	};

	struct LightAttributes {
		LightType lightType;
		XMFLOAT3 lightColor;
		XMFLOAT4 atts1;
		XMFLOAT4 atts2;
		XMFLOAT3 atts3;
		BOOL hasShadowMap;
	};

	struct LightPool {
		uint128_t numLights;
		LightAttributes lights[MaxLights];
	};

	struct LightDefinition {
		std::wstring name = L"Light";
		LightType lightType = Ambient;
		AmbientLight ambient = {};
		DirectionalLight directional = {};
		SpotLight spot = {};
		PointLight point = {};
		bool hasShadowMap = false;
		DirectionalLightShadowMapParams directionalLightShadowMapParams = {};
		SpotLightShadowMapParams spotLightShadowMapParams = {};
		PointLightShadowMapParams pointLightShadowMapParams = {};
	};

	struct ShadowMapAttributes {
		XMMATRIX atts0;
		XMMATRIX atts1;
		XMMATRIX atts2;
		XMMATRIX atts3;
		XMMATRIX atts4;
		XMMATRIX atts5;
		XMFLOAT4 atts6;
	};

	struct Light
	{
		Light(){}
		~Light() { this_ptr = nullptr; } //will this work?
		std::shared_ptr<Light> this_ptr = nullptr; //dumb but efective

		std::wstring name = L"";
		LightType lightType = Ambient;

		union {
			AmbientLight ambient;
			DirectionalLight directional;
			SpotLight spot;
			PointLight point;
		};

		union {
			DirectionalLightShadowMap directionalShadowMap;
			SpotLightShadowMap spotShadowMap;
			PointLightShadowMap pointShadowMap;
		};
		bool hasShadowMaps = false;
		nlohmann::json shadowMapParams;

		//DSV Heap
		UINT shadowMapDescriptorSize;
		CComPtr<ID3D12DescriptorHeap> shadowMapDsvDescriptorHeap;
		CD3DX12_CPU_DESCRIPTOR_HANDLE shadowMapDsvCpuHandle;
		CComPtr<ID3D12Resource> shadowMap;

		//Camera
		std::vector<std::shared_ptr<Scene::Camera::Camera>> shadowMapCameras;

#if defined(_EDITOR)
		nlohmann::json json();
#endif
	};

	//lights and shadow map CBV data
	ConstantsBufferViewDataPtr GetLightsConstantBufferView();
	ConstantsBufferViewDataPtr GetShadowMapConstantBufferView();
	CD3DX12_GPU_DESCRIPTOR_HANDLE& GetShadowMapGpuDescriptorHandleStart();

	Concurrency::task<void> CreateLightsResources();
	void DestroyLightsResources();

	void UpdateConstantsBufferNumLights(UINT backbufferIndex, UINT numLights);
	void UpdateConstantsBufferLightAttributes(std::shared_ptr<Light>& light, UINT backbufferIndex, UINT lightIndex);
	void UpdateConstantsBufferShadowMapAttributes(std::shared_ptr<Light>& light, UINT backbufferIndex, UINT shadowMapIndex);

	typedef void LoadLightFn(std::shared_ptr<Light> light);

	std::shared_ptr<Light> CreateLight(nlohmann::json light);
	std::vector<std::shared_ptr<Light>> GetLights();
	std::shared_ptr<Light> GetLight(std::wstring lightName);
	void DestroyLights();
	
	void CreateDirectionalLightShadowMap(const std::shared_ptr<Light>& light, nlohmann::json params);
	void CreateSpotLightShadowMap(const std::shared_ptr<Light>& light, nlohmann::json params);
	void CreatePointLightShadowMap(const std::shared_ptr<Light>& light, nlohmann::json params);
	void CreateShadowMapDepthStencilResource(const std::shared_ptr<Light>& light);
	void RenderShadowMap(std::shared_ptr<Light> light, std::function<void(UINT)> renderScene);

	Concurrency::task<void> CreateShadowMapPipeline(const std::map<VertexClass, const MaterialPtr> shadowMapsInputLayoutMaterial);

	std::pair<CComPtr<ID3D12RootSignature>, CComPtr<ID3D12PipelineState>> GetShadowMapRenderAttributes(VertexClass vertexClass, const MaterialPtr material);
	void DestroyShadowMapAttributes();
	
};

typedef Scene::Lights::Light LightT;
typedef std::shared_ptr<LightT> LightPtr;

