#pragma once

#include <vector>
#include <cstddef>

#include "../Shaders/Compiler/ShaderCompilerOutput.h"
#include "../Types.h"

namespace Templates::Shader {

	struct ShaderDefaultValues
	{
		//shader filename
		std::wstring shaderFileName = L"";

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
}
typedef Templates::Shader::Shader ShaderT;
typedef std::shared_ptr<ShaderT> ShaderPtr;

