#pragma once

typedef std::tuple<std::string, std::string> UUIDName;

//Create a new UUID
inline std::string getUUID()
{
	UUID uuid = { 0 };
	std::string guid;

	// Create uuid or load from a string by UuidFromString() function
	::UuidCreate(&uuid);

	// If you want to convert uuid to string, use UuidToString() function
	RPC_CSTR szUuid = NULL;
	if (::UuidToStringA(&uuid, &szUuid) == RPC_S_OK)
	{
		guid = (char*)szUuid;
		::RpcStringFreeA(&szUuid);
	}

	return guid;
}

inline int FindSelectableIndex(auto selectables, nlohmann::json& json, auto att)
{
	auto& value = json.at(att);
	return static_cast<int>(std::find_if(selectables.begin(), selectables.end(), [value](UUIDName uuidName)
		{
			return value == std::get<0>(uuidName);
		}
	) - selectables.begin());
}

inline std::function<std::vector<UUIDName>()> SortUUIDNameByName(std::function<std::vector<UUIDName>()> getUUIDNames)
{
	return [getUUIDNames]()
		{
			std::vector<UUIDName> uuidNames = getUUIDNames();
			std::sort(uuidNames.begin(), uuidNames.end(), [](UUIDName a, UUIDName b)
				{
					return std::get<1>(a) < std::get<1>(b);
				}
			);
			return uuidNames;
		};
};

inline void SortUUIDByName(std::vector<UUIDName>& uuidNames)
{
	std::sort(uuidNames.begin(), uuidNames.end(), [](UUIDName a, UUIDName b)
		{
			return std::get<1>(a) < std::get<1>(b);
		}
	);
}
