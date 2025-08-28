#include "pch.h"
#include <Scene.h>
#include <Lights/Lights.h>
#include <Lights/ShadowMap.h>
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>
#include <Sound/SoundFX.h>
#include <Renderer.h>
#include <Effects.h>
#include <StepTimer.h>
#include <JExposeTypes.h>

extern std::shared_ptr<Renderer> renderer;
void AnimableStep(double elapsedSeconds);
void AudioStep(float step);

namespace Scene
{
	void SceneObjectsStep(DX::StepTimer& timer)
	{
		using namespace Effects;
		EffectsStep(static_cast<FLOAT>(timer.GetElapsedSeconds()));
		AnimableStep(timer.GetElapsedSeconds());
		RenderablesStep();
		LightsStep();
		AudioStep(static_cast<FLOAT>(timer.GetElapsedSeconds()));
		CamerasStep();
	}

	void CreateRenderablesCameraBinding()
	{
		std::vector<std::shared_ptr<Renderable>> renderablesToBind = GetRenderables();
		std::for_each(renderablesToBind.begin(), renderablesToBind.end(), [](std::shared_ptr<Renderable> r)
			{
				auto cams = r->cameras();
				std::for_each(cams.begin(), cams.end(), [r](std::string uuid)
					{
						if (r->bindedCameras.contains(uuid)) return;

						auto cam = FindInCameras(uuid);
						if (!cam) return;

						cam->BindRenderable(r);
					}
				);
			}
		);
	}

	void WriteConstantsBuffers()
	{
		using namespace Effects;
		unsigned int backBufferIndex = renderer->backBufferIndex;

		for (auto& r : GetRenderables())
		{
			r->WriteAnimationConstantsBuffer(backBufferIndex);
			r->WriteConstantsBuffer(backBufferIndex);
		}

		WriteEffectsToConstantsBuffer(backBufferIndex);

		unsigned int lightIndex = 0U;
		ResetConstantsBufferLightAttributes(backBufferIndex);
		ResetConstantsBufferShadowMapAttributes(backBufferIndex);
		for (auto& l : GetLights())
		{
			WriteConstantsBufferLightAttributes(l, backBufferIndex, lightIndex++, l->shadowMapIndex);
			if (l->hasShadowMaps() && l->lightType() != LT_Ambient)
			{
				WriteConstantsBufferShadowMapAttributes(l, backBufferIndex, l->shadowMapIndex);
				WriteShadowMapCamerasConstantsBuffers(l, backBufferIndex);
			}
		}
		WriteConstantsBufferNumLights(backBufferIndex, lightIndex);
	}

