#include "pch.h"
#include "Shader.h"
#include "../../Shaders/Compiler/ShaderCompiler.h"
#include <nlohmann/json.hpp>
#if defined(_EDITOR)
#include "../../Editor/Editor.h"
#include "../../pch/Application.h"
#include "../../Resources/ShaderByteCode.h"
#endif

namespace Templates {

	std::map<std::string, nlohmann::json> shaderTemplates;

#if defined(_EDITOR)
	std::map<std::string, std::map<std::string, MaterialVariablesTypes>> shaderConstantVariablesTypes;
#endif

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

	void SetShaderMappedVariable(std::string shaderName, std::string varName, MaterialVariablesTypes type)
	{
		shaderConstantVariablesTypes[shaderName].insert_or_assign(varName, type);
	}

	std::map<std::string, MaterialVariablesTypes> GetShaderMappeableVariables(std::string shaderName)
	{
		if (!shaderConstantVariablesTypes.contains(shaderName)) return std::map<std::string, MaterialVariablesTypes>();
		return shaderConstantVariablesTypes.at(shaderName);
	}



	void DrawShaderPanel(std::string& shader, ImVec2 pos, ImVec2 size, bool pop)
	{
		nlohmann::json& mat = shaderTemplates.at(shader);

		std::string fileName = mat.contains("fileName") ? std::string(mat.at("fileName")) : shader;
		if (ImGui::Button(ICON_FA_ELLIPSIS_H))
		{
			Editor::OpenFile([&mat](std::filesystem::path shaderPath)
				{
					std::filesystem::path hlslFilePath = shaderPath;
					hlslFilePath.replace_extension(".hlsl");
					mat.at("fileName") = hlslFilePath.stem().string();
				}
			, defaultShadersFolder, "Shader files. (*.hlsl)", "*.hlsl");
		}
		ImGui::SameLine();
		ImGui::InputText("fileName", fileName.data(), fileName.size(), ImGuiInputTextFlags_ReadOnly);

		std::set<std::string> existingVariables;
		if (mat.contains("mappedValues"))
		{
			unsigned int sz = static_cast<unsigned int>(mat.at("mappedValues").size());
			for (unsigned int i = 0; i < sz; i++)
			{
				existingVariables.insert(mat.at("mappedValues").at(i).at("variable"));
			}
		}

		std::vector<std::string> selectables = { " " };
		std::map<std::string, MaterialVariablesTypes> mappeables = GetShaderMappeableVariables(fileName.data());
		for (auto it = mappeables.begin(); it != mappeables.end(); it++)
		{
			if (existingVariables.contains(it->first)) continue;
			selectables.push_back(it->first);
		}

		//draw a combo for adding attributes to the object
		ImGui::Text("Mapped Values");
		if (selectables.size() > 1)
		{
			ImGui::PushID("mapped-values-add-combo");
			DrawComboSelection(selectables[0], selectables, [&mat, mappeables](std::string variable)
				{
					if (!mat.contains("mappedValues")) { mat["mappedValues"] = nlohmann::json::array(); }
					MaterialVariablesMappedJsonInitializer.at(mappeables.at(variable))(mat["mappedValues"], variable);
				}, ""
			);
			ImGui::PopID();
		}

		if (mat.contains("mappedValues"))
		{
			unsigned int sz = static_cast<unsigned int>(mat.at("mappedValues").size());
			for (unsigned int i = 0; i < sz; i++)
			{
				nlohmann::json& mappedV = mat.at("mappedValues").at(i);
				ImMaterialVariablesDraw.at(StrToMaterialVariablesTypes.at(std::string(mappedV.at("variableType"))))(i, mappedV);
			}
		}
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