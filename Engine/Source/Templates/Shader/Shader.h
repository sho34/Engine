#pragma once
#include <map>
#include <vector>
#include <memory>
#include <d3d12shader.h>
#include <wrl.h>
#include <wrl/client.h>
#include <dxcapi.h>
#include <string>
#include <nlohmann/json.hpp>
#include <ShaderMaterials.h>
#include "../Material/Variables.h"

namespace Templates {

	namespace Shader
	{
		inline static const std::string templateName = "shaders.json";
	};

	struct ShaderInstance {
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

		std::vector<std::function<void()>> changesCallbacks;

		void CopyFrom(std::shared_ptr<ShaderInstance>& src);

		void CreateVSSemantics(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc);

		void CreateResourcesBinding(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc);

		void CreateConstantsBuffersVariables(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc);

		void CreateByteCode(const ComPtr<IDxcResult>& result);

		void BindChange(std::function<void()> changeListener);

		void NotifyChanges();
	};

	//CREATE
	void CreateShader(std::string name, nlohmann::json json);

	//READ&GET
	nlohmann::json GetShaderTemplate(std::string name);
	std::vector<std::string> GetShadersNames();
	std::shared_ptr<ShaderInstance> GetShaderInstance(Source params);

	//UPDATE
	void MonitorShaderChanges(std::string folder);

	//DESTROY
	void ReleaseShaderTemplates();
	void DestroyShaderBinary(std::shared_ptr<ShaderInstance>& shaderBinary);

	//EDITOR
#if defined(_EDITOR)
	void SetShaderMappedVariable(std::string shaderName, std::string varName, MaterialVariablesTypes type);
	std::map<std::string, MaterialVariablesTypes> GetShaderMappeableVariables(std::string shaderName);
	std::set<TextureType> GetShaderTextureParameters(std::string shaderName);
	unsigned int GetShaderSamplerParameters(std::string shaderName);
	void DrawShaderPanel(std::string& shader, ImVec2 pos, ImVec2 size, bool pop);
	/*
	std::string GetShaderName(void* ptr);
	nlohmann::json json();
	*/
#endif
}

