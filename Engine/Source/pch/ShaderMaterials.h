#pragma once


enum ShaderType {
	VERTEX_SHADER,
	PIXEL_SHADER,
	GEOMETRY_SHADER
};

inline static std::map<ShaderType, std::string> ShaderTypeToStr = {
	{ VERTEX_SHADER, "VERTEX_SHADER" },
	{ PIXEL_SHADER, "PIXEL_SHADER" },
	{ GEOMETRY_SHADER, "GEOMETRY_SHADER" },
};

//shader compilation source (shaderName + it's shader type)
struct Source {
	ShaderType shaderType;
	std::string shaderName;
	std::vector<std::string> defines;

	bool operator<(const Source& other) const
	{
		return std::tie(shaderName, shaderType, defines) < std::tie(other.shaderName, other.shaderType, other.defines);
	}

	std::string to_string()
	{
		std::string defs;
		for (unsigned int i = 0; i < defines.size(); i++)
		{
			defs += defines[i];
			if (i < (defines.size() - 1))
				defs += ",";
		}
		return ShaderTypeToStr.at(shaderType) + ":" + shaderName + ".hlsl (" + defs + ")";
	}
};

//define the byte code
typedef std::vector<unsigned char> ShaderByteCode;

//define the Constants Buffer binding
struct ShaderConstantsBufferParameter {
	unsigned int registerId;
	unsigned int numConstantsBuffers;
};
typedef std::map<std::string, ShaderConstantsBufferParameter> ShaderConstantsBufferParametersMap;
typedef std::pair<std::string, ShaderConstantsBufferParameter> ShaderConstantsBufferParametersPair;

//Texture Type
enum TextureType
{
	TextureType_Base,
	TextureType_NormalMap,
	TextureType_MetallicRoughness,
	TextureType_ShadowMaps,
	TextureType_MinTexture,
	TextureType_MaxTexture,
	TextureType_DepthTexture,
};

inline static std::map<TextureType, std::string> textureTypeToStr = {
	{ TextureType_Base, "BaseTexture" },
	{ TextureType_NormalMap, "NormalMapTexture" },
	{ TextureType_MetallicRoughness, "MetallicRoughnessTexture" },
	{ TextureType_ShadowMaps, "ShadowMapsTextures" },
	{ TextureType_MinTexture, "MinTexture" },
	{ TextureType_MaxTexture, "MaxTexture" },
	{ TextureType_DepthTexture, "DepthTexture" },
};

inline static std::map<std::string, TextureType> strToTextureType = {
	{ "BaseTexture", TextureType_Base },
	{ "NormalMapTexture", TextureType_NormalMap },
	{ "MetallicRoughnessTexture", TextureType_MetallicRoughness },
	{ "ShadowMapsTextures", TextureType_ShadowMaps },
	{ "MinTexture", TextureType_MinTexture },
	{ "MaxTexture", TextureType_MaxTexture },
	{ "DepthTexture", TextureType_DepthTexture },
};

inline static std::map<TextureType, std::string> textureTypeToShaderDefine = {
	{ TextureType_Base, "_HAS_BASE_TEXTURE" },
	{ TextureType_NormalMap, "_HAS_NORMALMAP_TEXTURE" },
	{ TextureType_MetallicRoughness, "_HAS_METALLIC_ROUGHNESS_TEXTURE" },
	{ TextureType_ShadowMaps, "_HAS_SHADOWMAPS_TEXTURES" },
	{ TextureType_MinTexture, "_HAS_MIN_TEXTURE" },
	{ TextureType_MaxTexture, "_HAS_MAX_TEXTURE" },
	{ TextureType_DepthTexture, "_HAS_DEPTH_TEXTURE" },
};

inline static std::set<TextureType> materialTexturesTypes = {
	TextureType_Base,
	TextureType_NormalMap,
	TextureType_MetallicRoughness
};

