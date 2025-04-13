#pragma once

enum ShaderType {
	VERTEX_SHADER,
	PIXEL_SHADER,
	GEOMETRY_SHADER,
	COMPUTE_SHADER,
};

inline static std::map<ShaderType, std::string> ShaderTypeToStr = {
	{ VERTEX_SHADER, "VERTEX_SHADER" },
	{ PIXEL_SHADER, "PIXEL_SHADER" },
	{ GEOMETRY_SHADER, "GEOMETRY_SHADER" },
	{ COMPUTE_SHADER, "GEOMETRY_SHADER" },
};

inline static std::map<std::string, ShaderType> StrToShaderType = {
	{ "VERTEX_SHADER", VERTEX_SHADER },
	{ "PIXEL_SHADER", PIXEL_SHADER },
	{ "GEOMETRY_SHADER", GEOMETRY_SHADER },
	{ "COMPUTE_SHADER", GEOMETRY_SHADER },
};

//shader compilation source (it's shader type, the hlsl path, the uuid and defines)
struct Source {
	ShaderType shaderType;
	std::string shaderUUID;
	std::vector<std::string> defines;

	bool operator<(const Source& other) const
	{
		return std::tie(shaderType, shaderUUID, defines) < std::tie(other.shaderType, other.shaderUUID, other.defines);
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
		return ShaderTypeToStr.at(shaderType) + ":" + shaderUUID + " (" + defs + ")";
	}
};

template <>
struct std::hash<Source>
{
	std::size_t operator()(const Source& src) const
	{
		using std::hash;
		std::string s = "";
		s += ShaderTypeToStr.at(src.shaderType);
		s += src.shaderUUID;
		for (auto& v : src.defines) { s += v; }
		return hash<std::string>()(s);
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

template <>
struct std::hash<ShaderConstantsBufferParametersMap>
{
	std::size_t operator()(const ShaderConstantsBufferParametersMap& m) const
	{
		std::string s;
		for (auto& [k, v] : m)
		{
			s += k;
			s += std::to_string(v.registerId);
			s += std::to_string(v.numConstantsBuffers);
		}
		return hash<std::string>()(s);
	}
};

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

inline static std::string BaseTextureFromGammaSpaceDefine = "_BASE_TEXTURE_FROM_GAMMA_SPACE";
inline static std::set<DXGI_FORMAT> FormatsInGammaSpace =
{
	DXGI_FORMAT_R16G16B16A16_UNORM,
	DXGI_FORMAT_R10G10B10A2_UNORM,
	DXGI_FORMAT_R8G8B8A8_UNORM,
	DXGI_FORMAT_R16G16_UNORM,
	DXGI_FORMAT_R8G8_UNORM,
	DXGI_FORMAT_D16_UNORM,
	DXGI_FORMAT_R16_UNORM,
	DXGI_FORMAT_R8_UNORM,
	DXGI_FORMAT_A8_UNORM,
	DXGI_FORMAT_R1_UNORM,
	DXGI_FORMAT_R8G8_B8G8_UNORM,
	DXGI_FORMAT_G8R8_G8B8_UNORM,
	DXGI_FORMAT_BC1_UNORM,
	DXGI_FORMAT_BC2_UNORM,
	DXGI_FORMAT_BC3_UNORM,
	DXGI_FORMAT_BC4_UNORM,
	DXGI_FORMAT_BC5_UNORM,
	DXGI_FORMAT_B5G6R5_UNORM,
	DXGI_FORMAT_B5G5R5A1_UNORM,
	DXGI_FORMAT_B8G8R8A8_UNORM,
	DXGI_FORMAT_B8G8R8X8_UNORM,
	DXGI_FORMAT_BC7_UNORM,
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

template <>
struct std::hash<ShaderTextureParametersMap>
{
	std::size_t operator()(const ShaderTextureParametersMap& m) const
	{
		std::string s;
		for (auto& [k, v] : m)
		{
			s += k;
			s += std::to_string(v.registerId);
			s += std::to_string(v.numTextures);
		}
		return hash<std::string>()(s);
	}
};

//define the Samplers binding
struct ShaderSamplerParameter {
	unsigned int registerId;
	unsigned int numSamplers;
};
typedef std::map<std::string, ShaderSamplerParameter> ShaderSamplerParametersMap;
typedef std::pair<std::string, ShaderSamplerParameter> ShaderSamplerParametersPair;

template <>
struct std::hash<ShaderSamplerParametersMap>
{
	std::size_t operator()(const ShaderSamplerParametersMap& m) const
	{
		std::string s;
		for (auto& [k, v] : m)
		{
			s += k;
			s += std::to_string(v.registerId);
			s += std::to_string(v.numSamplers);
		}
		return hash<std::string>()(s);
	}
};

//define the layout of the cbuffer definition
struct ShaderConstantsBufferVariable {
	size_t bufferIndex;
	size_t size;
	size_t offset;
};
typedef std::map<std::string, ShaderConstantsBufferVariable> ShaderConstantsBufferVariablesMap;
typedef std::pair<std::string, ShaderConstantsBufferVariable> ShaderConstantsBufferVariablesPair;

//Collection of header(.h) files
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