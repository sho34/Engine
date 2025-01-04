#pragma once

#include <vector>
#include <cstddef>

#if defined(_DEVELOPMENT)
#include "../../Shaders/Compiler/ShaderCompilerOutput.h"
#endif

namespace Templates::Shader {

	static const std::string templateName = "shaders.json";

	struct ShaderDefaultValues
	{
		//shader filename
		std::string shaderFileName = "";

		//system created flag
		bool systemCreated = false;

		//default values to map to the constant buffers
		MaterialInitialValueMap mappedValues = MaterialInitialValueMap();
	};

	struct Shader
	{
		std::string name;
		static std::map<ShaderType, std::string> shaderEntryPoint;
		static std::map<ShaderType, std::string> shaderTarget;

		ShaderCompilerOutputPtr vertexShader;
		ShaderCompilerOutputPtr pixelShader;
		ShaderDefaultValues defaultValues;
		bool loading = false;
	};

	typedef void LoadShaderFn(std::shared_ptr<Shader>* shader);

	Concurrency::task<void> CreateShaderTemplate(std::string shaderTemplateName, ShaderDefaultValues defaultValues = {}, LoadShaderFn loadFn = nullptr);
	void ReleaseShaderTemplates();
	Concurrency::task<void> BindToShaderTemplate(const std::string& shaderTemplateName, void* target, NotificationCallbacks callbacks);
	std::shared_ptr<Shader>* GetShaderTemplate(std::string shaderTemplateName);
	std::map<std::string, std::shared_ptr<Shader>> GetNamedShaders();
	std::vector<std::string> GetShadersNames();

#if defined(_EDITOR)
	void SelectShader(std::string shaderName, void*& ptr);
	void DrawShaderPanel(void*& ptr, ImVec2 pos, ImVec2 size);
	std::string GetShaderName(void* ptr);
	nlohmann::json json();
#endif
	Concurrency::task<void> json(std::string, nlohmann::json);

}
typedef Templates::Shader::Shader ShaderT;
typedef std::shared_ptr<ShaderT> ShaderPtr;

