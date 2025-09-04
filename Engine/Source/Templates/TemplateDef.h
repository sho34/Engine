#define TEMPDEF_TUPLE(TemplateName) std::map<std::string, TemplateName##Template> TemplateName##templates

#define TEMPDEF_GETTEMPLATES(TemplateName) TemplatesContainer<TemplateName##Template>& Get##TemplateName##Templates() { return TemplateName##templates ; }

#define TEMPDEF_CREATE(TemplateName) void Create##TemplateName(nlohmann::json json)\
{\
	CreateJsonTemplate<TemplateName##Template,TemplateName##Json>(json, Get##TemplateName##Templates);\
}

#define TEMPDEF_GET(TemplateName) std::shared_ptr<TemplateName##Json> Get##TemplateName##Template(std::string uuid)\
{\
	return TemplateName##templates.contains(uuid)?std::get<1>(Get##TemplateName##Templates().at(uuid)):nullptr;\
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

#define TEMPDEF_EXIST(TemplateName) bool TemplateName##TemplateExist(std::string uuid)\
{\
	return Get##TemplateName##Templates().contains(uuid);\
}

#define TEMPDEF_RENAME(TemplateName) void Rename##TemplateName(std::string uuid,std::string newName)\
{\
	std::string& refName= std::get<0>(Get##TemplateName##Templates().at(uuid));\
	refName = newName;\
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
	TEMPDEF_RELEASE(TemplateName);\
	TEMPDEF_EXIST(TemplateName);\
	TEMPDEF_RENAME(TemplateName)

#define TEMPDEF_INSTANCECALLBACK(TemplateName,uuid) []\
{\
	return std::make_shared<TemplateName##Instance>(uuid); \
}

#define TEMPDEF_REFTRACKER(TemplateName) static RefTracker<std::string, std::shared_ptr<TemplateName##Instance>> refTracker; \
\
std::shared_ptr<TemplateName##Instance>\
Get##TemplateName##Instance(\
	std::string uuid,\
	std::function<std::shared_ptr<TemplateName##Instance>()> newRefCallback\
)\
{\
	if(refTracker.Has(uuid))\
		return refTracker.FindValue(uuid);\
	else\
		return refTracker.AddRef(uuid, newRefCallback);\
}\
std::shared_ptr<TemplateName##Instance> Get##TemplateName##Instance(std::string uuid)\
{\
	if(refTracker.Has(uuid))\
		return refTracker.FindValue(uuid);\
	else\
		return refTracker.AddRef(uuid, [uuid]{return std::make_shared<TemplateName##Instance>(uuid);});\
}\
bool Remove##TemplateName##Instance(std::string uuid, std::shared_ptr<TemplateName##Instance>& instance)\
{\
	if(refTracker.Has(uuid))\
	{\
		refTracker.RemoveRef(uuid,instance);\
		return true;\
	}\
	return false;\
}\
bool Remove##TemplateName##Instance(std::function<std::string()> getKey, std::shared_ptr<TemplateName##Instance>& instance)\
{\
	std::string uuid = getKey();\
	return Remove##TemplateName##Instance(uuid,instance);\
}\
std::shared_ptr<TemplateName##Instance> Find##TemplateName##Instance(std::string uuid)\
{\
	if(refTracker.Has(uuid))\
		return refTracker.FindValue(uuid); \
	return nullptr;\
}
