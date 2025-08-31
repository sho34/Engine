#pragma once

namespace Templates
{
	struct ShaderInstance
	{
		std::string instanceUUID;
		std::string shaderUUID;
		Source shaderSource;

		//vertex shader semantics(POSITION,TEXCOORD0, etc)
		std::vector<std::string> vsSemantics;

		//CBV
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
		struct
		{
			int camera = -1;
			int light = -1;
			int animation = -1;
			int lightsShadowMap = -1;
		} CBV;

		struct
		{
			int lightsShadowMap = -1;
			int iblIrradiance = -1;
			int iblPrefiteredEnv = -1;
			int iblBRDFLUT = -1;
		} SRV;

		//the bytecode(vector of bytes)
		ShaderByteCode byteCode;

		ShaderInstance(std::string uuid) { assert(!!!"do not use"); }
		explicit ShaderInstance(
			std::string instance_uuid,
			std::string uuid, Source params,
			std::string objectUUID = "",
			JObjectChangeCallback cb = [](std::shared_ptr<JObject>) {},
			JObjectChangePostCallback postCb = [](unsigned int, unsigned int) {}
		);
		~ShaderInstance() {}

		void CreateVSSemantics(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc);
		void CreateResourcesBinding(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc);
		void CreateConstantsBuffersVariables(const ComPtr<ID3D12ShaderReflection>& reflection, const D3D12_SHADER_DESC& desc);
		void CreateByteCode(const ComPtr<IDxcResult>& result);
	};
};