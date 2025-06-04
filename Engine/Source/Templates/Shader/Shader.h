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
#include <UUID.h>
#include "../TemplateDecl.h"

namespace Templates {

#if defined(_EDITOR)
	enum ShaderPopupModal
	{
		ShaderPopupModal_CannotDelete = 1,
		ShaderPopupModal_CreateNew = 2
	};
#endif

	typedef std::tuple<
		std::string, //name
		nlohmann::json //json data
#if defined(_EDITOR)
		,
		std::map<std::string, MaterialVariablesTypes>, // constant variables types
		std::set<TextureShaderUsage>, // textures types
		unsigned int, // num samplers
		std::vector<std::string> //materials uuid references
#endif
	> ShaderTemplate;

	TEMPDECL_GETTEMPLATES(Shader);

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

		//UAV
		ShaderUAVParametersMap uavParameters;

		//SRV CS
		ShaderSRVCSParametersMap srvCSParameters;

		//SRV Tex
		ShaderSRVTexParametersMap srvTexParameters;

		//Samplers
		ShaderSamplerParametersMap samplersParameters;

		std::vector<size_t> cbufferSize;

		//Specific registers slots (c1,c2,c3,...) 
		int cameraCBVRegister = -1;
		int lightCBVRegister = -1;
		int animationCBVRegister = -1;
		int lightsShadowMapCBVRegister = -1;
		int lightsShadowMapSRVRegister = -1;
		int iblIrradianceSRVRegister = -1;
		int iblPrefiteredEnvSRVRegister = -1;
		int iblBRDFLUTSRVRegister = -1;

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
	TEMPDECL_GET(Shader);
	TEMPDECL_GETNAMES(Shader);
	TEMPDECL_GETNAME(Shader);
	TEMPDECL_FINDUUIDBYNAME(Shader);
	TEMPDECL_GETUUIDNAMES(Shader);
	TEMPDECL_RELEASE(Shader);
	std::vector<UUIDName> GetShadersUUIDsNamesByType(ShaderType type);
	std::shared_ptr<ShaderInstance> GetShaderInstance(Source params);

	std::shared_ptr<ShaderInstance> LoadShaderInstanceFromBinary(size_t hash);
#if defined(_DEVELOPMENT)
	void SaveShaderInstanceFromBinary(std::shared_ptr<ShaderInstance> instance);
#endif
	void LoadShaderSource(std::ifstream& file, Source& source);
#if defined(_DEVELOPMENT)
	void WriteShaderSource(std::ofstream& file, Source& source);
#endif
	void LoadShaderVSSemantics(std::ifstream& file, std::vector<std::string>& vsSemantics);
#if defined(_DEVELOPMENT)
	void WriteShaderVSSemantics(std::ofstream& file, std::vector<std::string>& vsSemantics);
#endif
	void LoadShaderConstantsBufferParameters(std::ifstream& file, ShaderConstantsBufferParametersMap& constantsBuffersParameters);
#if defined(_DEVELOPMENT)
	void WriteShaderConstantsBufferParameters(std::ofstream& file, ShaderConstantsBufferParametersMap& constantsBuffersParameters);
#endif
	void LoadShaderConstantsBufferVariables(std::ifstream& file, ShaderConstantsBufferVariablesMap& constantsBuffersVariables);
#if defined(_DEVELOPMENT)
	void WriteShaderConstantsBufferVariables(std::ofstream& file, ShaderConstantsBufferVariablesMap& constantsBuffersVariables);
#endif
	void LoadShaderComputeParameters(std::ifstream& file, ShaderSRVCSParametersMap& srvCSParameters);
#if defined(_DEVELOPMENT)
	void WriteShaderComputeParameters(std::ofstream& file, ShaderSRVCSParametersMap& srvCSParameters);
#endif
	void LoadShaderTextureParameters(std::ifstream& file, ShaderSRVTexParametersMap& srvTexParameters);
#if defined(_DEVELOPMENT)
	void WriteShaderTextureParameters(std::ofstream& file, ShaderSRVTexParametersMap& srvTexParameters);
#endif
	void LoadShaderSamplerParameters(std::ifstream& file, ShaderSamplerParametersMap& samplersParameters);
#if defined(_DEVELOPMENT)
	void WriteShaderSamplerParameters(std::ofstream& file, ShaderSamplerParametersMap& samplersParameters);
#endif
	void LoadShaderBufferSizes(std::ifstream& file, std::vector<size_t>& cbufferSize);
#if defined(_DEVELOPMENT)
	void WriteShaderBufferSizes(std::ofstream& file, std::vector<size_t>& cbufferSize);
#endif
	void LoadShaderRegister(std::ifstream& file, int& reg);
#if defined(_DEVELOPMENT)
	void WriteShaderRegister(std::ofstream& file, int& reg);
#endif	
	void LoadShaderByteCode(std::ifstream& file, ShaderByteCode& byteCode);
#if defined(_DEVELOPMENT)
	void WriteShaderByteCode(std::ofstream& file, ShaderByteCode& byteCode);
#endif

#if defined(_EDITOR)
	std::map<std::string, MaterialVariablesTypes> GetShaderMappeableVariables(std::string uuid);
	std::set<TextureShaderUsage> GetShaderTextureParameters(std::string uuid);
	unsigned int GetShaderSamplerParameters(std::string uuid);
#endif

	//UPDATE
#if defined(_EDITOR)
	void SetShaderMappedVariable(std::string uuid, std::string varName, MaterialVariablesTypes type);
#endif
	void MonitorShaderChanges(std::string folder);

	//DESTROY
	void DestroyShaderBinary(std::shared_ptr<ShaderInstance>& shaderBinary);

	//EDITOR
#if defined(_EDITOR)
	void DrawShaderPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop);
	void AttachMaterialToShader(std::string uuid, std::string materialUUID);
	void DetachMaterialsFromShader(std::string uuid);
	void CreateNewShader();
	void DeleteShader(std::string uuid);
	void DrawShadersPopups();
	bool ShadersPopupIsOpen();
	void WriteShadersJson(nlohmann::json& json);
#endif
}

