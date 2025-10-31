#include "pch.h"
#include "Binder.h"
#include <Scene.h>
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>
#include <Light/Light.h>
#include <Sound/SoundFX.h>

using namespace Scene;

std::map<SceneObjectType, std::function<void(JUUID, JUUID)>> BindFnc =
{
	{ SO_Renderables, [](JUUID uuid, JUUID uuidB)
		{
			auto& so = GetRenderableSceneObject(uuid);
			so->Bind(uuidB);
		}
	},
	{ SO_Cameras, [](JUUID uuid, JUUID uuidB)
		{
			auto& so = GetCameraSceneObject(uuid);
			so->Bind(uuidB);
		}
	},
	{ SO_Lights, [](JUUID uuid, JUUID uuidB)
		{
			auto& so = GetLightSceneObject(uuid);
			so->Bind(uuidB);
		}
	},
	{ SO_SoundEffects, [](JUUID uuid, JUUID uuidB)
		{
			auto& so = GetSoundFXSceneObject(uuid);
			so->Bind(uuidB);
		}
	}
};
std::map<SceneObjectType, std::function<void(JUUID, JUUID)>> UnbindFnc =
{
	{ SO_Renderables, [](JUUID uuid, JUUID uuidB)
		{
			auto& so = GetRenderableSceneObject(uuid);
			so->Unbind(uuidB);
		}
	},
	{ SO_Cameras, [](JUUID uuid, JUUID uuidB)
		{
			auto& so = GetCameraSceneObject(uuid);
			so->Unbind(uuidB);
		}
	},
	{ SO_Lights, [](JUUID uuid, JUUID uuidB)
		{
			auto& so = GetLightSceneObject(uuid);
			so->Unbind(uuidB);
		}
	},
	{ SO_SoundEffects, [](JUUID uuid, JUUID uuidB)
		{
			auto& so = GetSoundFXSceneObject(uuid);
			so->Unbind(uuidB);
		}
	}
};

void Binder::insert(JUUID soA, JUUID soB)
{
	bool AtoB = false;
	auto rangeAtoB = binding.equal_range(soA);
	for (auto& it = rangeAtoB.first; it != rangeAtoB.second; it++)
	{
		if (it->second == soB)
		{
			AtoB = true;
			break;
		}
	}
	if (!AtoB)
	{
		binding.insert({ soA,soB });
		BindFnc.at(GetSceneObjectType(soA))(soA, soB);
	}

	bool BtoA = false;
	auto rangeBtoA = binding.equal_range(soB);
	for (auto& it = rangeBtoA.first; it != rangeBtoA.second; it++)
	{
		if (it->second == soA)
		{
			BtoA = true;
			break;
		}
	}
	if (!BtoA)
	{
		binding.insert({ soB,soA });
		BindFnc.at(GetSceneObjectType(soB))(soB, soA);
	}
}

void Binder::erase(JUUID soA)
{
	std::set<JUUID> soBs;
	auto rangeA = binding.equal_range(soA);
	for (auto it = rangeA.first; it != rangeA.second; it++)
	{
		soBs.insert(it->second);
	}
	binding.erase(soA);
	for (auto soB : soBs)
	{
		auto rangeB = binding.equal_range(soB);
		for (auto it = rangeB.first; it != rangeB.second; )
		{
			if (it->second == soA)
			{
				it = binding.erase(it);
				if (SceneObjectExists(soA) && SceneObjectExists(soB))
				{
					UnbindFnc.at(GetSceneObjectType(soA))(soA, soB);
					UnbindFnc.at(GetSceneObjectType(soB))(soB, soA);
				}
			}
			else
				it++;
		}
	}
}

void Binder::erase(JUUID soA, JUUID soB)
{
	UnbindFnc.at(GetSceneObjectType(soA))(soA, soB);
	UnbindFnc.at(GetSceneObjectType(soB))(soB, soA);

	auto rangeA = binding.equal_range(soA);
	for (auto it = rangeA.first; it != rangeA.second; )
	{
		if (it->second == soB)
			it = binding.erase(it);
		else
			it++;
	}
	auto rangeB = binding.equal_range(soB);
	for (auto it = rangeB.first; it != rangeB.second; )
	{
		if (it->second == soA)
			it = binding.erase(it);
		else
			it++;
	}
}