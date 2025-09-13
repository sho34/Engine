#if defined(JEXPOSE_TRACK_UUID_DECL)
#define JCLASS(CLASS,GETJOBJECTS)
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JTRACKUUID(CLASS,NAME,LIMIT,COND) \
	size_t GetNum##NAME();\
	std::vector<std::shared_ptr<CLASS>> Get##NAME();\
	std::shared_ptr<CLASS> FindIn##NAME(std::string uuid);\
	std::shared_ptr<CLASS> FindIn##NAME##ByName(std::string name);\
	std::vector<std::string> Get##NAME##Names();\
	std::vector<UUIDName> Get##NAME##UUIDNames();\
	std::string FindNameIn##NAME(std::string uuid);\
	void Insert##CLASS##Into##NAME(std::shared_ptr<CLASS> v);\
	void Erase##CLASS##From##NAME(std::shared_ptr<CLASS> v);

#endif

#if defined(JEXPOSE_TRACK_UUID_DEF)
#define JCLASS(CLASS,GETJOBJECTS)
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JTRACKUUID(CLASS,NAME,LIMIT,COND) \
	static std::map<std::string, std::shared_ptr<CLASS>> NAME;\
	size_t GetNum##NAME(){ return NAME.size(); } \
	std::vector<std::shared_ptr<CLASS>> Get##NAME()\
	{\
		std::vector<std::shared_ptr<CLASS>> v;\
		std::transform(NAME.begin(),NAME.end(),std::back_inserter(v),[](auto& pair){ return pair.second; });\
		return v;\
	}\
	std::shared_ptr<CLASS> FindIn##NAME(std::string uuid) { return NAME.contains(uuid)?NAME.at(uuid):nullptr; }\
	std::shared_ptr<CLASS> FindIn##NAME##ByName(std::string name)\
	{\
		auto it = std::find_if(NAME.begin(),NAME.end(),[name](auto& pair)\
			{\
				return pair.second->name()==name;\
			}\
			); \
			return (it != NAME.end()) ? it->second: nullptr;\
	}\
	std::vector<std::string> Get##NAME##Names() {\
		std::map<std::string, std::shared_ptr<CLASS>> j;\
		std::copy_if(NAME.begin(), NAME.end(), std::inserter(j, j.end()), [](const auto& pair)\
			{\
				return !bool(pair.second->at("hidden"));\
			}\
		);\
		std::vector<std::string> v;\
		std::transform(j.begin(),j.end(),std::back_inserter(v),[](const auto& pair)\
			{\
				return pair.second->at("name");\
			}\
		);\
		return v;\
	}\
	std::vector<UUIDName> Get##NAME##UUIDNames()\
	{\
		std::map<std::string, std::shared_ptr<CLASS>> j; \
		std::copy_if(NAME.begin(), NAME.end(), std::inserter(j, j.end()), [](const auto& pair)\
			{\
				return !bool(pair.second->at("hidden")); \
			}\
		);\
		std::vector<UUIDName> v;\
		std::transform(j.begin(),j.end(),std::back_inserter(v),[](const auto& pair)\
			{\
				UUIDName uuidName;\
				std::string& uuid = std::get<0>(uuidName);\
				uuid = pair.first;\
				std::string& name = std::get<1>(uuidName);\
				name = pair.second->at("name");\
				return uuidName;\
			}\
		);\
		return v;\
	}\
	std::string FindNameIn##NAME(std::string uuid) { return NAME.at(uuid)->name(); }\
	void Insert##CLASS##Into##NAME(std::shared_ptr<CLASS> v)\
	{\
		if(LIMIT>0 && !NAME.contains(v->uuid()))\
		assert(NAME.size()<LIMIT);\
		NAME.insert_or_assign(v->uuid(), v);\
	}\
	void Erase##CLASS##From##NAME(std::shared_ptr<CLASS> v)\
	{\
		NAME.erase(v->uuid());\
	}

#endif

#if defined(JEXPOSE_TRACK_UUID_INSERT)
#define JCLASS(CLASS,GETJOBJECTS)
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JTRACKUUID(CLASS,NAME,LIMIT,COND) if(COND) { Insert##CLASS##Into##NAME(this_ptr); }

#endif

#if defined(JEXPOSE_TRACK_UUID_ERASE)
#define JCLASS(CLASS,GETJOBJECTS)
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JTRACKUUID(CLASS,NAME,LIMIT,COND) Erase##CLASS##From##NAME(this_ptr);

#endif

#if defined(JEXPOSE_TRACK_UUID_CLEAR)
#define JCLASS(CLASS,GETJOBJECTS)
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JTRACKUUID(CLASS,NAME,LIMIT,COND) NAME.clear();

#endif
