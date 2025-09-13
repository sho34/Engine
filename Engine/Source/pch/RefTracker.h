#pragma once
#include <functional>

template<typename K, typename V, typename C = std::less<K>>
struct RefTracker
{
	std::map<K, V, C> instances;
	std::map<V, unsigned int> instancesRefCount;

	V AddRef(K key, std::function<V()> newRefCallback)
	{
		V instance;
		if (instances.contains(key))
		{
			instance = instances.at(key);
		}
		else
		{
			instance = newRefCallback();
			instances.insert_or_assign(key, instance);
			instancesRefCount.insert_or_assign(instance, 0U);
		}
		instancesRefCount.find(instance)->second++;
		return instance;
	}

	void IncrementRefCount(V& instance, unsigned int d)
	{
		instancesRefCount.find(instance)->second += d;
	}

	void RemoveRef(K key, V& instance)
	{
		assert(instancesRefCount.contains(instance));

		instancesRefCount.at(instance)--;
		if (instancesRefCount.at(instance) == 0U)
		{
			instancesRefCount.erase(instance);
			instances.erase(key);
		}
		instance = nullptr;
	}

	bool Has(K k)
	{
		return instances.contains(k);
	}

	unsigned int Count(V& instance)
	{
		assert(instancesRefCount.contains(instance));
		return instancesRefCount.at(instance);
	}

	void Clear()
	{
		instances.clear();
		instancesRefCount.clear();
	}

	K FindKey(V& instance)
	{
		for (auto it = instances.begin(); it != instances.end(); it++)
		{
			if (it->second == instance) { return it->first; }
		}
		return K();
	}

	V FindValue(K key)
	{
		return instances.at(key);
	}

	void RenameKey(K from, K to)
	{
		instances[to] = instances[from];
		instances.erase(from);
	}

	void ForEach(std::function<void(V&)> cb)
	{
		for (auto it = instances.begin(); it != instances.end(); it++)
		{
			cb(it->second);
		}
	}
};