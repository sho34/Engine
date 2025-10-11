#pragma once
#include <Controller.h>

namespace Scene
{
	struct Renderable;
	struct Camera;
};

namespace Game
{
	struct CameraController : Controller
	{
		std::shared_ptr<Scene::Renderable> venom;
		std::shared_ptr<Scene::Camera> camera;

		virtual void Map(std::shared_ptr<Scene::SceneObject> so);
		virtual void Unmap();
		virtual void Step(float delta);
	};
};