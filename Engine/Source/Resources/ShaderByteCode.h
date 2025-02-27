#pragma once

#include <map>
#include <vector>
#include <memory>
#include <d3d12shader.h>
#include <wrl.h>
#include <wrl/client.h>
#include <dxcapi.h>

enum ShaderType {
	VERTEX_SHADER,
	PIXEL_SHADER,
	GEOMETRY_SHADER
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

struct ShaderBinary {
	Source shaderSource;

	//vertex shader semantics(POSITION,TEXCOORD0, etc)
	std::vector<std::string> vsSemantics;

	//constants buffer parameters & variables
	ShaderConstantsBufferParametersMap constantsBuffersParameters;
	ShaderConstantsBufferVariablesMap constantsBuffersVariables;

	//texture & samplers
	ShaderTextureParametersMap texturesParameters;
	ShaderSamplerParametersMap samplersParameters;

	std::vector<size_t> cbufferSize;

	//Specific registers slots (c1,c2,c3,...) 
	int cameraCBVRegister = -1;
	int lightCBVRegister = -1;
	int animationCBVRegister = -1;
	int lightsShadowMapCBVRegister = -1;
	int lightsShadowMapSRVRegister = -1;

	//the bytecode(vector of bytes)
	ShaderByteCode byteCode;

	void CopyFrom(std::shared_ptr<ShaderBinary>& src);

	void CreateVSSemantics(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc);

	void CreateResourcesBinding(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc);

	void CreateConstantsBuffersVariables(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc);

	void CreateByteCode(const ComPtr<IDxcResult>& result);
};