//define the Textures binding
struct ShaderTextureParameter {
	unsigned int registerId;
	unsigned int numTextures;
};
typedef std::map<TextureType, ShaderTextureParameter> ShaderTextureParametersMap;
typedef std::pair<TextureType, ShaderTextureParameter> ShaderTextureParametersPair;

//define the Samplers binding
struct ShaderSamplerParameter {
	unsigned int registerId;
	unsigned int numSamplers;
};
typedef std::map<std::string, ShaderSamplerParameter> ShaderSamplerParametersMap;
typedef std::pair<std::string, ShaderSamplerParameter> ShaderSamplerParametersPair;

//define the layout of the cbuffer definition
struct ShaderConstantsBufferVariable {
	size_t bufferIndex;
	size_t size;
	size_t offset;
};
typedef std::map<std::string, ShaderConstantsBufferVariable> ShaderConstantsBufferVariablesMap;
typedef std::pair<std::string, ShaderConstantsBufferVariable> ShaderConstantsBufferVariablesPair;

//Dependencies of hlsl+shaderType(Source) file to many .h files
typedef std::unordered_set<std::string> ShaderDependencies;

//Dependencies of hlsl+shaderType(Source) file to many .h files
typedef std::map<Source, ShaderDependencies> ShaderIncludesDependencies;

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

static std::map<MaterialVariablesTypes, std::function<void(nlohmann::json&, std::string)>> MaterialVariablesMappedJsonInitializer = {
	{ MAT_VAR_BOOLEAN, [](nlohmann::json& mappedValues, std::string variable)
	{
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesNames.at(MAT_VAR_BOOLEAN)},
			{"value",false}
		});
	}},
	{ MAT_VAR_INTEGER, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesNames.at(MAT_VAR_INTEGER)},
			{"value",0}
		});
	}},
	{ MAT_VAR_UNSIGNED_INTEGER, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesNames.at(MAT_VAR_UNSIGNED_INTEGER)},
			{"value",0U}
		});
	}},
	{ MAT_VAR_RGB, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesNames.at(MAT_VAR_RGB)},
			{"value", {1.0f, 1.0f, 1.0f } }
		});
	}},
	{ MAT_VAR_RGBA, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesNames.at(MAT_VAR_RGBA)},
			{"value", {1.0f, 1.0f, 1.0f, 1.0f } }
		});
	}},
	{ MAT_VAR_FLOAT, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesNames.at(MAT_VAR_FLOAT)},
			{"value", 0.0f }
		});
	}},
	{ MAT_VAR_FLOAT2, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesNames.at(MAT_VAR_FLOAT2)},
			{"value", { 0.0f, 0.0f } }
		});
	}},
	{ MAT_VAR_FLOAT3, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesNames.at(MAT_VAR_FLOAT3)},
			{"value", { 0.0f, 0.0f, 0.0f }}
		});
	}},
	{ MAT_VAR_FLOAT4, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesNames.at(MAT_VAR_FLOAT4)},
			{"value", { 0.0f, 0.0f, 0.0f, 0.0f } }
		});
	}},
	{ MAT_VAR_MATRIX4X4, [](nlohmann::json& mappedValues, std::string variable) {
		mappedValues.push_back({
			{"variable",variable},
			{"variableType",MaterialVariablesTypesNames.at(MAT_VAR_MATRIX4X4)},
			{"value", {
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f,
			}}
		});
	}},
};

struct MaterialVariableInitialValue
{
	MaterialVariablesTypes variableType;
	std::any value;
};
typedef std::pair<std::string, MaterialVariableInitialValue> MaterialInitialValuePair;
typedef std::map<std::string, MaterialVariableInitialValue> MaterialInitialValueMap;

struct MaterialVariableMapping
{
	MaterialVariablesTypes variableType;
	ShaderConstantsBufferVariable mapping;
};
typedef std::map<std::string, MaterialVariableMapping> MaterialVariablesMapping;
typedef std::pair<std::string, MaterialVariableMapping> MaterialVariablesPair;