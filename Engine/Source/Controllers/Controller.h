#pragma once
#include <memory>
#include <string>
#include <vector>

namespace Scene
{
	struct SceneObject;
};

namespace Game
{
	struct Controller
	{
		std::shared_ptr<Scene::SceneObject> sceneObject;
		virtual void Step(float delta) {};
		virtual void Map(std::shared_ptr<Scene::SceneObject> so) { sceneObject = so; }
		virtual void Unmap() { sceneObject = nullptr; }
	};

	void RegisterController(std::shared_ptr<Controller> controller, std::shared_ptr<Scene::SceneObject> sceneObject);
	void UnregisterController(std::shared_ptr<Controller> controller);
	void DestroyControllers();
	void StepControllers(float delta);

	extern std::vector<std::string> GetGameControllers();
	extern std::shared_ptr<Game::Controller> GetGameController(std::string name);
};
