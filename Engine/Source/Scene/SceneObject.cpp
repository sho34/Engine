#include "pch.h"
#include "SceneObject.h"
#if defined(_EDITOR)
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>
#endif
#include <Scene.h>

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
}