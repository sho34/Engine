#pragma once

#include <map>
#include <JObject.h>
#include <nlohmann/json.hpp>
#include <ImGuizmo.h>
#if defined(_EDITOR)
#include <IconsFontAwesome5.h>
#endif
#include <DirectXMath.h>

enum SceneObjectType {
	SO_None,
	SO_Renderables,
	SO_Lights,
	SO_Cameras,
	SO_SoundEffects
};

inline const std::map<SceneObjectType, std::string> SceneObjectTypeToString = {
	{ SO_Renderables, "Renderables" },
	{ SO_Lights,	"Lights" },
	{ SO_Cameras, "Cameras" },
	{ SO_SoundEffects, "SoundEffects" }
};

inline const std::map<std::string, SceneObjectType> StringToSceneObjectType = {
	{ "Renderables", SO_Renderables },
	{ "Lights", SO_Lights },
	{ "Cameras", SO_Cameras },
	{ "SoundEffects", SO_SoundEffects }
};

#if defined(_EDITOR)
inline const std::map<SceneObjectType, const char* > SceneObjectsTypePanelMenuItems = {
	{ SO_Renderables, ICON_FA_SNOWMAN "Renderables" },
	{ SO_Lights, ICON_FA_LIGHTBULB "Lights" },
	{ SO_Cameras, ICON_FA_CAMERA "Cameras" },
	{ SO_SoundEffects, ICON_FA_MUSIC "SoundEffects" }
};
#endif

namespace Scene
{
	struct SceneObject : JObject
	{
		SceneObject(nlohmann::json json) :JObject(json) {}
		virtual void Initialize() {};
		virtual void BindToScene() {};
		virtual void UnbindFromScene() {};
		virtual XMVECTOR rotationQ() { return XMQuaternionIdentity(); }
		virtual XMMATRIX world() { return XMMatrixIdentity(); }
		virtual BoundingBox GetBoundingBox() { return BoundingBox(); };
		virtual void Bind(std::shared_ptr<SceneObject> sceneObject) {}
		virtual void Unbind(std::shared_ptr<SceneObject> sceneObject) {}
		virtual SceneObjectType JType() { return SO_None; }
	};
};