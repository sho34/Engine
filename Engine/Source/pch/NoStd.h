#pragma once

namespace nostd {

	template<typename T, typename E>
	inline bool bytesHas(T bytes, E flag) {
		return (bytes & flag) == flag;
	}

	template<typename T>
	inline std::shared_ptr<T> dumb_shared_ptr(T* t) { return std::shared_ptr<T>(t, [](T*) {}); }

	template<typename T>
	inline void vector_erase(std::vector<T>& vec, T value) {
		auto it = std::find(vec.begin(), vec.end(), value);
		if (it != vec.end()) { vec.erase(it); }
	}

	inline std::vector<std::string> split(const std::string& str, const std::string& regex_str) {
		std::regex regexz(regex_str);
		std::vector<std::string> list(
			std::sregex_token_iterator(str.begin(), str.end(), regexz, -1),
			std::sregex_token_iterator()
		);
		return list;
	}

	inline std::string join(const std::vector<std::string>& strings, const std::string delim = ",")
	{
		return std::accumulate(strings.begin(), strings.end(), std::string(""), [&strings, delim](std::string a, std::string b)
			{
				return std::move(a) + (b == strings[0] ? "" : delim) + b;
			}
		);
	}

	inline std::vector<std::string> readVectorFromIfstream(std::ifstream& file, std::string delim = ",")
	{
		std::vector<std::string> strings;
		size_t size;
		file.read(reinterpret_cast<char*>(&size), sizeof(size));
		if (size > 0ULL)
		{
			std::string str;
			str.resize(size);
			file.read(str.data(), size);
			strings = nostd::split(str, delim);
		}
		return strings;
	}

	inline void writeVectorToOfsream(std::ofstream& file, std::vector<std::string> strings, std::string delim = ",")
	{
		std::string str = nostd::join(strings);
		size_t size = str.size();
		file.write(reinterpret_cast<char*>(&size), sizeof(size));
		if (size > 0ULL)
		{
			file.write(str.c_str(), size);
		}
	}

	inline void loadMapFromIfstream(std::ifstream& file, auto& map, auto readPair)
	{
		size_t size;
		file.read(reinterpret_cast<char*>(&size), sizeof(size));
		for (size_t i = 0ULL; i < size; i++)
		{
			map.insert(readPair(file));
		}
	}

	inline void writeMapToOfstream(std::ofstream& file, auto& map, auto writePair)
	{
		size_t size = map.size();
		file.write(reinterpret_cast<char*>(&size), sizeof(size));
		if (size > 0ULL)
		{
			std::for_each(map.begin(), map.end(), [writePair, &file](auto& pair)
				{
					writePair(file, pair);
				}
			);
		}
	}

	inline std::string WStringToString(std::wstring wstr) {
		std::string str;
		std::transform(wstr.begin(), wstr.end(), std::back_inserter(str), [](wchar_t c) { return (char)c; });
		return str;
	}

	inline std::wstring StringToWString(std::string str) {
		std::wstring wstr(str.begin(), str.end());
		return wstr;
	}

	template<typename T>
	inline std::vector<std::string> GetKeysFromMap(std::map<std::string, T>& map) {
		std::vector<std::string> names;
		std::transform(map.begin(), map.end(), std::back_inserter(names), [](std::pair<std::string, T> pair) { return pair.first; });
		return names;
	}

	template<typename T>
	inline void RenameKey(std::map<std::string, T>& map, std::string from, std::string to)
	{
		map[to] = map[from];
		map.erase(from);
	}

	inline std::vector<std::string> GetKeysFromSet(std::set<std::string>& set) {
		std::vector<std::string> names;
		std::transform(set.begin(), set.end(), std::back_inserter(names), [](std::string name) { return name; });
		return names;
	}

	template<typename T>
	inline void AppendToVector(std::vector<T>& dst, std::vector<T>& src)
	{
		dst.insert(dst.end(), src.begin(), src.end());
	}

	template<typename T>
	inline T GetValueFromMap(std::map<std::string, T>& map, const std::string& key) {
		auto it = map.find(key);
		return (it != map.end()) ? it->second : nullptr;
	}

	template<typename K, typename V>
	inline K GetKeyFromValueInMap(std::map<K, V> map, V v) {
		for (auto it : map) {
			if (it.second == v) return it.first;
		}
		return K(nullptr);
	}

	template<typename a, typename b>
	inline void EraseByValue(std::map<a, b>& map, b value) {
		for (auto it = map.begin(); it != map.end();) {
			it = (it->second == value) ? map.erase(it) : std::next(it);
		}
	}

	template<typename T>
	inline void VecN_push_back(unsigned int n, std::vector<T>& vec) {
		for (unsigned int i = 0; i < n; i++) {
			vec.push_back(T());
		}
	}

	template<typename T>
	void ReplaceFromJsonUsingMap(T& value, std::map<std::string, T> stringToT, nlohmann::json& object, const std::string& key) {
		if (object.contains(key) && stringToT.contains(object.at(key))) value = static_cast<T>(stringToT.at(object.at(key)));
	}

	template<typename T>
	void InsertFromJsonUsingMap(std::vector<T>& vecValues, std::map<std::string, T> stringToT, nlohmann::json& object, const std::string& key) {
		if (!object.contains(key)) return;
		nlohmann::json& jvalues = object.at(key);
		std::transform(jvalues.begin(), jvalues.end(), std::back_inserter(vecValues), [stringToT](const nlohmann::json& value)
			{
				return static_cast<T>(stringToT.at(value));
			}
		);
	}

	template<typename T>
	void PushBackFromJson(std::vector<T>& vecValues, nlohmann::json object, const std::string& key) {
		if (!object.contains(key)) return;
		nlohmann::json& jvalues = object.at(key);
		std::transform(jvalues.begin(), jvalues.end(), std::back_inserter(vecValues), [](const nlohmann::json& value)
			{
				return static_cast<T>(value);
			}
		);
	}

	template<typename T, typename V>
	bool in_between(T value, V min, V max)
	{
		return static_cast<T>(min) <= value && value <= static_cast<T>(max);
	}

	template<typename URBG>
	std::string gen_string(std::size_t length, URBG&& g) {
		static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		std::string result;
		result.resize(length);
		std::sample(std::cbegin(charset), std::cend(charset), std::begin(result), std::intptr_t(length), std::forward<URBG>(g));
		return result;
	}

	inline void ToLower(std::string& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
	}

	// trim from start (in place)
	inline void ltrim(std::string& s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
	}

	// trim from end (in place)
	inline void rtrim(std::string& s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
	}

	inline void trim(std::string& s) {
		rtrim(s); ltrim(s);
	}

	template <typename T, typename... Rest>
	void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
	{
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hash_combine(seed, rest), ...);
	}

	template<typename K, typename V, typename C = std::less<K>>
	struct RefTracker
	{
		std::map<K, V, C> instances;
		std::map<V, unsigned int> instancesRefCount;

		V AddRef(K key, std::function<V()> newRefCallback = []() { return V(); })
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
			return instancesRefCount.at(instancesRefCount);
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
	};
}
