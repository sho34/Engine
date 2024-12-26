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

static const std::unordered_map<_SceneObjects, std::wstring> SceneObjectsToStr = {
	{ Renderables, L"Renderables" },
	{ Lights,	L"Lights" },
	{ Cameras, L"Cameras" },
	{ SoundEffects, L"SoundEffects" }
};

static const std::map<_SceneObjects,std::function<std::vector<std::wstring>()>> GetSceneObjects = {
	{ Renderables, GetRenderablesNames },
	{ Lights, GetLightsNames },
	{ Cameras, GetCamerasNames },
	{ SoundEffects, GetSoundEffectsNames }
};