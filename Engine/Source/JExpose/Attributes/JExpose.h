#if defined(JEXPOSE_ATT_DECL)

#define JCLASS(CLASS)

#define JTYPE(TYPE,VALUE) virtual TYPE JType() { return VALUE; }

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

#define JTRACKUUID(CLASS,NAME,LIMIT,COND)

#endif

#if defined(JEXPOSE_ATT_INIT)

#define JCLASS(CLASS)
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) create_##ATT(INITIAL);
#define JTRACKUUID(CLASS,NAME,LIMIT,COND)

#endif

#if defined(JEXPOSE_ATT_ORDER)

#define JCLASS(CLASS) static std::vector<std::pair<std::string,JsonToEditorValueType>> Get##CLASS##Attributes()\
{\
	return std::vector<std::pair<std::string,JsonToEditorValueType>>({
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) std::make_pair(#ATT,JEDVALUETYPE),
#define JTRACKUUID(CLASS,NAME,LIMIT,COND)

#endif

#if defined(JEXPOSE_ATT_FLAGS)

#define JCLASS(CLASS) enum CLASS##_UpdateFlags\
{
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) Update_##ATT,
#define JTRACKUUID(CLASS,NAME,LIMIT,COND)

#endif

#if defined(JEXPOSE_ATT_UPDATE)

#define JCLASS(CLASS) UpdateFlagsMap=\
{
#define JTYPE(TYPE,VALUE)
#define JEXPOSE(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_ENUM(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_VECTOR(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_VECTOR_TRANSFORM(TYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_SET(TYPE,ATT,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JEXPOSE_MAP_TRANSFORM(KEYTYPE,VALUETYPE,ATT,TOTYPE,FROMTYPE,INITIAL,JEDVALUETYPE,UPDATEMASK,REQUIREDTOCREATE) { #ATT, std::make_tuple(1 << Update_##ATT, !!UPDATEMASK) },
#define JTRACKUUID(CLASS,NAME,LIMIT,COND)

#endif