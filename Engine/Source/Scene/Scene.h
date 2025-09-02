#pragma once

#include "Level.h"
#include <Application.h>
#include <map>
#include <string>
#include <vector>
#include <UUID.h>
#if defined(_EDITOR)
#include <IconsFontAwesome5.h>
#endif
#include <memory>
#include <nlohmann/json.hpp>
#include <SceneObject.h>
#include <JExposeTypes.h>

enum SceneObjectType {
	SO_None,
	SO_Renderables,
	SO_Lights,
	SO_Cameras,
	SO_SoundEffects
};

inline const std::map<SceneObjectType, std::string> SceneObjectTypeToString = {
	{ SO_Renderables, "Renderables" },
	{ SO_Lights,	"Lights" },
	{ SO_Cameras, "Cameras" },
	{ SO_SoundEffects, "SoundEffects" }
};

inline const std::map<std::string, SceneObjectType> StringToSceneObjectType = {
	{ "Renderables", SO_Renderables },
	{ "Lights", SO_Lights },
	{ "Cameras", SO_Cameras },
	{ "SoundEffects", SO_SoundEffects }
};

#if defined(_EDITOR)
inline const std::map<SceneObjectType, const char* > SceneObjectsTypePanelMenuItems = {
	{ SO_Renderables, ICON_FA_SNOWMAN "Renderables" },
	{ SO_Lights, ICON_FA_LIGHTBULB "Lights" },
	{ SO_Cameras, ICON_FA_CAMERA "Cameras" },
	{ SO_SoundEffects, ICON_FA_MUSIC "SoundEffects" }
};
#endif

using namespace Scene::Level;

inline bool NameCollideWithSceneObjects(auto map, nlohmann::json& json)
{
	std::string name = json.at("name");
	return map.end() != std::find_if(map.begin(), map.end(), [name](auto pair)
		{
			return pair.second->name() == name;
		}
	);
}

namespace DX { class StepTimer; }

namespace Scene
{
	struct Camera;

	void SceneObjectsStep(DX::StepTimer& timer);
	void CreateRenderablesCameraBinding();
	void WriteConstantsBuffers();
	void RenderSceneShadowMaps();
	void RenderSceneCameras();

#if defined(_EDITOR)
	bool AnySceneObjectPopupOpen();
	void DrawSceneObjectsPopups(SceneObjectType so);
	bool SceneObjectPopupIsOpen(SceneObjectType so);

	std::shared_ptr<SceneObject> GetSceneObject(std::string uuid);
	std::map<SceneObjectType, std::vector<UUIDName>> GetSceneObjects();
	std::vector<UUIDName> GetSceneObjects(SceneObjectType so);
	SceneObjectType GetSceneObjectType(std::string uuid);
	std::string GetSceneObjectName(SceneObjectType so, std::string uuid);
	std::string GetSceneObjectUUID(std::string name);
	std::vector<std::pair<std::string, JsonToEditorValueType>> GetSceneObjectAttributes(SceneObjectType so);
	std::map<std::string, JEdvDrawerFunction> GetSceneObjectDrawers(SceneObjectType so);
	std::vector<std::pair<std::string, bool>> GetSceneObjectRequiredAttributes(SceneObjectType so);
	nlohmann::json GetSceneObjectJson(SceneObjectType so);

	void CreateSceneObject(SceneObjectType so);
	void DeleteSceneObject(SceneObjectType so, std::string uuid);
	void DeleteSceneObject(std::string uuid);
#endif
}