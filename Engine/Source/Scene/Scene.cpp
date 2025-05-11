#include "pch.h"
#include "Scene.h"
#include "Lights/Lights.h"
#include "Lights/ShadowMap.h"
#include "../Renderer/Renderer.h"
#include "../Effects/Effects.h"
#include "../Common/StepTimer.h"

extern std::shared_ptr<Renderer> renderer;
void AnimableStep(double elapsedSeconds);
void AudioStep();

namespace Scene
{
	void SceneObjectsStep(DX::StepTimer& timer)
	{
		using namespace Effects;
		EffectsStep(static_cast<FLOAT>(timer.GetElapsedSeconds()));
		AnimableStep(timer.GetElapsedSeconds());
		RenderablesStep();
		LightsStep();
		AudioStep();
		CamerasStep();
	}

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
#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandQueue.p, 0, L"ShadowMaps");
#endif
		for (auto& l : GetLights())
		{
			if (!l->hasShadowMaps()) continue;

			auto renderSceneShadowMap = [&l](size_t passHash, unsigned int cameraIndex)
				{
					l->shadowMapCameras[cameraIndex]->WriteConstantsBuffer(renderer->backBufferIndex);
					for (auto& [name, r] : GetRenderables())
					{
						if (!r->castShadows()) continue;
						r->RenderShadowMap(passHash, l, cameraIndex);
					}
				};

#if defined(_DEVELOPMENT)
			std::string shadowMapEvent = "ShadowMap:" + l->name();
			PIXBeginEvent(renderer->commandList.p, 0, nostd::StringToWString(shadowMapEvent).c_str());
#endif

			l->RenderShadowMap(renderSceneShadowMap);

#if defined(_DEVELOPMENT)
			PIXEndEvent(renderer->commandList.p);
#endif
		}
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandQueue.p);
#endif
	}

	void RenderSceneObjects(size_t passHash, std::shared_ptr<Camera>& camera)
	{
#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, L"Render Scene");
#endif
		{
			unsigned int backBufferIndex = renderer->backBufferIndex;

			camera->WriteConstantsBuffer(backBufferIndex);

			unsigned int lightIndex = 0U;
			for (auto l : GetLights())
			{
				UpdateConstantsBufferLightAttributes(l, backBufferIndex, lightIndex++, l->shadowMapIndex);
				if (l->hasShadowMaps() && l->lightType() != LT_Ambient)
				{
					UpdateConstantsBufferShadowMapAttributes(l, renderer->backBufferIndex, l->shadowMapIndex);
				}
			}
			UpdateConstantsBufferNumLights(backBufferIndex, lightIndex);

			for (auto& [name, r] : GetRenderables())
			{
				r->Render(passHash, camera);
			}

		}
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif
	}

}