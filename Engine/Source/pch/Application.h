#pragma once

static const std::string defaultLevelsFolder = "Levels/";
static const std::string defaultTemplatesFolder = "Templates/";
static const std::string defaultShadersFolder = "Shaders/";
static const std::string defaultAssetsFolder = "Assets/";
static const std::string default3DModelsFolder = "Assets/models/";

typedef std::tuple<std::string, std::string> UUIDName;

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