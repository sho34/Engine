#include "pch.h"
#include <Scene.h>
#include <Light/Light.h>
#include <Light/ShadowMap.h>
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>
#include <Sound/SoundFX.h>
#include <Renderer.h>
#include <Effects.h>
#include <StepTimer.h>
#include <JTypes.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

extern std::shared_ptr<Renderer> renderer;
void AnimableStep(double elapsedSeconds);
void AudioStep(float step);

namespace Scene
{
	Binder binder;

	std::set<std::shared_ptr<SceneObject>> sceneObjects;
	std::map<std::shared_ptr<SceneObject>, SceneObjectType> sceneObjectType;
	std::map<std::string, std::shared_ptr<SceneObject>> sceneObjectFromUUID;
	std::multimap<SceneObjectType, std::shared_ptr<SceneObject>> sceneObjectsByType;

	void BindSceneObjects()
	{
		std::for_each(sceneObjects.begin(), sceneObjects.end(), [](std::shared_ptr<SceneObject> so)
			{
				so->BindToScene();
			}
		);
	}

	void AddSceneObject(std::shared_ptr<SceneObject> sceneObject)
	{
		sceneObjects.insert(sceneObject);
		sceneObjectType.insert_or_assign(sceneObject, sceneObject->JType());
		sceneObjectFromUUID.insert_or_assign(sceneObject->at("uuid"), sceneObject);
		sceneObjectsByType.insert({ sceneObject->JType(), sceneObject });
	}

	void DeleteSceneObject(std::shared_ptr<SceneObject> sceneObject)
	{
		sceneObjects.erase(sceneObject);
		sceneObjectType.erase(sceneObject);
		sceneObjectFromUUID.erase(sceneObject->at("uuid"));
		auto range = sceneObjectsByType.equal_range(sceneObject->JType());
		for (auto it = range.first; it != range.second; )
		{
			if (it->second != sceneObject) {
				it++; continue;
			}
			sceneObjectsByType.erase(it);
			break;
		}
	}

	void BindToScene(std::shared_ptr<SceneObject> soA, std::shared_ptr<SceneObject> soB)
	{
		binder.insert(soA, soB);
	}

	void UnbindFromScene(std::shared_ptr<SceneObject> soA)
	{
		binder.erase(soA);
	}

	void UnbindFromScene(std::shared_ptr<SceneObject> soA, std::shared_ptr<SceneObject> soB)
	{
		binder.erase(soA, soB);
	}

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

		auto newIt = std::remove_if(cameras.begin(), cameras.end(), [](std::shared_ptr<Camera> cam) { return !!cam->lightCam; });
		cameras.erase(newIt, cameras.end());

		std::vector<std::shared_ptr<Camera>> nonSwapChainCams;
		std::copy_if(cameras.begin(), cameras.end(), std::back_inserter(nonSwapChainCams), [](std::shared_ptr<Camera> cam) {return !cam->useSwapChain(); });

		for (auto& cam : nonSwapChainCams)
		{
			cam->Render();
		}

		if (GetNumSwapChainCameras() > 0ULL)
		{
			GetSwapChainCameras().at(0)->Render();
		}
	}

