#pragma once

#include "Level.h"
using namespace Scene::Level;

#include "Camera/Camera.h"
#include "Lights/Lights.h"
#include "Renderable/Renderable.h"
#include "Sound/SoundEffect.h"

using namespace Scene;

enum _SceneObjects {
	SO_Renderables,
	SO_Lights,
	SO_Cameras,
	SO_SoundEffects
};

static const std::unordered_map<_SceneObjects, std::string> SceneObjectsToStr = {
	{ SO_Renderables, "Renderables" },
	{ SO_Lights,	"Lights" },
	{ SO_Cameras, "Cameras" },
	{ SO_SoundEffects, "SoundEffects" }
};

#if defined(_EDITOR)
static const std::map<_SceneObjects, std::function<std::vector<std::string>()>> GetSceneObjects = {
	{ SO_Renderables, GetRenderablesNames },
	{ SO_Lights, GetLightsNames },
	{ SO_Cameras, GetCamerasNames },
	{ SO_SoundEffects, GetSoundEffectsNames }
};

static const std::map<_SceneObjects, std::function<void(std::string, void*&)>> SetSelectedSceneObject = {
	{ SO_Renderables, SelectRenderable },
	{ SO_Lights, SelectLight },
	{ SO_Cameras, SelectCamera },
	{ SO_SoundEffects, SelectSoundEffect }
};

static const std::map<_SceneObjects, std::function<void(void*&)>> DeSelectSceneObject = {
	{ SO_Renderables, DeSelectRenderable },
	{ SO_Lights, DeSelectLight },
	{ SO_Cameras, DeSelectCamera },
	{ SO_SoundEffects, DeSelectSoundEffect }
};

static const std::map<_SceneObjects, std::function<void(void*&, ImVec2, ImVec2, bool)>> DrawSceneObjectPanel = {
	{ SO_Renderables, DrawRenderablePanel },
	{ SO_Lights, DrawLightPanel },
	{ SO_Cameras, DrawCameraPanel },
	{ SO_SoundEffects, DrawSoundEffectPanel }
};

static const std::map<_SceneObjects, std::function<std::string(void*)>> GetSceneObjectName = {
	{ SO_Renderables, GetRenderableName },
	{ SO_Lights, GetLightName },
	{ SO_Cameras, GetCameraName },
	{ SO_SoundEffects, GetSoundEffectName }
};

#endif

namespace Scene
{
	void WriteConstantsBuffers();
	void RenderSceneShadowMaps();
	void RenderSceneObjects(std::shared_ptr<Camera>& camera);
}