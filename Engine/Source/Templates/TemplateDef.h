
#define TEMPDEF_CREATE(TemplateName) void Create##TemplateName(nlohmann::json json)\
{\
	CreateJsonTemplate<TemplateName##Template>(json, Get##TemplateName##Templates);\
}

#define TEMPDEF_GET(TemplateName) nlohmann::json Get##TemplateName##Template(std::string uuid)\
{\
	return GetTemplate(uuid, Get##TemplateName##Templates);\
}

#define TEMPDEF_GETUUIDNAMES(TemplateName) std::vector<UUIDName> Get##TemplateName##sUUIDsNames()\
{\
	return GetUUIDsNames(Get##TemplateName##Templates());\
}

#define TEMPDEF_GETNAMES(TemplateName) std::vector<std::string> Get##TemplateName##sNames()\
{\
	return GetNames(Get##TemplateName##Templates());\
}

#define TEMPDEF_GETNAME(TemplateName) std::string Get##TemplateName##Name(std::string uuid)\
{\
	return GetName(uuid, Get##TemplateName##Templates);\
}

#define TEMPDEF_FINDUUIDBYNAME(TemplateName) std::string Find##TemplateName##UUIDByName(std::string name)\
{\
	return FindUUIDByName(name, Get##TemplateName##Templates);\
}

#define TEMPDEF_FULL(TemplateName) \
	TEMPDEF_CREATE(TemplateName);\
	TEMPDEF_GET(TemplateName);\
	TEMPDEF_GETUUIDNAMES(TemplateName);\
	TEMPDEF_GETNAMES(TemplateName);\
	TEMPDEF_GETNAME(TemplateName);\
	TEMPDEF_FINDUUIDBYNAME(TemplateName)
