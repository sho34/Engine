#pragma once

#include <vector>
#include <cstddef>

#if defined(_DEVELOPMENT)
#include "../../Shaders/Compiler/ShaderCompilerOutput.h"
#endif

namespace Templates::Shader {

	static const std::wstring templateName = L"shaders.json";

	struct ShaderDefaultValues
	{
		//shader filename
		std::wstring shaderFileName = L"";

		//system created flag
		bool systemCreated = false;

		//default values to map to the constant buffers
		MaterialInitialValueMap mappedValues = MaterialInitialValueMap();
	};

	struct Shader
	{
		static std::map<ShaderType, std::wstring> shaderEntryPoint;
		static std::map<ShaderType, std::wstring> shaderTarget;

		ShaderCompilerOutputPtr vertexShader;
		ShaderCompilerOutputPtr pixelShader;
		ShaderDefaultValues defaultValues;
		bool loading = false;
	};

	typedef void LoadShaderFn(std::shared_ptr<Shader>* shader);

	Concurrency::task<void> CreateShaderTemplate(std::wstring shaderTemplateName, ShaderDefaultValues defaultValues = {}, LoadShaderFn loadFn = nullptr);
	void ReleaseShaderTemplates();
	Concurrency::task<void> BindToShaderTemplate(const std::wstring& shaderTemplateName, void* target, NotificationCallbacks callbacks);
	std::shared_ptr<Shader>* GetShaderTemplate(std::wstring shaderTemplateName);

#if defined(_EDITOR)
	nlohmann::json json();
#endif
	Concurrency::task<void> json(std::wstring, nlohmann::json);

}
typedef Templates::Shader::Shader ShaderT;
typedef std::shared_ptr<ShaderT> ShaderPtr;

