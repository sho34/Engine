#include "pch.h"
#include "Variables.h"

template<>
MaterialVariableInitialValue TransformJsonToMaterialVariableInitialValue<MAT_VAR_RGB, XMFLOAT3>(nlohmann::json json)
{
	XMFLOAT3 value = XMFLOAT3({ json["value"][0] , json["value"][1], json["value"][2] });
	return MaterialVariableInitialValue({ .variableType = MAT_VAR_RGB, .value = value });
}

template<>
MaterialVariableInitialValue TransformJsonToMaterialVariableInitialValue<MAT_VAR_RGBA, XMFLOAT4>(nlohmann::json json)
{
	XMFLOAT4 value = XMFLOAT4({ json["value"][0] , json["value"][1], json["value"][2], json["value"][3] });
	return MaterialVariableInitialValue({ .variableType = MAT_VAR_RGBA, .value = value });
}

template<>
MaterialVariableInitialValue TransformJsonToMaterialVariableInitialValue<MAT_VAR_FLOAT2, XMFLOAT2>(nlohmann::json json)
{
	XMFLOAT2 value = XMFLOAT2({ json["value"][0] , json["value"][1] });
	return MaterialVariableInitialValue({ .variableType = MAT_VAR_FLOAT2, .value = value });
}

template<>
MaterialVariableInitialValue TransformJsonToMaterialVariableInitialValue<MAT_VAR_FLOAT3, XMFLOAT3>(nlohmann::json json)
{
	XMFLOAT3 value = XMFLOAT3({ json["value"][0] , json["value"][1], json["value"][2] });
	return MaterialVariableInitialValue({ .variableType = MAT_VAR_FLOAT3, .value = value });
}

template<>
MaterialVariableInitialValue TransformJsonToMaterialVariableInitialValue<MAT_VAR_FLOAT4, XMFLOAT4>(nlohmann::json json)
{
	XMFLOAT4 value = XMFLOAT4({ json["value"][0] , json["value"][1], json["value"][2], json["value"][3] });
	return MaterialVariableInitialValue({ .variableType = MAT_VAR_FLOAT4, .value = value });
}

template<>
MaterialVariableInitialValue TransformJsonToMaterialVariableInitialValue<MAT_VAR_MATRIX4X4, XMMATRIX>(nlohmann::json json)
{
	XMMATRIX value = XMMATRIX(
		json["value"][0], json["value"][1], json["value"][2], json["value"][3],
		json["value"][4], json["value"][5], json["value"][6], json["value"][7],
		json["value"][8], json["value"][9], json["value"][10], json["value"][11],
		json["value"][12], json["value"][13], json["value"][14], json["value"][15]
	);
	return MaterialVariableInitialValue({ .variableType = MAT_VAR_MATRIX4X4, .value = value });
}

static std::map<std::string, std::function<MaterialVariableInitialValue(nlohmann::json)>> JsonToMaterialVariableInitialValue =
{
	{ "BOOLEAN", TransformJsonToMaterialVariableInitialValue<MAT_VAR_BOOLEAN,BOOLEAN> },
	{ "INTEGER", TransformJsonToMaterialVariableInitialValue<MAT_VAR_INTEGER,INT> },
	{ "UNSIGNED_INTEGER", TransformJsonToMaterialVariableInitialValue<MAT_VAR_UNSIGNED_INTEGER,UINT> },
	{ "RGB", TransformJsonToMaterialVariableInitialValue<MAT_VAR_RGB,XMFLOAT3> },
	{ "RGBA", TransformJsonToMaterialVariableInitialValue<MAT_VAR_RGBA,XMFLOAT4> },
	{ "FLOAT", TransformJsonToMaterialVariableInitialValue<MAT_VAR_FLOAT,FLOAT> },
	{ "FLOAT2", TransformJsonToMaterialVariableInitialValue<MAT_VAR_FLOAT2,XMFLOAT2> },
	{ "FLOAT3", TransformJsonToMaterialVariableInitialValue<MAT_VAR_FLOAT3,XMFLOAT3> },
	{ "FLOAT4", TransformJsonToMaterialVariableInitialValue<MAT_VAR_FLOAT4,XMFLOAT4> },
	{ "MATRIX4X4", TransformJsonToMaterialVariableInitialValue<MAT_VAR_MATRIX4X4,XMMATRIX> }
};

MaterialVariableInitialValue JsonToMaterialInitialValue(nlohmann::json json)
{
	return JsonToMaterialVariableInitialValue.at(json.at("variableType"))(json);
}

static std::map<MaterialVariablesTypes, std::function<void(MaterialVariableInitialValue& def, void* dst, size_t size)>> materialVariableInitialValueToDestionation =
{
	{ MAT_VAR_BOOLEAN, WriteMappedValueToDestination<BOOLEAN>	},
	{ MAT_VAR_INTEGER, WriteMappedValueToDestination<INT> },
	{ MAT_VAR_UNSIGNED_INTEGER,	WriteMappedValueToDestination<UINT> },
	{ MAT_VAR_RGB, WriteMappedValueToDestination<XMFLOAT3> },
	{ MAT_VAR_RGBA,	WriteMappedValueToDestination<XMFLOAT4> },
	{ MAT_VAR_FLOAT, WriteMappedValueToDestination<FLOAT> },
	{ MAT_VAR_FLOAT2,	WriteMappedValueToDestination<XMFLOAT2> },
	{ MAT_VAR_FLOAT3,	WriteMappedValueToDestination<XMFLOAT3> },
	{ MAT_VAR_FLOAT4,	WriteMappedValueToDestination<XMFLOAT4> },
	{ MAT_VAR_MATRIX4X4, WriteMappedValueToDestination<XMMATRIX> },
};