#if defined(_EDITOR)

	std::shared_ptr<SceneObject> GetSceneObject(std::string uuid)
	{
		return sceneObjectFromUUID.at(uuid);
	}

	std::map<SceneObjectType, std::vector<UUIDName>> GetSceneObjects()
	{
		auto SOTypes = { SO_Renderables, SO_Lights, SO_Cameras, SO_SoundEffects };

		std::map<SceneObjectType, std::vector<UUIDName>> sceneObjects;
		for (auto type : SOTypes)
		{
			auto append = GetSceneObjects(type);
			nostd::AppendToVector(sceneObjects[type], append);
		}
		return sceneObjects;
	}

	std::vector<UUIDName> GetSceneObjects(SceneObjectType so)
	{
		std::vector<UUIDName> uuidNames;
		auto range = sceneObjectsByType.equal_range(so);
		for (auto it = range.first; it != range.second; it++) {
			if (it->second->at("hidden") == true) continue;
			std::string uuid = it->second->at("uuid");
			std::string name = it->second->at("name");
			uuidNames.push_back(std::make_tuple(uuid, name));
		}
		return uuidNames;
	}

	SceneObjectType GetSceneObjectType(std::string uuid)
	{
		std::shared_ptr<SceneObject> so = sceneObjectFromUUID.at(uuid);
		return sceneObjectType.at(so);
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

	std::map<std::string, JEdvEditorDrawerFunction> GetSceneObjectDrawers(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<std::map<std::string, JEdvEditorDrawerFunction>()>> GetSODrawers =
		{
			{ SO_Renderables, GetRenderableDrawers },
			{ SO_Lights, GetLightDrawers },
			{ SO_Cameras, GetCameraDrawers },
			{ SO_SoundEffects, GetSoundFXDrawers }
		};
		return GetSODrawers.at(so)();
	}

	std::vector<std::string> GetSceneObjectRequiredAttributes(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<std::vector<std::string>()>> GetSORequiredAtts =
		{
			{ SO_Renderables, GetRenderableRequiredAttributes },
			{ SO_Lights, GetLightRequiredAttributes },
			{ SO_Cameras, GetCameraRequiredAttributes },
			{ SO_SoundEffects, GetSoundFXRequiredAttributes }
		};
		return GetSORequiredAtts.at(so)();
	}

	nlohmann::json GetSceneObjectJson(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<nlohmann::json()>> GetSOJson =
		{
			{ SO_Renderables, CreateRenderableJson },
			{ SO_Lights, CreateLightJson },
			{ SO_Cameras, CreateCameraJson },
			{ SO_SoundEffects, CreateSoundFXJson }
		};
		return GetSOJson.at(so)();
	}

	std::map<std::string, JEdvCreatorDrawerFunction> GetSceneObjectCreatorDrawers(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<std::map<std::string, JEdvCreatorDrawerFunction>()>> GetSODrawers =
		{
			{ SO_Renderables, GetRenderableCreatorDrawers },
			{ SO_Lights, GetLightCreatorDrawers },
			{ SO_Cameras, GetCameraCreatorDrawers },
			{ SO_SoundEffects, GetSoundFXCreatorDrawers }
		};
		return GetSODrawers.at(so)();
	}

	std::map<std::string, JEdvCreatorValidatorFunction> GetSceneObjectValidators(SceneObjectType so)
	{
		const std::map<SceneObjectType, std::function<std::map<std::string, JEdvCreatorValidatorFunction>()>> GetSOValidators =
		{
			{ SO_Renderables, GetRenderableCreatorValidator },
			{ SO_Lights, GetLightCreatorValidator },
			{ SO_Cameras, GetCameraCreatorValidator },
			{ SO_SoundEffects, GetSoundFXCreatorValidator }
		};
		return GetSOValidators.at(so)();
	}

	template<typename S>
	std::shared_ptr<S> CreateAndBind(nlohmann::json& json)
	{
		nlohmann::json patch = { {"uuid",getUUID()} };
		json.merge_patch(patch);
		std::shared_ptr<S> o = CreateSceneObjectFromJson<S>(json);
		o->BindToScene();
		Editor::MarkScenePanelAssetsAsDirty();
		return o;
	}

	void CreateSceneObject(SceneObjectType so, nlohmann::json json)
	{
		const std::map<SceneObjectType, std::function<void(nlohmann::json json)>> CreateSO =
		{
			{ SO_Renderables, [](nlohmann::json json) {
				std::shared_ptr<Renderable> r = CreateAndBind<Renderable>(json);
				Editor::BindRenderableToPickingPass(r);
				r->BindShadowMapCameras();
			}},
			{ SO_Lights, [](nlohmann::json json) {
				std::shared_ptr<Light> l = CreateAndBind<Light>(json);
				if (l->lightBillboard)
				{
					Editor::BindRenderableToPickingPass(l->lightBillboard);
				}
			}},
			{ SO_Cameras, [](nlohmann::json json) {
				std::shared_ptr<Camera> c = CreateAndBind<Camera>(json);
				if (c->cameraBillboard)
				{
					Editor::BindRenderableToPickingPass(c->cameraBillboard);
				}
			} },
			{ SO_SoundEffects, [](nlohmann::json json) {
				std::shared_ptr<SoundFX> s = CreateAndBind<SoundFX>(json);
				if (s->soundFXBillboard)
				{
					Editor::BindRenderableToPickingPass(s->soundFXBillboard);
				}
			}}
		};
		CreateSO.at(so)(json);
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