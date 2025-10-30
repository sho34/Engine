#pragma once
#include <functional>
#include <SceneObject.h>
#include <map>

struct Binder {

	std::multimap<JUUID, JUUID> binding;

	void insert(JUUID soA, JUUID soB);
	void erase(JUUID soA);
	void erase(JUUID soA, JUUID soB);
};
