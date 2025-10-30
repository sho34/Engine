#pragma once
#include <memory>
#include <string>
#include <vector>
#include <UUID.h>

namespace Game
{
	struct Controller
	{
		JUUID sceneObject;
		virtual void Step(float delta) {};
		virtual void Map(JUUID so) { sceneObject = so; }
		virtual void Unmap() { sceneObject.clear(); }
	};

	JUUID RegisterController(std::unique_ptr<Controller>& controller, JUUID sceneObject);
	void UnregisterController(JUUID controllerUUID);
	void DestroyControllers();
	void StepControllers(float delta);

	extern std::vector<std::string> GetGameControllers();
	extern std::unique_ptr<Game::Controller> GetGameController(std::string name);
};
