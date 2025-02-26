#include "pch.h"
#include "Scene.h"
#include "Lights/Lights.h"
#include "Lights/ShadowMap.h"
#include "../Renderer/Renderer.h"
#include "../Effects/Effects.h"

extern std::shared_ptr<Renderer> renderer;
namespace Scene
{
	void WriteConstantsBuffers()
	{
		using namespace Effects;
		unsigned int backBufferIndex = renderer->backBufferIndex;

		for (auto& [name, r] : GetRenderables())
		{
			r->WirteAnimationConstantsBuffer(backBufferIndex);
			r->WriteConstantsBuffer(backBufferIndex);
			r->WriteShadowMapConstantsBuffer(backBufferIndex);
		}

		WriteEffectsToConstantsBuffer(backBufferIndex);
	}

	void RenderSceneShadowMaps()
	{
		PIXBeginEvent(renderer->commandQueue.p, 0, L"ShadowMaps");
		for (auto& l : GetLights())
		{
			if (!l->hasShadowMaps) continue;

			auto renderSceneShadowMap = [&l](unsigned int cameraIndex)
				{
					l->shadowMapCameras[cameraIndex]->WriteConstantsBuffer(renderer->backBufferIndex);
					for (auto& [name, r] : GetRenderables())
					{
						r->RenderShadowMap(l, cameraIndex);
					}
				};

			std::string shadowMapEvent = "ShadowMap:" + l->name;
			PIXBeginEvent(renderer->commandList.p, 0, nostd::StringToWString(shadowMapEvent).c_str());

			l->RenderShadowMap(renderSceneShadowMap);

			PIXEndEvent(renderer->commandList.p);
		}
		PIXEndEvent(renderer->commandQueue.p);
	}

	void RenderSceneObjects(std::shared_ptr<Camera>& camera)
	{
		PIXBeginEvent(renderer->commandList.p, 0, L"Render Scene");
		{
			unsigned int backBufferIndex = renderer->backBufferIndex;

			camera->WriteConstantsBuffer(backBufferIndex);

			unsigned int lightIndex = 0U;
			//unsigned int shadowMapIndex = 0U;
			for (auto l : GetLights())
			{
				UpdateConstantsBufferLightAttributes(l, backBufferIndex, lightIndex++, l->shadowMapIndex);
				if (l->hasShadowMaps && l->lightType != LT_Ambient)
				{
					UpdateConstantsBufferShadowMapAttributes(l, renderer->backBufferIndex, l->shadowMapIndex);
				}
			}
			UpdateConstantsBufferNumLights(backBufferIndex, lightIndex);

			for (auto& [name, r] : GetRenderables())
			{
				r->Render(camera);
			}

		}
		PIXEndEvent(renderer->commandList.p);
	}

}