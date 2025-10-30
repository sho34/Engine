#include "pch.h"
#include "Controller.h"
#include <map>
#include <NoStd.h>

namespace Game
{
	std::unordered_map<JUUID, std::unique_ptr<Controller>> controllersUUIDs;

	JUUID RegisterController(std::unique_ptr<Controller>& controller, JUUID sceneObject)
	{
		JUUID controllerUUID = getUUID();
		controller->Map(sceneObject);
		controllersUUIDs.insert_or_assign(controllerUUID, std::move(controller));
		return controllerUUID;
	}

	void UnregisterController(JUUID controllerUUID)
	{
		if (controllersUUIDs.contains(controllerUUID))
		{
			auto& controller = controllersUUIDs.at(controllerUUID);
			controller->Unmap();
			controllersUUIDs.erase(controllerUUID);
		}
	}

	void DestroyControllers()
	{
		for (auto it = controllersUUIDs.begin(); it != controllersUUIDs.end();)
		{
			it->second->Unmap();
			it = controllersUUIDs.erase(it);
		}
	}

	void StepControllers(float delta)
	{
		for (auto& [_, c] : controllersUUIDs)
		{
			c->Step(delta);
		}
	}
}
