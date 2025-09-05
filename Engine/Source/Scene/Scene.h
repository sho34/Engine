#pragma once

#include "Level.h"
#include <Application.h>
#include <map>
#include <string>
#include <vector>
#include <UUID.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <SceneObject.h>
#include <JExposeTypes.h>
#include <Binder.h>

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
	std::shared_ptr<SceneObject> GetSceneObject(std::string uuid);
	std::map<SceneObjectType, std::vector<UUIDName>> GetSceneObjects();
	std::vector<UUIDName> GetSceneObjects(SceneObjectType so);
	SceneObjectType GetSceneObjectType(std::string uuid);
	std::string GetSceneObjectName(SceneObjectType so, std::string uuid);
	std::string GetSceneObjectUUID(std::string name);
	std::vector<std::pair<std::string, JsonToEditorValueType>> GetSceneObjectAttributes(SceneObjectType so);
	std::map<std::string, JEdvDrawerFunction> GetSceneObjectDrawers(SceneObjectType so);
	std::vector<std::string> GetSceneObjectRequiredAttributes(SceneObjectType so);
	nlohmann::json GetSceneObjectJson(SceneObjectType so);
	std::map<std::string, JEdvCreatorDrawerFunction> GetSceneObjectCreatorDrawers(SceneObjectType so);

	void CreateSceneObject(SceneObjectType so, nlohmann::json json);
	void DeleteSceneObject(SceneObjectType so, std::string uuid);
	void DeleteSceneObject(std::string uuid);
#endif
}