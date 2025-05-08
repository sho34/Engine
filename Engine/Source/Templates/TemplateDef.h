#define TEMPDEF_TUPLE(TemplateName) std::map<std::string, TemplateName##Template> templates

#define TEMPDEF_GETTEMPLATES(TemplateName) TemplatesContainer<TemplateName##Template>& Get##TemplateName##Templates() { return templates ; }

#define TEMPDEF_CREATE(TemplateName) void Create##TemplateName(nlohmann::json json)\
{\
	CreateJsonTemplate<TemplateName##Template>(json, Get##TemplateName##Templates);\
}

#define TEMPDEF_GET(TemplateName) nlohmann::json& Get##TemplateName##Template(std::string uuid)\
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

#define TEMPDEF_WRITEJSON(TemplateName) void Write##TemplateName##sJson(nlohmann::json& json)\
{\
	WriteTemplateJson(json, Get##TemplateName##Templates());\
}

#define TEMPDEF_RELEASE(TemplateName) void Release##TemplateName##Templates()\
{\
	Get##TemplateName##Templates().clear();\
}

#define TEMPDEF_FULL(TemplateName) \
	TEMPDEF_TUPLE(TemplateName);\
	TEMPDEF_GETTEMPLATES(TemplateName);\
	TEMPDEF_CREATE(TemplateName);\
	TEMPDEF_GET(TemplateName);\
	TEMPDEF_GETUUIDNAMES(TemplateName);\
	TEMPDEF_GETNAMES(TemplateName);\
	TEMPDEF_GETNAME(TemplateName);\
	TEMPDEF_FINDUUIDBYNAME(TemplateName);\
	TEMPDEF_WRITEJSON(TemplateName);\
	TEMPDEF_RELEASE(TemplateName)

#define TEMPDEF_REFTRACKER(TemplateName) static nostd::RefTracker<std::string, std::shared_ptr<TemplateName##Instance>> refTracker; \
std::shared_ptr<TemplateName##Instance> Get##TemplateName##Instance(std::string uuid)\
{\
	if(refTracker.Has(uuid))\
		return refTracker.FindValue(uuid);\
	return nullptr;\
}
