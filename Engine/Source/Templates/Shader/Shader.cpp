#include "pch.h"
#include "Shader.h"
#include "../../Shaders/Compiler/ShaderCompiler.h"
#include <nlohmann/json.hpp>

namespace Templates {

	//std::map<std::string, std::shared_ptr<Shader>> shaderTemplates;
	std::map<std::string, nlohmann::json> shaderTemplates;
	/*
	std::map<ShaderType, std::string> ShaderT::shaderEntryPoint = {
		 {	ShaderType::VERTEX_SHADER, "main_vs"	},
		 {	ShaderType::PIXEL_SHADER, "main_ps"	},
	};
	std::map<ShaderType, std::string> ShaderT::shaderTarget = {
		 {	ShaderType::VERTEX_SHADER, "vs_6_1"	},
		 {	ShaderType::PIXEL_SHADER, "ps_6_1"},
	};
	*/
	//TemplatesNotification<ShaderPtr*> shaderChangeNotifications;

	//NOTIFICATIONS
	/*
	static void OnShaderCompilationStart(void* shaderPtr) {
		auto shader = static_cast<ShaderPtr*>(shaderPtr);
		if (shaderChangeNotifications.contains(shader)) {
			NotifyOnLoadStart<ShaderPtr*>(shaderChangeNotifications[shader]);
		}
		(*shader)->flags|= TemplateFlags::Loading;
		(*shader)->pixelShader = nullptr;
		(*shader)->vertexShader = nullptr;
	}
	*/

	/*
	static void OnShaderCompilationComplete(void* shaderPtr, void* shaderCompilerOutputPtr) {
		auto shader = static_cast<ShaderPtr*>(shaderPtr);
		auto output = static_cast<ShaderCompilerOutputPtr*>(shaderCompilerOutputPtr);
		switch ((*output)->shaderType) {
		case ShaderType::VERTEX_SHADER:
			(*shader)->vertexShader = *output;
			break;
		case ShaderType::PIXEL_SHADER:
			(*shader)->pixelShader = *output;
			break;
		}
		if ((*shader)->vertexShader && (*shader)->pixelShader != nullptr) {
			(*shader)->loading = false;
			if (shaderChangeNotifications.contains(shader)) {
				NotifyOnLoadComplete(shader,shaderChangeNotifications[shader]);
			}
		}
	}
	*/

	/*
	static void OnShaderCompilerOutputDestroy(void* shaderPtr, void* shaderCompilerOutputPtr) {}
	*/

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