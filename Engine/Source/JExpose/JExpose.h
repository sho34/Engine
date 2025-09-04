#if defined(JEXPOSE_ATT_JSON_DECL)

#define JCLASS(CLASS) nlohmann::json Create##CLASS##Json();
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JTRACKUUID(CLASS,NAME,LIMIT)

#endif

#if defined(JEXPOSE_ATT_JSON_DEF)

#define JCLASS(CLASS) nlohmann::json Create##CLASS##Json()\
{\
	nlohmann::json created;

#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) \
	if(REQUIREDTOCREATE)\
	{\
		created[#ATT] = INITIAL;\
	}

#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	if(REQUIREDTOCREATE)\
	{\
		created[#ATT] = FROMTYPE(INITIAL);\
	}

#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) \
	if(REQUIREDTOCREATE)\
	{\
		created[#ATT] = TYPE##ToString.at(INITIAL);\
	}

#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	if(REQUIREDTOCREATE)\
	{\
		created[#ATT] = nlohmann::json::array();\
		std::vector<TYPE> vec = std::vector<TYPE>(INITIAL);\
		for (auto& v : vec)\
		{\
			created[#ATT].push_back(v);\
		}\
	}

#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	if(REQUIREDTOCREATE)\
	{\
		created[#ATT] = nlohmann::json::array();\
		std::vector<TYPE> vec = std::vector<TYPE>(INITIAL);\
		for (auto& v : vec)\
		{\
			created[#ATT].push_back(FROMTYPE(v));\
		}\
	}

#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	if(REQUIREDTOCREATE)\
	{\
		created[#ATT] = nlohmann::json::array();\
		std::vector<TYPE> vec = std::vector<TYPE>(INITIAL);\
		std::set<TYPE> iterableSet(vec.begin(),vec.end());\
		for (auto& v : iterableSet)\
		{\
			created[#ATT].push_back(v);\
		}\
	}

#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	if(REQUIREDTOCREATE)\
	{\
		created[#ATT] = nlohmann::json::object({}); \
		std::map<KEYTYPE,VALUETYPE> mapKV(INITIAL);\
		for (auto& pair : mapKV)\
		{\
			created[#ATT].merge_patch(FROMTYPE(pair)); \
		}\
	}

#define JTRACKUUID(CLASS,NAME,LIMIT)

#endif

#if defined(JEXPOSE_ATT_DECL)

#define JCLASS(CLASS)

#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) \
	void create_##ATT(TYPE t)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = t;\
		}\
	}\
	TYPE ATT()\
	{\
		return at(#ATT);\
	}\
	void ATT(TYPE v)\
	{\
		at(#ATT)=v;\
	}\

#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	void create_##ATT(TYPE t)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = FROMTYPE(t);\
		}\
	}\
	TYPE ATT()\
	{\
		if(contains(#ATT))\
		{\
			return TOTYPE(at(#ATT)); \
		}\
		else\
		{\
			return TYPE();\
		}\
	}\
	void ATT(TYPE v)\
	{\
		(*this)[#ATT] = FROMTYPE(v);\
	}\

#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) \
	void create_##ATT(TYPE v)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = TYPE##ToString.at(v);\
		}\
	}\
	TYPE ATT()\
	{\
		return StringTo##TYPE.at(at(#ATT));\
	}\
	void ATT(TYPE v)\
	{\
		at(#ATT)=TYPE##ToString.at(v);\
	}\

#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	void create_##ATT(std::vector<TYPE> vec)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = nlohmann::json::array();\
			for (auto& v : vec)\
			{\
				at(#ATT).push_back(v);\
			}\
		}\
	}\
	std::vector<TYPE> ATT()\
	{\
		std::vector<TYPE> vec;\
		if(contains(#ATT))\
		{\
			for (nlohmann::json::iterator it = at(#ATT).begin(); it != at(#ATT).end(); it++)\
			{\
				vec.push_back(*it);\
			}\
		}\
		return vec;\
	}\
	void ATT(std::vector<TYPE> vec)\
	{\
		at(#ATT) = nlohmann::json::array();\
		for (auto& v : vec)\
		{\
			at(#ATT).push_back(v);\
		}\
	}\
	void ATT##_push_back(TYPE v)\
	{\
		at(#ATT).push_back(v);\
	}

#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	void create_##ATT(std::vector<TYPE> vec)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = nlohmann::json::array();\
			for (auto& v : vec)\
			{\
				at(#ATT).push_back(FROMTYPE(v));\
			}\
		}\
	}\
	std::vector<TYPE> ATT()\
	{\
		std::vector<TYPE> vec;\
		if(contains(#ATT))\
		{\
			for (nlohmann::json::iterator it = at(#ATT).begin(); it != at(#ATT).end(); it++)\
			{\
				vec.push_back(TOTYPE(*it));\
			}\
		}\
		return vec;\
	}\
	void ATT(std::vector<TYPE> vec)\
	{\
		at(#ATT) = nlohmann::json::array();\
		for (auto& v : vec)\
		{\
			at(#ATT).push_back(FROMTYPE(v));\
		}\
	}\
	void ATT##_push_back(TYPE v)\
	{\
		at(#ATT).push_back(FROMTYPE(v));\
	}

#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	void create_##ATT(std::set<TYPE> attset)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = nlohmann::json::array();\
			for (auto& v : attset)\
			{\
				at(#ATT).push_back(v);\
			}\
		}\
	}\
	std::set<TYPE> ATT()\
	{\
		std::set<TYPE> s;\
		if(contains(#ATT))\
		{\
			for (auto& v : at(#ATT))\
			{\
				s.insert(static_cast<TYPE>(v)); \
			}\
		}\
		return s;\
	}\
	bool ATT##_contains(TYPE value)\
	{\
		for (auto& v : at(#ATT)) {\
			if (v == value) return true;\
		}\
		return false;\
	}\
	void ATT##_insert(TYPE value)\
	{\
		for (auto& v : at(#ATT)) {\
			if (v == value) return;\
		}\
		at(#ATT).push_back(value);\
	}\
	void ATT##_erase(TYPE value)\
	{\
		unsigned int idx = -1;\
		for(unsigned int i = 0 ; i < at(#ATT).size(); i++)\
		{\
			if(at(#ATT).at(i)==value)\
			{\
				idx = i;\
				break;\
			}\
		}\
		if (idx != -1) at(#ATT).erase(idx);\
	}\
	size_t ATT##_size()\
	{\
		return at(#ATT).size();\
	}\
	void ATT##_clear()\
	{\
		at(#ATT).clear();\
	}

#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)\
	void create_##ATT(std::map<KEYTYPE,VALUETYPE> map)\
	{\
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = nlohmann::json::object({}); \
			for (auto& pair : map)\
			{\
				at(#ATT).merge_patch(FROMTYPE(pair)); \
			}\
		}\
	}\
	std::map<KEYTYPE, VALUETYPE> ATT()\
	{\
		std::map<KEYTYPE, VALUETYPE> map;\
		if(contains(#ATT))\
		{\
			for (nlohmann::json::iterator it = at(#ATT).begin(); it != at(#ATT).end(); it++)\
			{\
				map.insert(TOTYPE(it)); \
			}\
		}\
		return map;\
	}\
	void ATT(std::map<KEYTYPE,VALUETYPE> map)\
	{\
		(*this)[#ATT] = nlohmann::json::object({}); \
		for (auto& pair : map)\
		{\
			at(#ATT).merge_patch(FROMTYPE(pair)); \
		}\
	}\
	bool ATT##_contains(KEYTYPE k)\
	{\
		if(!contains(#ATT)) return false;\
		std::pair<KEYTYPE, VALUETYPE> pair(k,""); \
		nlohmann::json o = FROMTYPE(pair);\
		nlohmann::json::iterator it = o.begin();\
		return at(#ATT).contains(it.key());\
	}\
	void ATT##_insert(KEYTYPE k,VALUETYPE v)\
	{\
		std::pair<KEYTYPE, VALUETYPE> pair(k,v); \
		if(!contains(#ATT))\
		{\
			(*this)[#ATT] = nlohmann::json::object({}); \
		}\
		at(#ATT).merge_patch(FROMTYPE(pair)); \
	}\
	void ATT##_erase(KEYTYPE k,VALUETYPE v)\
	{\
		std::pair<KEYTYPE, VALUETYPE> pair(k,nullptr); \
		at(#ATT).merge_patch(FROMTYPE(pair)); \
	}

#define JTRACKUUID(CLASS,NAME,LIMIT)

#endif

#if defined(JEXPOSE_ATT_INIT)

#define JCLASS(CLASS)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JTRACKUUID(CLASS,NAME,LIMIT)

#endif

#if defined(JEXPOSE_ATT_ORDER)

#define JCLASS(CLASS) static std::vector<std::pair<std::string,JsonToEditorValueType>> Get##CLASS##Attributes()\
{\
	return std::vector<std::pair<std::string,JsonToEditorValueType>>({
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JTRACKUUID(CLASS,NAME,LIMIT)

#endif

#if defined(JEXPOSE_ATT_FLAGS)

#define JCLASS(CLASS) enum CLASS##_UpdateFlags\
{
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JTRACKUUID(CLASS,NAME,LIMIT)

#endif

#if defined(JEXPOSE_ATT_UPDATE)

#define JCLASS(CLASS) UpdateFlagsMap=\
{
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JTRACKUUID(CLASS,NAME,LIMIT)

#endif

#if defined(JEXPOSE_ATT_DRAWERS_DECL)

#define JCLASS(CLASS) std::map<std::string, JEdvDrawerFunction> Get##CLASS##Drawers();
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) 
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JTRACKUUID(CLASS,NAME,LIMIT)

#endif

#if defined(JEXPOSE_ATT_DRAWERS_DEF)

#define JCLASS(CLASS) std::map<std::string, JEdvDrawerFunction> Get##CLASS##Drawers()\
{\
	return std::map<std::string, JEdvDrawerFunction>({
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, DrawValue<TYPE,JEDVALUETYPE>() },
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, DrawValue<TYPE,JEDVALUETYPE>() },
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, DrawEnum<TYPE,JEDVALUETYPE>(TYPE##ToString,StringTo##TYPE) },
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, DrawVector<TYPE,JEDVALUETYPE>() },
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, DrawVector<TYPE,JEDVALUETYPE>() },
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, nullptr }, //do it when you need it
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, DrawMap<KEYTYPE,VALUETYPE>() },
#define JTRACKUUID(CLASS,NAME,LIMIT)

#endif

#if defined(JEXPOSE_TRACK_UUID_DECL)
#define JCLASS(CLASS)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JTRACKUUID(CLASS,NAME,LIMIT) \
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
#define JCLASS(CLASS)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JTRACKUUID(CLASS,NAME,LIMIT) \
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
		return nostd::GetKeysFromMap(j);\
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
		if(LIMIT>0)\
		assert(NAME.size()<LIMIT);\
		NAME.insert_or_assign(v->uuid(), v);\
	}\
	void Erase##CLASS##From##NAME(std::shared_ptr<CLASS> v)\
	{\
		NAME.erase(v->uuid());\
	}

#endif

#if defined(JEXPOSE_ATT_REQUIRED)

#define JCLASS(CLASS) inline std::vector<std::string> Get##CLASS##RequiredAttributes()\
{\
	std::vector<std::string> requiredAtts;
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) requiredAtts.push_back(#ATT);
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) requiredAtts.push_back(#ATT);
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) requiredAtts.push_back(#ATT);
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) requiredAtts.push_back(#ATT);
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) requiredAtts.push_back(#ATT);
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) requiredAtts.push_back(#ATT);
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) requiredAtts.push_back(#ATT);
#define JTRACKUUID(CLASS,NAME,LIMIT)

#endif

#if defined(JEXPOSE_ATT_CREATOR_DRAWERS_DECL)

#define JCLASS(CLASS) std::map<std::string, JEdvCreatorDrawerFunction> Get##CLASS##CreatorDrawers();
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) 
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE)
#define JTRACKUUID(CLASS,NAME,LIMIT)

#endif

#if defined(JEXPOSE_ATT_CREATOR_DRAWERS_DEF)

#define JCLASS(CLASS) std::map<std::string, JEdvCreatorDrawerFunction> Get##CLASS##CreatorDrawers()\
{\
	std::map<std::string, JEdvCreatorDrawerFunction> creatorDrawers;
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) creatorDrawers.insert_or_assign(#ATT, DrawCreatorValue<TYPE,JEDVALUETYPE>());
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) creatorDrawers.insert_or_assign(#ATT, DrawCreatorValue<TYPE,JEDVALUETYPE>());
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) creatorDrawers.insert_or_assign(#ATT, DrawCreatorEnum<TYPE,JEDVALUETYPE>(TYPE##ToString,StringTo##TYPE));
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) creatorDrawers.insert_or_assign(#ATT, DrawCreatorVector<TYPE,JEDVALUETYPE>());
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) creatorDrawers.insert_or_assign(#ATT, DrawCreatorVector<TYPE,JEDVALUETYPE>());
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) creatorDrawers.insert_or_assign(#ATT, nullptr); //do it when you need it
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) if(REQUIREDTOCREATE) creatorDrawers.insert_or_assign(#ATT, DrawCreatorMap<KEYTYPE,VALUETYPE>());
#define JTRACKUUID(CLASS,NAME,LIMIT)

#endif