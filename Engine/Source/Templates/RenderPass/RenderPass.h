#pragma once
#include <string>
#include <tuple>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
#include <JTemplate.h>
#include <TemplateDecl.h>
#include <dxgiformat.h>
#include <NoStd.h>
#include <Json.h>
#include <RenderPass/SwapChainPass.h>
#include <RenderPass/RenderToTexturePass.h>
#include <Mesh/Mesh.h>
#include <JExposeTypes.h>

namespace Scene { struct Camera; };
struct OverridePass;
using namespace Scene;

using namespace DeviceUtils;
namespace Templates { struct RenderPassInstance; };

enum RenderPassType
{
	RenderPassType_SwapChainPass,
	RenderPassType_RenderToTexturePass
};

inline static std::map<RenderPassType, std::string> RenderPassTypeToString =
{
	{ RenderPassType_SwapChainPass, "SwapChainPass"},
	{ RenderPassType_RenderToTexturePass, "RenderToTexturePass"},
};

inline static std::map<std::string, RenderPassType> StringToRenderPassType =
{
	{ "SwapChainPass", RenderPassType_SwapChainPass },
	{ "RenderToTexturePass", RenderPassType_RenderToTexturePass },
};

enum RenderPassMaterialOverride
{
	RenderPassMaterialOverride_None,
	RenderPassMaterialOverride_ShadowMap,
	RenderPassMaterialOverride_Picking
};

inline static std::map<RenderPassMaterialOverride, std::string> RenderPassMaterialOverrideToString =
{
	{ RenderPassMaterialOverride_None, "None"},
	{ RenderPassMaterialOverride_ShadowMap, "ShadowMap"},
	{ RenderPassMaterialOverride_Picking, "Picking" },
};

inline static std::map<std::string, RenderPassMaterialOverride> StringToRenderPassMaterialOverride =
{
	{ "None" , RenderPassMaterialOverride_None },
	{ "ShadowMap" , RenderPassMaterialOverride_ShadowMap },
	{ "Picking", RenderPassMaterialOverride_Picking },
};

enum RenderPassRenderCallbackOverride
{
	RenderPassRenderCallbackOverride_None,
	RenderPassRenderCallbackOverride_ToneMapping,
	RenderPassRenderCallbackOverride_Resolve,
	RenderPassRenderCallbackOverride_MinMaxChain,
	RenderPassRenderCallbackOverride_MinMaxChainResult,
};

inline static std::map<RenderPassRenderCallbackOverride, std::string> RenderPassRenderCallbackOverrideToString =
{
	{ RenderPassRenderCallbackOverride_None,	"None" },
	{ RenderPassRenderCallbackOverride_ToneMapping,	"ToneMapping" },
	{ RenderPassRenderCallbackOverride_Resolve, "Resolve" },
	{ RenderPassRenderCallbackOverride_MinMaxChain, "MinMaxChain" },
	{ RenderPassRenderCallbackOverride_MinMaxChainResult, "MinMaxChainResult" },
};

inline static std::map<std::string, RenderPassRenderCallbackOverride> StringToRenderPassRenderCallbackOverride =
{
	{ "None",	RenderPassRenderCallbackOverride_None },
	{ "ToneMapping",	RenderPassRenderCallbackOverride_ToneMapping },
	{ "Resolve", RenderPassRenderCallbackOverride_Resolve },
	{ "MinMaxChain", RenderPassRenderCallbackOverride_MinMaxChain },
	{ "MinMaxChainResult", RenderPassRenderCallbackOverride_MinMaxChainResult },
};

namespace Templates
{
#include <JExposeAttOrder.h>
#include <RenderPassAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttDrawersDecl.h>
#include <RenderPassAtt.h>
#include <JExposeEnd.h>

	struct RenderPassJson : public JTemplate
	{
		TEMPLATE_DECL(RenderPass);

#include <JExposeAttFlags.h>
#include <RenderPassAtt.h>
#include <JExposeEnd.h>

#include <JExposeDecl.h>
#include <RenderPassAtt.h>
#include <JExposeEnd.h>
	};

	TEMPDECL_FULL(RenderPass);

	struct RenderPassInstance;

	namespace RenderPass
	{
		inline static const std::string templateName = "renderpasses.json";
		void CreateMainHeap();
		void DestroyMainHeap();
		void ResizeRelease();
		void Resize(unsigned int width, unsigned int height);
	};

	std::shared_ptr<RenderPassInstance> GetRenderPassInstance(std::shared_ptr<Camera> cam, unsigned int renderPassIndex, std::string uuid, unsigned int width = 0U, unsigned int height = 0U);
	void DestroyRenderPassInstance(std::shared_ptr<RenderPassInstance>& rp);

	struct RenderPassInstance
	{
		std::shared_ptr<Camera> camera;
		std::string renderPassUUID;
		RenderPassMaterialOverride materialOverride;
		RenderPassRenderCallbackOverride renderCallbackOverride;
		std::shared_ptr<OverridePass> overridePass;
		RenderPassType type = RenderPassType_SwapChainPass;
		std::shared_ptr<SwapChainPass> swapChainPass;
		std::shared_ptr<RenderToTexturePass> rendererToTexturePass;

		RenderPassInstance() {}
		~RenderPassInstance();
		void Pass(std::function<void()> renderCallback = [] {}, bool clearRTV = true, XMVECTORF32 clearColor = DirectX::Colors::Black) const;
		std::shared_ptr<MaterialInstance> GetRenderPassMaterialInstance(
			std::string materialUUID,
			std::shared_ptr<MeshInstance> mesh,
			bool shadowed,
			std::string objectUUID = "",
			JObjectChangeCallback cb = [](std::shared_ptr<JObject>) {},
			JObjectChangePostCallback postCb = [](unsigned int, unsigned int) {}
		) const;
		void ResizeRelease() const;
		void Resize(unsigned int width, unsigned int height) const;
	};
};

