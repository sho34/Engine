#include "pch.h"
#include "Controller.h"
#include <map>
#include <NoStd.h>

namespace Game
{
	std::map<std::shared_ptr<Controller>, std::shared_ptr<Scene::SceneObject>> controllerRegistry;

	void RegisterController(std::shared_ptr<Controller> controller, std::shared_ptr<Scene::SceneObject> sceneObject)
	{
		controller->Map(sceneObject);
		controllerRegistry.insert_or_assign(controller, sceneObject);
	}

	void UnregisterController(std::shared_ptr<Controller> controller)
	{
		controller->Unmap();
		controllerRegistry.erase(controller);
	}

	void DestroyControllers()
	{
		std::vector<std::shared_ptr<Controller>> controllers;
		std::transform(controllerRegistry.begin(), controllerRegistry.end(), std::back_inserter(controllers), [](auto& pair) {return pair.first; });
		for (auto& controller : controllers)
		{
			UnregisterController(controller);
		}
	}

	void StepControllers(float delta)
	{
		for (auto& [c, _] : controllerRegistry)
		{
			c->Step(delta);
		}
	}
}
