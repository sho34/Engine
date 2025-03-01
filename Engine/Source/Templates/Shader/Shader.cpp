#include "pch.h"
#include "Shader.h"
#include "../../Shaders/Compiler/ShaderCompiler.h"
#include <nlohmann/json.hpp>

namespace Templates {

	std::map<std::string, nlohmann::json> shaderTemplates;

	//NOTIFICATIONS

	//CREATE
	void CreateShader(std::string name, nlohmann::json json)
	{
		if (shaderTemplates.contains(name)) return;
		shaderTemplates.insert_or_assign(name, json);
	}

	//READ&GET
	nlohmann::json GetShaderTemplate(std::string name)
	{
		return shaderTemplates.at(name);
	}

	std::vector<std::string> GetShadersNames() {
		return nostd::GetKeysFromMap(shaderTemplates);
	}
	//UPDATE

	//DELETE
	void ReleaseShaderTemplates()
	{
		shaderTemplates.clear();
	}

	//EDITOR
#if defined(_EDITOR)
	void DrawShaderPanel(std::string& shader, ImVec2 pos, ImVec2 size, bool pop)
	{
	}

	/*
	nlohmann::json json()
	{
		nlohmann::json j = nlohmann::json({});

		//for (auto& [name, shader] : shaderTemplates) {
		//	if (shader->defaultValues.systemCreated) continue;
		//	j[name] = nlohmann::json({});
		//	j[name]["fileName"] = shader->defaultValues.shaderFileName;
		//	j[name]["mappedValues"] = TransformMaterialValueMappingToJson(shader->defaultValues.mappedValues);
		//}
		return j;
	}
	*/
#endif

}