#pragma once
#include "../Material/Variables.h"
#include "../../Resources/ShaderByteCode.h"

namespace Templates {

	namespace Shader
	{
		inline static const std::string templateName = "shaders.json";
	};

	//CREATE
	void CreateShader(std::string name, nlohmann::json json);

	//READ&GET
	nlohmann::json GetShaderTemplate(std::string name);
	std::vector<std::string> GetShadersNames();

	//UPDATE

	//DESTROY
	void ReleaseShaderTemplates();

	//EDITOR
#if defined(_EDITOR)
	void SetShaderMappedVariable(std::string shaderName, std::string varName, MaterialVariablesTypes type);
	std::map<std::string, MaterialVariablesTypes> GetShaderMappeableVariables(std::string shaderName);
	void DrawShaderPanel(std::string& shader, ImVec2 pos, ImVec2 size, bool pop);
	/*
	std::string GetShaderName(void* ptr);
	nlohmann::json json();
	*/
#endif
}

