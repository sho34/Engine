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

namespace DX { class StepTimer; }

namespace Scene
{
	struct Camera;

	template<typename S>
	std::shared_ptr<S> CreateSceneObjectFromJson(nlohmann::json& j)
	{
		std::shared_ptr<S> r = std::make_shared<S>(j);
		r->this_ptr = r;
		r->Initialize();
		AddSceneObject(r);
		return r;
	}

	void BindSceneObjects();

	void AddSceneObject(std::shared_ptr<SceneObject> sceneObject);
	template<typename S>
	void SafeDeleteSceneObject(std::shared_ptr<S>& sceneObject)
	{
		if (sceneObject == nullptr) return;
		DEBUG_PTR_COUNT_JSON(sceneObject);

		DeleteSceneObject(sceneObject);
		sceneObject->UnbindFromScene();
		sceneObject->this_ptr = nullptr;
		sceneObject = nullptr;
	}
	void DeleteSceneObject(std::shared_ptr<SceneObject> sceneObject);

	void BindToScene(std::shared_ptr<SceneObject> soA, std::shared_ptr<SceneObject> soB);
	void UnbindFromScene(std::shared_ptr<SceneObject> soA);
	void UnbindFromScene(std::shared_ptr<SceneObject> soA, std::shared_ptr<SceneObject> soB);
	void SceneObjectsStep(DX::StepTimer& timer);
	void WriteConstantsBuffers();
	void RenderSceneShadowMaps();
	void RenderSceneCameras();

#if defined(_EDITOR)
	std::shared_ptr<SceneObject> GetSceneObject(std::string uuid);
	std::map<SceneObjectType, std::vector<UUIDName>> GetSceneObjects();
	std::vector<UUIDName> GetSceneObjects(SceneObjectType so);
	SceneObjectType GetSceneObjectType(std::string uuid);
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