	void RenderSceneShadowMaps()
	{
#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandQueue.p, 0, L"ShadowMaps");
#endif
		for (auto& l : GetLights())
		{
			if (!l->hasShadowMaps()) continue;

			auto renderSceneShadowMap = [&l](unsigned int cameraIndex)
				{
					auto& camera = l->shadowMapCameras.at(cameraIndex);
					auto& rp = camera->cameraRenderPasses.at(0);
					for (auto& r : GetRenderables())
					{
						if (r->castShadows())
						{
							r->Render(rp, camera);
						}
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

	void RenderSceneCameras()
	{
		auto cameras = GetCameras();
		for (auto& cam : cameras)
		{
			if (!cam->lightCam)
				cam->Render();
		}
	}

#if defined(_EDITOR)
	bool AnySceneObjectPopupOpen()
	{
		for (auto& [type, _] : SceneObjectTypeToString)
		{
			if (SceneObjectPopupIsOpen(type)) return true;
		}

		return false;
	}

	const std::map<SceneObjectType, std::function<std::vector<UUIDName>()>> GetSO =
	{
		{ SO_Renderables, GetRenderablesUUIDNames },
		{ SO_Lights, GetLightsUUIDNames },
		{ SO_Cameras, GetCamerasUUIDNames },
		{ SO_SoundEffects, GetSoundEffectsUUIDNames }
	};

	std::shared_ptr<SceneObject> GetSceneObject(std::string uuid)
	{
		const std::map<
			SceneObjectType,
			std::function<std::shared_ptr<SceneObject>(std::string)>
		> GetSharedPtrSO =
		{
			{ SO_Renderables, [](std::string uuid) { std::shared_ptr<SceneObject> o = FindInRenderables(uuid); return o; } },
			{ SO_Lights, [](std::string uuid) { std::shared_ptr<SceneObject> o = FindInLights(uuid); return o; } },
			{ SO_Cameras, [](std::string uuid) { std::shared_ptr<SceneObject> o = FindInCameras(uuid); return o; } },
			{ SO_SoundEffects, [](std::string uuid) { std::shared_ptr<SceneObject> o = FindInSoundEffects(uuid); return o; } }
		};

		return GetSharedPtrSO.at(GetSceneObjectType(uuid))(uuid); //kill me please this is slow as hell
	}

	std::map<SceneObjectType, std::vector<UUIDName>> GetSceneObjects()
	{
		std::map<SceneObjectType, std::vector<UUIDName>> sceneObjects;
		for (auto& [type, cb] : GetSO)
		{
			auto append = cb();
			nostd::AppendToVector(sceneObjects[type], append);
		}
		return sceneObjects;
	}

	std::vector<UUIDName> GetSceneObjects(SceneObjectType so)
	{
		return GetSO.at(so)();
	}

	void DrawSceneObjectsPopups(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<void()>> DrawSOPopups =
		{
			{ SO_Renderables, [] {} },
			{ SO_Lights, [] {}},
			{ SO_Cameras, [] {} },
			{ SO_SoundEffects, [] {}}
		};
		DrawSOPopups.at(so)();
	}

	SceneObjectType GetSceneObjectType(std::string uuid)
	{
		for (auto& [type, cb] : GetSO)
		{
			auto objects = cb();
			for (auto& uuidname : objects)
			{
				std::string uuidSO = std::get<0>(uuidname);
				if (uuid == uuidSO) return type;
			}
		}
		return SO_None;
	}

	std::string GetSceneObjectName(SceneObjectType so, std::string uuid)
	{
		const std::map<SceneObjectType, std::function<std::string(std::string)>> GetSOName =
		{
			{ SO_Renderables, FindNameInRenderables },
			{ SO_Lights, FindNameInLights },
			{ SO_Cameras, FindNameInCameras },
			{ SO_SoundEffects, FindNameInSoundEffects }
		};
		return GetSOName.at(so)(uuid);
	}

	std::string GetSceneObjectUUID(std::string name)
	{
		for (auto& [type, cb] : GetSO)
		{
			auto objects = cb();
			for (auto uuidname : objects)
			{
				std::string uuid = std::get<0>(uuidname);
				std::string nameSO = std::get<1>(uuidname);
				if (name == nameSO) return uuid;
			}
		}
		return "";
	}

	std::vector<std::pair<std::string, JsonToEditorValueType>> GetSceneObjectAttributes(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<std::vector<std::pair<std::string, JsonToEditorValueType>>()>> GetSOAtts =
		{
			{ SO_Renderables, GetRenderableAttributes },
			{ SO_Lights, GetLightAttributes },
			{ SO_Cameras, GetCameraAttributes },
			{ SO_SoundEffects, GetSoundFXAttributes }
		};
		return GetSOAtts.at(so)();
	}

	std::map<std::string, JEdvDrawerFunction> GetSceneObjectDrawers(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<std::map<std::string, JEdvDrawerFunction>()>> GetSODrawers =
		{
			{ SO_Renderables, GetRenderableDrawers },
			{ SO_Lights, GetLightDrawers },
			{ SO_Cameras, GetCameraDrawers },
			{ SO_SoundEffects, GetSoundFXDrawers }
		};
		return GetSODrawers.at(so)();
	}

	bool SceneObjectPopupIsOpen(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<bool()>> SOPopupIsOpen = {
			{ SO_Renderables, [] {return false; }},
			{ SO_Lights, [] {return false; }},
			{ SO_Cameras, [] {return false; }},
			{ SO_SoundEffects, [] {return false; }}
		};
		return SOPopupIsOpen.at(so)();
	}

	void CreateSceneObject(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<void()>> CreateSO =
		{
			{ SO_Renderables, [] {} },
			{ SO_Lights, [] {}},
			{ SO_Cameras, [] {} },
			{ SO_SoundEffects, [] {}}
		};
		CreateSO.at(so)();
	}

	void DeleteSceneObject(SceneObjectType so, std::string uuid)
	{
		const std::map<SceneObjectType, std::function<void(std::string)>> DeleteSO =
		{
			{ SO_Renderables, [](std::string uuid) {} },
			{ SO_Lights, [](std::string uuid) {}},
			{ SO_Cameras, [](std::string uuid) {} },
			{ SO_SoundEffects, [](std::string uuid) {}}
		};
		DeleteSO.at(so)(uuid);
	}

	void DeleteSceneObject(std::string uuid)
	{
		SceneObjectType type = GetSceneObjectType(uuid);
		DeleteSceneObject(type, uuid);
	}

#endif
}