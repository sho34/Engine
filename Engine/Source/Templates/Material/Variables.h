#pragma once

#include "../../Resources/ShaderByteCode.h"

enum MaterialVariablesTypes {
	MAT_VAR_BOOLEAN,
	MAT_VAR_INTEGER,
	MAT_VAR_UNSIGNED_INTEGER,
	MAT_VAR_RGB,
	MAT_VAR_RGBA,
	MAT_VAR_FLOAT,
	MAT_VAR_FLOAT2,
	MAT_VAR_FLOAT3,
	MAT_VAR_FLOAT4,
	MAT_VAR_MATRIX4X4
};

static std::vector<std::string> MaterialVariablesTypesStr = {
	"BOOLEAN",
	"INTEGER",
	"UNSIGNED_INTEGER",
	"RGB",
	"RGBA",
	"FLOAT",
	"FLOAT2",
	"FLOAT3",
	"FLOAT4",
	"MATRIX4X4"
};

static std::map<MaterialVariablesTypes, std::string> MaterialVariablesTypesNames = {
	{ MAT_VAR_BOOLEAN, "BOOLEAN"},
	{ MAT_VAR_INTEGER, "INTEGER"},
	{ MAT_VAR_UNSIGNED_INTEGER, "UNSIGNED_INTEGER"},
	{ MAT_VAR_RGB, "RGB"},
	{ MAT_VAR_RGBA, "RGBA"},
	{ MAT_VAR_FLOAT, "FLOAT"},
	{ MAT_VAR_FLOAT2, "FLOAT2"},
	{ MAT_VAR_FLOAT3, "FLOAT3"},
	{ MAT_VAR_FLOAT4, "FLOAT4"},
	{ MAT_VAR_MATRIX4X4, "MATRIX4X4"},
};

static std::map<std::string, MaterialVariablesTypes> StrToMaterialVariablesTypes = {
	{ "BOOLEAN", MAT_VAR_BOOLEAN },
	{ "INTEGER", MAT_VAR_INTEGER },
	{ "UNSIGNED_INTEGER", MAT_VAR_UNSIGNED_INTEGER },
	{ "RGB", MAT_VAR_RGB },
	{ "RGBA", MAT_VAR_RGBA },
	{ "FLOAT", MAT_VAR_FLOAT },
	{ "FLOAT2", MAT_VAR_FLOAT2 },
	{ "FLOAT3", MAT_VAR_FLOAT3 },
	{ "FLOAT4", MAT_VAR_FLOAT4 },
	{ "MATRIX4X4", MAT_VAR_MATRIX4X4 }
};

static std::map<MaterialVariablesTypes, size_t> MaterialVariablesTypesSizes = {
	{ MAT_VAR_BOOLEAN, sizeof(bool)},
	{ MAT_VAR_INTEGER, sizeof(int)},
	{ MAT_VAR_UNSIGNED_INTEGER, sizeof(unsigned int)},
	{ MAT_VAR_RGB, sizeof(float[3])},
	{ MAT_VAR_RGBA, sizeof(float[4])},
	{ MAT_VAR_FLOAT, sizeof(float)},
	{ MAT_VAR_FLOAT2, sizeof(float[2])},
	{ MAT_VAR_FLOAT3, sizeof(float[3])},
	{ MAT_VAR_FLOAT4, sizeof(float[4])},
	{ MAT_VAR_MATRIX4X4, sizeof(float[16])},
};

static std::map<std::string, MaterialVariablesTypes> HLSLVariableClassToMaterialVariableTypes = {
	{	"bool", MAT_VAR_BOOLEAN },
	{	"int", MAT_VAR_INTEGER },
	{	"uint", MAT_VAR_UNSIGNED_INTEGER },
	{	"unsigned int", MAT_VAR_UNSIGNED_INTEGER },
	{	"dword", MAT_VAR_UNSIGNED_INTEGER },
	{	"float", MAT_VAR_FLOAT },
	{	"float2", MAT_VAR_FLOAT2 },
	{	"float3", MAT_VAR_FLOAT3 },
	{	"float4", MAT_VAR_FLOAT4 },
	{	"float4x4", MAT_VAR_MATRIX4X4 },
	{	"matrix", MAT_VAR_MATRIX4X4 },
};

static std::map<std::tuple<std::string, std::string>, MaterialVariablesTypes> HLSLVariablePatternToMaterialVariableTypes = {
	{ { "float3","color" }, MAT_VAR_RGB },
	{ { "float4","color" }, MAT_VAR_RGBA },
};

struct MaterialVariableInitialValue
{
	MaterialVariablesTypes variableType;
	std::any value;
};
typedef std::pair<std::string, MaterialVariableInitialValue> MaterialInitialValuePair;
typedef std::map<std::string, MaterialVariableInitialValue> MaterialInitialValueMap;

template<MaterialVariablesTypes T, typename V>
MaterialVariableInitialValue TransformJsonToMaterialVariableInitialValue(nlohmann::json json) {
	V value = json.at("value");
	return MaterialVariableInitialValue({ .variableType = T, .value = value });
}

struct MaterialVariableMapping
{
	MaterialVariablesTypes variableType;
	ShaderConstantsBufferVariable mapping;
};
typedef std::map<std::string, MaterialVariableMapping> MaterialVariablesMapping;
typedef std::pair<std::string, MaterialVariableMapping> MaterialVariablesPair;

MaterialVariableInitialValue JsonToMaterialInitialValue(nlohmann::json json);

template<typename T>
void WriteMappedValueToDestination(MaterialVariableInitialValue& def, void* dst, size_t size)
{
	auto src = std::any_cast<T>(def.value);
	memcpy(dst, &src, size);
}

void WriteMappedInitialValuesToDestination(MaterialVariableInitialValue& def, void* dst, size_t size);

template<typename T>
void WriteMaterialVariableInitialValueToJson(nlohmann::json& matInitialValue, MaterialVariableInitialValue& varValue)
{
	auto value = std::any_cast<T>(varValue.value);
	matInitialValue["value"] = value;
}

nlohmann::json TransformMaterialValueMappingToJson(MaterialInitialValueMap mappedValues);

void TransformJsonToMaterialValueMapping(MaterialInitialValueMap& map, nlohmann::json object, std::string key);

