#pragma once
#include <functional>
#include <SceneObject.h>
#include <map>

struct Binder {
	std::multimap<std::shared_ptr<SceneObject>, std::shared_ptr<SceneObject>> binding;

	void insert(std::shared_ptr<SceneObject> soA, std::shared_ptr<SceneObject> soB)
	{
		bool AtoB = false;
		auto rangeAtoB = binding.equal_range(soA);
		for (auto it = rangeAtoB.first; it != rangeAtoB.second; it++)
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
			soA->Bind(soB);
		}

		bool BtoA = false;
		auto rangeBtoA = binding.equal_range(soB);
		for (auto it = rangeBtoA.first; it != rangeBtoA.second; it++)
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
			soB->Bind(soA);
		}
	}

	void erase(std::shared_ptr<SceneObject> soA)
	{
		std::set<std::shared_ptr<SceneObject>> soBs;
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
					soA->Unbind(soB);
					soB->Unbind(soA);
				}
				else
					it++;
			}
		}
	}

	void erase(std::shared_ptr<SceneObject> soA, std::shared_ptr<SceneObject> soB)
	{
		soA->Unbind(soB);
		soB->Unbind(soA);
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

};
