
#define TEMPDECL_CREATE(TemplateName) void Create##TemplateName(nlohmann::json json)
#define TEMPDECL_GET(TemplateName) nlohmann::json Get##TemplateName##Template(std::string uuid)
#define TEMPDECL_GETUUIDNAMES(TemplateName) std::vector<UUIDName> Get##TemplateName##sUUIDsNames()
#define TEMPDECL_GETNAMES(TemplateName) std::vector<std::string> Get##TemplateName##sNames()
#define TEMPDECL_GETNAME(TemplateName) std::string Get##TemplateName##Name(std::string uuid)
#define TEMPDECL_FINDUUIDBYNAME(TemplateName) std::string Find##TemplateName##UUIDByName(std::string name)

#define TEMPDECL_FULL(TemplateName) \
	TEMPDECL_CREATE(TemplateName);\
	TEMPDECL_GET(TemplateName);\
	TEMPDECL_GETUUIDNAMES(TemplateName);\
	TEMPDECL_GETNAMES(TemplateName);\
	TEMPDECL_GETNAME(TemplateName);\
	TEMPDECL_FINDUUIDBYNAME(TemplateName)

