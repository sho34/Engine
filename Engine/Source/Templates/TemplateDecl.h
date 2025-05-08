
#define TEMPDECL_TUPLE(TemplateName) typedef std::tuple<std::string,nlohmann::json> TemplateName##Template
#define TEMPDECL_GETTEMPLATES(TemplateName) TemplatesContainer<TemplateName##Template>& Get##TemplateName##Templates()
#define TEMPDECL_CREATE(TemplateName) void Create##TemplateName(nlohmann::json json)
#define TEMPDECL_GET(TemplateName) nlohmann::json& Get##TemplateName##Template(std::string uuid)
#define TEMPDECL_GETUUIDNAMES(TemplateName) std::vector<UUIDName> Get##TemplateName##sUUIDsNames()
#define TEMPDECL_GETNAMES(TemplateName) std::vector<std::string> Get##TemplateName##sNames()
#define TEMPDECL_GETNAME(TemplateName) std::string Get##TemplateName##Name(std::string uuid)
#define TEMPDECL_FINDUUIDBYNAME(TemplateName) std::string Find##TemplateName##UUIDByName(std::string name)
#define TEMPDECL_WRITEJSON(TemplateName) void Write##TemplateName##sJson(nlohmann::json& json)
#define TEMPDECL_RELEASE(TemplateName) void ReleaseShaderTemplates()

#define TEMPDECL_FULL(TemplateName) \
	TEMPDECL_TUPLE(TemplateName);\
	TEMPDECL_GETTEMPLATES(TemplateName);\
	TEMPDECL_CREATE(TemplateName);\
	TEMPDECL_GET(TemplateName);\
	TEMPDECL_GETUUIDNAMES(TemplateName);\
	TEMPDECL_GETNAMES(TemplateName);\
	TEMPDECL_GETNAME(TemplateName);\
	TEMPDECL_FINDUUIDBYNAME(TemplateName);\
	TEMPDECL_WRITEJSON(TemplateName);\
	TEMPDECL_RELEASE(TemplateName)

#define TEMPDECL_REFTRACKER(TemplateName) std::shared_ptr<TemplateName##Instance> Get##TemplateName##Instance(std::string uuid)