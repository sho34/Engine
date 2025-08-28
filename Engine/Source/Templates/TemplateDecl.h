#define TEMPDECL_TUPLE(TemplateName) typedef std::tuple<std::string,std::shared_ptr<TemplateName##Json>> TemplateName##Template
#define TEMPDECL_GETTEMPLATES(TemplateName) TemplatesContainer<TemplateName##Template>& Get##TemplateName##Templates()
#define TEMPDECL_CREATE(TemplateName) void Create##TemplateName(nlohmann::json json)
#define TEMPDECL_GET(TemplateName) std::shared_ptr<TemplateName##Json> Get##TemplateName##Template(std::string uuid)
#define TEMPDECL_GETUUIDNAMES(TemplateName) std::vector<UUIDName> Get##TemplateName##sUUIDsNames()
#define TEMPDECL_GETNAMES(TemplateName) std::vector<std::string> Get##TemplateName##sNames()
#define TEMPDECL_GETNAME(TemplateName) std::string Get##TemplateName##Name(std::string uuid)
#define TEMPDECL_FINDUUIDBYNAME(TemplateName) std::string Find##TemplateName##UUIDByName(std::string name)
#define TEMPDECL_WRITEJSON(TemplateName) void Write##TemplateName##sJson(nlohmann::json& json)
#define TEMPDECL_RELEASE(TemplateName) void Release##TemplateName##Templates()
#define TEMPDECL_EXIST(TemplateName) bool TemplateName##TemplateExist(std::string uuid)
#define TEMPDECL_RENAME(TemplateName) void Rename##TemplateName(std::string uuid, std::string newName)

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
	TEMPDECL_RELEASE(TemplateName);\
	TEMPDECL_EXIST(TemplateName);\
	TEMPDECL_RENAME(TemplateName)

#define TEMPDECL_REFTRACKER(TemplateName)\
	std::shared_ptr<TemplateName##Instance> Get##TemplateName##Instance(\
		std::string uuid,\
		std::function<std::shared_ptr<TemplateName##Instance>()> newRefCallback\
	);\
	std::shared_ptr<TemplateName##Instance> Get##TemplateName##Instance(std::string uuid);\
	bool Remove##TemplateName##Instance(std::string uuid, std::shared_ptr<TemplateName##Instance> &instance);\
	bool Remove##TemplateName##Instance(std::function<std::string()> getKey, std::shared_ptr<TemplateName##Instance> &instance);\
	std::shared_ptr<TemplateName##Instance> Find##TemplateName##Instance(std::string uuid);

#define TEMPLATE_DECL(TemplateName)\
	TemplateName##Json(nlohmann::json json);
