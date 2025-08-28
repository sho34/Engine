#pragma once

#include <JObject.h>
#include <nlohmann/json.hpp>
#include <ImGuizmo.h>

namespace Scene
{
	struct SceneObject : JObject
	{
		SceneObject(nlohmann::json json) :JObject(json) {}
		virtual void BindToScene() {};
		virtual void UnbindFromScene() {};
		virtual XMVECTOR rotationQ() { return XMQuaternionIdentity(); }
		virtual XMMATRIX world() { return XMMatrixIdentity(); }
		virtual BoundingBox GetBoundingBox() { return BoundingBox(); };
	};
};