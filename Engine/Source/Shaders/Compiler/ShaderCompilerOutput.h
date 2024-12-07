#pragma once

enum ShaderType {
	VERTEX_SHADER,
	PIXEL_SHADER,
	GEOMETRY_SHADER
};

//define the byte code
typedef std::vector<byte> ShaderByteCode;
typedef std::shared_ptr<ShaderByteCode> ShaderByteCodePtr;

//define the Constants Buffer binding
struct CBufferParameter {
	UINT registerId;
	UINT numCBuffers;
};
typedef std::map<std::wstring, CBufferParameter> CBufferParametersDefinition;
typedef std::shared_ptr<CBufferParametersDefinition> CBufferParametersDefinitionPtr;

//define the Textures binding
struct TextureParameter {
	UINT registerId;
	UINT numTextures;
};
typedef std::map<std::wstring, TextureParameter> TextureParametersDefinition;
typedef std::shared_ptr<TextureParametersDefinition> TextureParametersDefinitionPtr;

//define the Samplers binding
struct SamplerParameter {
	UINT registerId;
	UINT numSamplers;
};
typedef std::map<std::wstring, SamplerParameter> SamplerParametersDefinition;
typedef std::shared_ptr<SamplerParametersDefinition> SamplerParametersDefinitionPtr;

//define the layout of the cbuffer definition
struct CBufferVariable {
	size_t bufferIndex;
	size_t size;
	size_t offset;
};
typedef std::map<std::wstring,CBufferVariable> CBufferVariablesDefinition;
typedef std::shared_ptr<CBufferVariablesDefinition> CBufferVariablesDefinitionPtr;

struct ShaderCompilerOutput {
	ShaderByteCodePtr byteCode;
	CBufferParametersDefinitionPtr cbufferParametersDef;
	TextureParametersDefinitionPtr texturesParametersDef;
	SamplerParametersDefinitionPtr samplersParametersDef;
	CBufferVariablesDefinitionPtr cbufferVariablesDef;
	std::vector<size_t> cbufferSize;
	ShaderType shaderType;
	INT cameraCBVRegister = -1;
	INT lightCBVRegister = -1;
	INT animationCBVRegister = -1;
	INT lightsShadowMapCBVRegister = -1;
	INT lightsShadowMapSRVRegister = -1;

	void CopyFrom(std::shared_ptr<ShaderCompilerOutput>& src) {
		byteCode = std::make_shared<ShaderByteCode>(src->byteCode->size());
		memcpy_s(&(*byteCode.get())[0], src->byteCode->size(), &(*src->byteCode.get())[0], src->byteCode->size());
		cbufferParametersDef = src->cbufferParametersDef;
		texturesParametersDef = src->texturesParametersDef;
		samplersParametersDef = src->samplersParametersDef;
		cbufferVariablesDef = src->cbufferVariablesDef;
		cbufferSize = src->cbufferSize;
		shaderType = src->shaderType;
		cameraCBVRegister = src->cameraCBVRegister;
		lightCBVRegister = src->lightCBVRegister;
		animationCBVRegister = src->animationCBVRegister;
		lightsShadowMapCBVRegister = src->lightsShadowMapCBVRegister;
		lightsShadowMapSRVRegister = src->lightsShadowMapSRVRegister;
	}
};
typedef std::shared_ptr<ShaderCompilerOutput> ShaderCompilerOutputPtr;