#include "pch.h"
#include "SceneObject.h"
#if defined(_EDITOR)
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>
#endif
#include <Scene.h>
#include <Controller.h>

namespace Scene
{
	void SceneObject::Initialize()
	{
	}

	void SceneObject::BindToScene()
	{
	}

	void SceneObject::JUpdate(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::levelModified = true;
#endif
		JObject::JUpdate(p);
	}

	void SceneObject::JPatch(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::levelModified = true;
#endif
		JObject::JPatch(p);
	}

	void SceneObject::BindControllers()
	{
		using namespace Game;

		if (!contains("controllers")) return;

		auto controllers = at("controllers");
		for (auto it = controllers.begin(); it != controllers.end(); it++)
		{
			std::shared_ptr<Controller> controller = GetGameController(*it);
			if (!controller) continue;

			Game::RegisterController(controller, ThisPtr());
		}
	}
}