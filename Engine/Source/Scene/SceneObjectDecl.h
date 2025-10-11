#pragma once

#define SCENEOBJECT_DECL(TYPE)\
	TYPE(nlohmann::json json);\
	~TYPE() { Destroy(); }\
	void BindToScene();\
	void UnbindFromScene();\
	std::shared_ptr<TYPE> this_ptr = nullptr;\
	virtual std::shared_ptr<SceneObject> ThisPtr() { return this_ptr; }

