#pragma once

#include "Level.h"
using namespace Scene::Level;

#include "Camera/CameraImpl.h"
#include "Lights/LightsImpl.h"
#include "Renderable/RenderableImpl.h"
#include "Sound/SoundEffectImpl.h"

enum _SceneObjects {
	Renderables,
	Lights,
	Cameras,
	SoundEffects
};

static const std::unordered_map<_SceneObjects, std::string> SceneObjectsToStr = {
	{ Renderables, "Renderables" },
	{ Lights,	"Lights" },
	{ Cameras, "Cameras" },
	{ SoundEffects, "SoundEffects" }
};

static const std::map<_SceneObjects,std::function<std::vector<std::string>()>> GetSceneObjects = {
	{ Renderables, GetRenderablesNames },
	{ Lights, GetLightsNames },
	{ Cameras, GetCamerasNames },
	{ SoundEffects, GetSoundEffectsNames }
};

static const std::map<_SceneObjects, std::function<void(std::string, void*&)>> SetSelectedSceneObject = {
	{ Renderables, SelectRenderable },
	{ Lights, SelectLight },
	{ Cameras, SelectCamera },
	{ SoundEffects, SelectSoundEffect }
};

static const std::map<_SceneObjects, std::function<void(void*&, ImVec2, ImVec2)>> DrawSceneObjectPanel = {
	{ Renderables, DrawRenderablePanel },
	{ Lights, DrawLightPanel },
	{ Cameras, DrawCameraPanel },
	{ SoundEffects, DrawSoundEffectPanel }
};

static const std::map<_SceneObjects, std::function<std::string(void*)>> GetSceneObjectName = {
	{ Renderables, GetRenderableName },
	{ Lights, GetLightName },
	{ Cameras, GetCameraName },
	{ SoundEffects, GetSoundEffectName }
};