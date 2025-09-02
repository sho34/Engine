#include "pch.h"
#include "RenderPass.h"
#include <DXTypes.h>
#include <Templates.h>
#include <TemplateDef.h>
#include <Renderer.h>
#include <RenderPass/SwapChainPass.h>
#include <RenderPass/RenderToTexturePass.h>
#include <Material/Material.h>
#include <Mesh/Mesh.h>
#include <Camera/Camera.h>
#include "Override/ResolvePass.h"
#include "Override/ToneMappingPass.h"
#include "Override/MinMaxChainPass.h"
#include "Override/MinMaxChainResultPass.h"

extern std::shared_ptr<Renderer> renderer;
using namespace Scene;

namespace Templates
{
#include <JExposeAttDrawersDef.h>
#include <RenderPassAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttJsonDef.h>
#include <RenderPassAtt.h>
#include <JExposeEnd.h>

	namespace RenderPass
	{
		std::shared_ptr<DeviceUtils::DescriptorHeap> mainHeap;

		void CreateMainHeap()
		{
			mainHeap = std::make_shared<DeviceUtils::DescriptorHeap>();
			mainHeap->CreateDescriptorHeap(renderer->d3dDevice, renderer->numFrames);
		}

		void DestroyMainHeap()
		{
			mainHeap->DestroyDescriptorHeap();
			mainHeap = nullptr;
		}

		void ResizeRelease()
		{
			auto cameras = GetWindowCameras();
			for (auto& cam : cameras)
			{
				cam->ResizeReleasePasses();
			}
		}

		void Resize(unsigned int width, unsigned int height)
		{
			auto cameras = GetWindowCameras();
			for (auto& cam : cameras)
			{
				cam->ResizePasses(width, height);
			}
		}
	}

	RenderPassJson::RenderPassJson(nlohmann::json json) : JTemplate(json)
	{
#include <JExposeInit.h>
#include <RenderPassAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttUpdate.h>
#include <RenderPassAtt.h>
#include <JExposeEnd.h>
	}

	TEMPDEF_FULL(RenderPass);

	std::shared_ptr<RenderPassInstance> GetRenderPassInstance(std::shared_ptr<Camera> cam, unsigned int renderPassIndex, std::string uuid, unsigned int width, unsigned int height)
	{
		const std::map<RenderPassRenderCallbackOverride, std::function<
			std::shared_ptr<OverridePass>(
				std::shared_ptr<Camera>,
				unsigned int,
				std::shared_ptr<Templates::RenderPassInstance>
			)>> RenderCallbackOverriders =
		{
			{ RenderPassRenderCallbackOverride_None, [](auto c,auto rpi, auto rp) { return nullptr; }},
			{ RenderPassRenderCallbackOverride_ToneMapping, [](auto c, auto rpi, auto rp) { return std::make_shared<ToneMappingPass>(c,rpi, rp); } },
			{ RenderPassRenderCallbackOverride_Resolve, [](auto c, auto rpi, auto rp) { return rpi > 0 ? std::make_shared<ResolvePass>(c,rpi, rp) : nullptr; } },
			{ RenderPassRenderCallbackOverride_MinMaxChain, [](auto c, auto rpi, auto rp) { return std::make_shared<MinMaxChainPass>(c,rpi, rp); } },
			{ RenderPassRenderCallbackOverride_MinMaxChainResult, [](auto c, auto rpi, auto rp) { return std::make_shared<MinMaxChainResultPass>(c,rpi, rp); } }
		};

		using namespace RenderPass;

		std::shared_ptr<RenderPassInstance> instance = std::make_shared<RenderPassInstance>();
		std::shared_ptr<RenderPassJson> rp = GetRenderPassTemplate(uuid);

		instance->camera = cam;
		instance->renderPassUUID = uuid;

		switch (rp->type())
		{
		case RenderPassType_SwapChainPass:
		{
			instance->type = RenderPassType_SwapChainPass;
			instance->swapChainPass = DeviceUtils::CreateRenderPass(GetRenderPassName(uuid), mainHeap, rp->depthStencilFormat());
		}
		break;
		case RenderPassType_RenderToTexturePass:
		{
			assert(width != 0U); assert(height != 0U);
			instance->type = RenderPassType_RenderToTexturePass;
			instance->rendererToTexturePass = DeviceUtils::CreateRenderPass(
				GetRenderPassName(uuid),
				rp->renderTargetFormats(),
				rp->depthStencilFormat(),
				width,
				height
			);
		}
		break;
		}

		instance->materialOverride = rp->materialOverride();
		instance->renderCallbackOverride = rp->renderCallbackOverride();
		instance->overridePass = RenderCallbackOverriders.at(instance->renderCallbackOverride)(cam, renderPassIndex, instance);

		return instance;
	}