void WriteMappedInitialValuesToDestination(MaterialVariableInitialValue& def, void* dst, size_t size)
{
	materialVariableInitialValueToDestionation.at(def.variableType)(def, dst, size);
}

template<>
void WriteMaterialVariableInitialValueToJson<XMFLOAT2>(nlohmann::json& matInitialValue, MaterialVariableInitialValue& varValue)
{
	XMFLOAT2 value = std::any_cast<XMFLOAT2>(varValue.value);
	matInitialValue["value"] = { value.x, value.y };
}

template<>
void WriteMaterialVariableInitialValueToJson<XMFLOAT3>(nlohmann::json& matInitialValue, MaterialVariableInitialValue& varValue)
{
	XMFLOAT3 value = std::any_cast<XMFLOAT3>(varValue.value);
	matInitialValue["value"] = { value.x, value.y, value.z };
}

template<>
void WriteMaterialVariableInitialValueToJson<XMFLOAT4>(nlohmann::json& matInitialValue, MaterialVariableInitialValue& varValue)
{
	XMFLOAT4 value = std::any_cast<XMFLOAT4>(varValue.value);
	matInitialValue["value"] = { value.x, value.y, value.z };
}

template<>
void WriteMaterialVariableInitialValueToJson<XMMATRIX>(nlohmann::json& matInitialValue, MaterialVariableInitialValue& varValue)
{
	XMMATRIX value = std::any_cast<XMMATRIX>(varValue.value);
	matInitialValue["value"] = {
		value.r[0].m128_f32[0], value.r[0].m128_f32[1], value.r[0].m128_f32[2], value.r[0].m128_f32[3],
		value.r[1].m128_f32[0], value.r[1].m128_f32[1], value.r[1].m128_f32[2], value.r[1].m128_f32[3],
		value.r[2].m128_f32[0], value.r[2].m128_f32[1], value.r[2].m128_f32[2], value.r[2].m128_f32[3],
		value.r[3].m128_f32[0], value.r[3].m128_f32[1], value.r[3].m128_f32[2], value.r[3].m128_f32[3],
	};
}

void valueMappingToJson(MaterialVariablesTypes type, nlohmann::json& matInitialValue, MaterialVariableInitialValue& varValue)
{
	const std::map<MaterialVariablesTypes, std::function<void(nlohmann::json& matInitialValue, MaterialVariableInitialValue& varValue)>> vMappingToJson = {
		{ MAT_VAR_BOOLEAN, WriteMaterialVariableInitialValueToJson<BOOLEAN>},
		{ MAT_VAR_INTEGER, WriteMaterialVariableInitialValueToJson<INT>},
		{ MAT_VAR_UNSIGNED_INTEGER, WriteMaterialVariableInitialValueToJson<UINT>},
		{ MAT_VAR_RGB, WriteMaterialVariableInitialValueToJson<XMFLOAT3>},
		{ MAT_VAR_RGBA, WriteMaterialVariableInitialValueToJson<XMFLOAT4>},
		{ MAT_VAR_FLOAT, WriteMaterialVariableInitialValueToJson<FLOAT>},
		{ MAT_VAR_FLOAT2, WriteMaterialVariableInitialValueToJson<XMFLOAT2>},
		{ MAT_VAR_FLOAT3, WriteMaterialVariableInitialValueToJson<XMFLOAT3>},
		{ MAT_VAR_FLOAT4, WriteMaterialVariableInitialValueToJson<XMFLOAT4>},
		{ MAT_VAR_MATRIX4X4, WriteMaterialVariableInitialValueToJson<XMMATRIX>},
	};
	return vMappingToJson.at(type)(matInitialValue, varValue);
}

nlohmann::json TransformMaterialValueMappingToJson(MaterialInitialValueMap mappedValues) {

	//using namespace DirectX;
	nlohmann::json jMappedValues = nlohmann::json::array();

	std::transform(mappedValues.begin(), mappedValues.end(), std::back_inserter(jMappedValues), [](MaterialInitialValuePair pair)
		{
			nlohmann::json matInitialValue = nlohmann::json({});

			matInitialValue["variable"] = pair.first;
			matInitialValue["variableType"] = MaterialVariablesTypesToString.at(pair.second.variableType);
			valueMappingToJson(pair.second.variableType, matInitialValue, pair.second);
			return matInitialValue;
		}
	);

	return jMappedValues;
}

void TransformJsonToMaterialValueMapping(MaterialInitialValueMap& map, nlohmann::json object, std::string key) {

	using namespace DirectX;

	if (!object.contains(key)) return;

	nlohmann::json mappedValues = object[key];

	std::transform(mappedValues.begin(), mappedValues.end(), std::inserter(map, map.end()), [](nlohmann::json value)
		{
			MaterialInitialValuePair pair;
			pair.first = value["variable"];
			pair.second = JsonToMaterialInitialValue(value);
			return pair;
		}
	);
}


