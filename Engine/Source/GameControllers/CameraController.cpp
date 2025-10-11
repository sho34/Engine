#include "pch.h"
#include "CameraController.h"
#include <SceneObject.h>
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>

namespace Game
{
	void CameraController::Map(std::shared_ptr<Scene::SceneObject> so)
	{
		Controller::Map(so);
		if (so->JType() == SO_Renderables)
		{
			venom = std::dynamic_pointer_cast<Scene::Renderable>(so);
		}
		camera = GetMouseCameras().size() > 0ULL ? GetMouseCameras().at(0) : nullptr;
	}

	void CameraController::Unmap()
	{
		Controller::Unmap();
		if (venom != nullptr)
		{
			venom = nullptr;
		}
		if (camera != nullptr)
		{
			camera = nullptr;
		}
	}

	void CameraController::Step(float delta)
	{
		if (!camera || !venom) return;

		XMFLOAT3 vpos = venom->position();
		XMFLOAT3 cpos = camera->position();
		cpos.x = vpos.x;
		camera->position(cpos);
	}
};