	void DestroyRenderPassInstance(std::shared_ptr<RenderPassInstance>& rp)
	{
		if (!rp) return;

		rp->camera = nullptr;
		rp->overridePass = nullptr;
		switch (rp->type)
		{
		case RenderPassType_SwapChainPass:
		{
			rp->swapChainPass->ReleaseResources();
			rp->swapChainPass = nullptr;
		}
		break;
		case RenderPassType_RenderToTexturePass:
		{
			rp->rendererToTexturePass->ReleaseResources();
			rp->rendererToTexturePass = nullptr;
		}
		break;
		}
	}

	RenderPassInstance::~RenderPassInstance()
	{
		if (swapChainPass) {
			swapChainPass->ReleaseResources();
			swapChainPass = nullptr;
		}
		if (rendererToTexturePass) {
			rendererToTexturePass->ReleaseResources();
			rendererToTexturePass = nullptr;
		}
	}

	void RenderPassInstance::Pass(std::function<void()> renderCallback, bool clearRTV, XMVECTORF32 clearColor) const
	{
		if (overridePass) return overridePass->Pass();

		switch (type)
		{
		case RenderPassType_RenderToTexturePass:
		{
			rendererToTexturePass->Pass(renderCallback, clearColor);
		}
		break;
		case RenderPassType_SwapChainPass:
		{
			swapChainPass->Pass(renderCallback, clearRTV, clearColor);
		}
		break;
		}
	}

	std::shared_ptr<MaterialInstance> RenderPassInstance::GetRenderPassMaterialInstance(
		std::string materialUUID,
		std::shared_ptr<MeshInstance> mesh,
		bool shadowed,
		std::string bindingUUID,
		JObjectChangeCallback materialChangeCallback,
		JObjectChangePostCallback materialChangePostCallback
	) const
	{
		VertexClass vertexClass = mesh->vertexClass;
		std::string vertexType = VertexClassToString.at(vertexClass);

		auto noOverride = [materialUUID, vertexClass, vertexType, shadowed, bindingUUID, materialChangeCallback, materialChangePostCallback]()
			{
				std::string instanceUUID = materialUUID + "-" + vertexType;
				return GetMaterialInstance(instanceUUID, [instanceUUID, materialUUID, vertexClass, shadowed, bindingUUID, materialChangeCallback, materialChangePostCallback]()
					{
						return std::make_shared<MaterialInstance>(instanceUUID, materialUUID, vertexClass, shadowed, TextureShaderUsageMap(),
							bindingUUID, materialChangeCallback, materialChangePostCallback);
					}
				);
			};

		auto shadowMapOverride = [materialUUID, vertexClass, vertexType]()
			{
				std::string smMatUUID = FindMaterialUUIDByName("ShadowMap");
				std::shared_ptr<MaterialJson> material = GetMaterialTemplate(materialUUID);
				std::string instanceUUID = smMatUUID + "-" + vertexType;
				TextureShaderUsageMap overrideTextures;
				if (material->textures_contains(TextureShaderUsage_Base))
				{
					overrideTextures.insert_or_assign(TextureShaderUsage_Base, material->textures().at(TextureShaderUsage_Base));
				}
				return GetMaterialInstance(instanceUUID, [instanceUUID, smMatUUID, vertexClass, overrideTextures]()
					{
						return std::make_shared<MaterialInstance>(instanceUUID, smMatUUID, vertexClass, false, overrideTextures);
					}
				);
			};

		auto pickingOverride = [vertexClass, vertexType]
			{
				std::string pickMaterialUUID = FindMaterialUUIDByName("Picking");
				std::string instanceUUID = pickMaterialUUID + "-" + vertexType;
				return GetMaterialInstance(instanceUUID, [instanceUUID, pickMaterialUUID, vertexClass]()
					{
						return std::make_shared<MaterialInstance>(instanceUUID, pickMaterialUUID, vertexClass, false);
					}
				);
			};

		const std::map<RenderPassMaterialOverride, std::function<std::shared_ptr<MaterialInstance>()>> overrideMaterial =
		{
			{ RenderPassMaterialOverride_None, noOverride },
			{ RenderPassMaterialOverride_ShadowMap, shadowMapOverride },
			{ RenderPassMaterialOverride_Picking, pickingOverride },
		};

		return overrideMaterial.at(materialOverride)();
	}

	void RenderPassInstance::ResizeRelease() const
	{
		switch (type)
		{
		case RenderPassType_SwapChainPass:
			swapChainPass->ReleaseResources();
			break;
		case RenderPassType_RenderToTexturePass:
			rendererToTexturePass->ReleaseResources();
			break;
		}
	}

	void RenderPassInstance::Resize(unsigned int width, unsigned int height) const
	{
		switch (type)
		{
		case RenderPassType_SwapChainPass:
			swapChainPass->Resize(width, height);
			break;
		case RenderPassType_RenderToTexturePass:
			rendererToTexturePass->Resize(width, height);
			break;
		}
	}
};