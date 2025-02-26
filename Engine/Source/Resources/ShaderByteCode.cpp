#include "pch.h"
#include "ShaderByteCode.h"
#include "../Scene/Scene.h"

void ShaderBinary::CopyFrom(std::shared_ptr<ShaderBinary>& src) {
	shaderSource = src->shaderSource;
	vsSemantics = src->vsSemantics;
	constantsBuffersParameters = src->constantsBuffersParameters;
	constantsBuffersVariables = src->constantsBuffersVariables;
	texturesParameters = src->texturesParameters;
	samplersParameters = src->samplersParameters;
	cbufferSize = src->cbufferSize;
	cameraCBVRegister = src->cameraCBVRegister;
	lightCBVRegister = src->lightCBVRegister;
	animationCBVRegister = src->animationCBVRegister;
	lightsShadowMapCBVRegister = src->lightsShadowMapCBVRegister;
	lightsShadowMapSRVRegister = src->lightsShadowMapSRVRegister;
	byteCode = src->byteCode;
}

void ShaderBinary::CreateVSSemantics(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc)
{
	if (shaderSource.shaderType != VERTEX_SHADER) return;
	for (unsigned int paramIdx = 0; paramIdx < desc.InputParameters; paramIdx++)
	{
		D3D12_SIGNATURE_PARAMETER_DESC signatureDesc{};
		reflection->GetInputParameterDesc(paramIdx, &signatureDesc);
		vsSemantics.push_back(signatureDesc.SemanticName);
	}
}

void ShaderBinary::CreateResourcesBinding(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc)
{
	using namespace Animation;
	using namespace Scene;

	for (unsigned int paramIdx = 0; paramIdx < desc.BoundResources; paramIdx++)
	{
		D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
		reflection->GetResourceBindingDesc(paramIdx, &bindDesc);

		std::string resourceName(bindDesc.Name);

		cameraCBVRegister = (resourceName == CameraConstantBufferName) ? bindDesc.BindPoint : cameraCBVRegister;
		lightCBVRegister = (resourceName == LightConstantBufferName) ? bindDesc.BindPoint : lightCBVRegister;
		animationCBVRegister = (resourceName == AnimationConstantBufferName) ? bindDesc.BindPoint : animationCBVRegister;
		lightsShadowMapCBVRegister = (resourceName == ShadowMapConstantBufferName) ? bindDesc.BindPoint : lightsShadowMapCBVRegister;
		lightsShadowMapSRVRegister = (resourceName == ShadowMapLightsShaderResourveViewName) ? bindDesc.BindPoint : lightsShadowMapSRVRegister;

		if (bindDesc.Type == D3D_SIT_CBUFFER)
		{
			constantsBuffersParameters.insert_or_assign(resourceName, ShaderConstantsBufferParameter({ .registerId = bindDesc.BindPoint, .numConstantsBuffers = bindDesc.BindCount }));
		}
		else if (bindDesc.Type == D3D_SIT_TEXTURE)
		{
			texturesParameters.insert_or_assign(resourceName, ShaderTextureParameter({ .registerId = bindDesc.BindPoint, .numTextures = bindDesc.BindCount > 0 ? bindDesc.BindCount : bindDesc.NumSamples })); //if N > 0 -> N else -1
		}
		else if (bindDesc.Type == D3D_SIT_SAMPLER)
		{
			samplersParameters.insert_or_assign(resourceName, ShaderSamplerParameter({ .registerId = bindDesc.BindPoint, .numSamplers = bindDesc.BindCount }));
		}
	}
}

void ShaderBinary::CreateConstantsBuffersVariables(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc)
{
	using namespace Animation;
	using namespace Scene;

	for (unsigned int paramIdx = 0; paramIdx < desc.ConstantBuffers; paramIdx++) {
		auto cbReflection = reflection->GetConstantBufferByIndex(paramIdx);
		D3D12_SHADER_BUFFER_DESC paramDesc{};
		cbReflection->GetDesc(&paramDesc);

		std::string cbufferName(paramDesc.Name);
		bool isCameraParam = cbufferName == CameraConstantBufferName;
		bool isLightParam = cbufferName == LightConstantBufferName;
		bool isAnimationParam = cbufferName == AnimationConstantBufferName;
		bool isShadowMapParam = cbufferName == ShadowMapConstantBufferName;
		if (isCameraParam || isLightParam || isAnimationParam || isShadowMapParam) continue;

		for (unsigned int varIdx = 0; varIdx < paramDesc.Variables; varIdx++) {
			ID3D12ShaderReflectionVariable* varReflection = cbReflection->GetVariableByIndex(varIdx);
			D3D12_SHADER_VARIABLE_DESC varDesc;
			varReflection->GetDesc(&varDesc);

			std::string varName(varDesc.Name);

			constantsBuffersVariables.insert_or_assign(varName, ShaderConstantsBufferVariable({ .bufferIndex = paramIdx, .size = varDesc.Size, .offset = varDesc.StartOffset }));
		}
		cbufferSize.push_back(paramDesc.Size);
	}
}

void ShaderBinary::CreateByteCode(const ComPtr<IDxcResult>& result)
{
	ComPtr<IDxcBlob> code;
	result->GetResult(&code);

	byteCode.resize(code->GetBufferSize());
	memcpy_s(byteCode.data(), code->GetBufferSize(), code->GetBufferPointer(), code->GetBufferSize());
}

