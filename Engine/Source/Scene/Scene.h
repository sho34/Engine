#pragma once

#include "Level.h"
using namespace Scene::Level;

#include "Camera/Camera.h"
#include "Lights/Lights.h"
#include "Renderable/Renderable.h"
#include "Sound/SoundEffect.h"
#include <Application.h>

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

template<typename T>
std::vector<UUIDName> GetSceneObjectsUUIDsNames(std::map<std::string, std::shared_ptr<T>>& objects)
{
	std::map<std::string, std::shared_ptr<T>> objs;
	std::copy_if(objects.begin(), objects.end(), std::inserter(objs, objs.end()), [](const auto& pair)
		{
			return !pair.second->hidden();
		}
	);

	std::vector<UUIDName> uuidNames;

	std::transform(objs.begin(), objs.end(), std::back_inserter(uuidNames), [](const auto& pair)
		{
			UUIDName uuidName;

			std::string& uuid = std::get<0>(uuidName);
			uuid = pair.first;

			std::string& name = std::get<1>(uuidName);
			name = pair.second->name();

			return uuidName;
		}
	);

	return uuidNames;
}

#if defined(_EDITOR)
static const std::map<_SceneObjects, std::function<std::vector<UUIDName>()>> GetSceneObjects =
{
	{ SO_Renderables, GetRenderablesUUIDNames },
	{ SO_Lights, GetLightsUUIDNames },
	{ SO_Cameras, GetCamerasUUIDNames },
	{ SO_SoundEffects, GetSoundEffectsUUIDNames }
};

static const std::map<_SceneObjects, std::function<void(std::string, std::string&)>> SetSelectedSceneObject =
{
	{ SO_Renderables, SelectRenderable },
	{ SO_Lights, SelectLight },
	{ SO_Cameras, SelectCamera },
	{ SO_SoundEffects, SelectSoundEffect }
};

static const std::map<_SceneObjects, std::function<void(std::string&)>> DeSelectSceneObject =
{
	{ SO_Renderables, DeSelectRenderable },
	{ SO_Lights, DeSelectLight },
	{ SO_Cameras, DeSelectCamera },
	{ SO_SoundEffects, DeSelectSoundEffect }
};

static const std::map<_SceneObjects, std::function<void(std::string, ImVec2, ImVec2, bool)>> DrawSceneObjectPanel =
{
	{ SO_Renderables, DrawRenderablePanel },
	{ SO_Lights, DrawLightPanel },
	{ SO_Cameras, DrawCameraPanel },
	{ SO_SoundEffects, DrawSoundEffectPanel }
};

static const std::map<_SceneObjects, std::function<void()>> DrawSceneObjectsPopups =
{
	{ SO_Renderables, DrawRenderablesPopups },
	{ SO_Lights, DrawLightsPopups },
	{ SO_Cameras, DrawCamerasPopups },
	{ SO_SoundEffects, DrawSoundEffectsPopups }
};

static const std::map<_SceneObjects, std::function<std::string(std::string)>> GetSceneObjectName =
{
	{ SO_Renderables, GetRenderableName },
	{ SO_Lights, GetLightName },
	{ SO_Cameras, GetCameraName },
	{ SO_SoundEffects, GetSoundEffectName }
};

static const std::map<_SceneObjects, std::function<void(std::string)>> DeleteSceneObject =
{
	{ SO_Renderables, DeleteRenderable },
	{ SO_Lights, DeleteLight },
	{ SO_Cameras, DeleteCamera },
	{ SO_SoundEffects, DeleteSoundEffect }
};

#endif

namespace Scene
{
	void WriteConstantsBuffers();
	void RenderSceneShadowMaps();
	void RenderSceneObjects(std::shared_ptr<Camera>& camera);
}