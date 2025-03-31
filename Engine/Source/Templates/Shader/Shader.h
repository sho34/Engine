#pragma once
#include <set>
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

	typedef std::tuple<
		std::string, //name
		nlohmann::json //json data
#if defined(_EDITOR)
		,
		std::map<std::string, MaterialVariablesTypes>, // constant variables types
		std::set<TextureType>, // textures types
		unsigned int, // num samplers
		std::vector<std::string> //materials uuid references
#endif
	> ShaderTemplate;

	namespace Shader
	{
		inline static const std::string templateName = "shaders.json";
#if defined(_EDITOR)
		void DrawEditorInformationAttributes(std::string uuid);
		void DrawEditorAssetAttributes(std::string uuid);
#endif
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
	void CreateShader(nlohmann::json json);

	//READ&GET
	nlohmann::json GetShaderTemplate(std::string uuid);
	std::vector<std::string> GetShadersNames();
	std::string GetShaderName(std::string uuid);
	std::vector<UUIDName> GetShadersUUIDsNames();
	std::shared_ptr<ShaderInstance> GetShaderInstance(Source params);
#if defined(_EDITOR)
	std::map<std::string, MaterialVariablesTypes> GetShaderMappeableVariables(std::string uuid);
	std::set<TextureType> GetShaderTextureParameters(std::string uuid);
	unsigned int GetShaderSamplerParameters(std::string uuid);
#endif

	//UPDATE
#if defined(_EDITOR)
	void SetShaderMappedVariable(std::string uuid, std::string varName, MaterialVariablesTypes type);
#endif
	void MonitorShaderChanges(std::string folder);

	//DESTROY
	void ReleaseShaderTemplates();
	void DestroyShaderBinary(std::shared_ptr<ShaderInstance>& shaderBinary);

	//EDITOR
#if defined(_EDITOR)
	void DrawShaderPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop);
	void AttachMaterialToShader(std::string uuid, std::string materialUUID);
	void DetachMaterialsFromShader(std::string uuid);
	void DeleteShader(std::string uuid);
	void DrawShadersPopups();
	/*
	nlohmann::json json();
	*/
#endif
}

