#pragma once
#include "Shader/Shader.h"
#include "Material/Material.h"
#include "Mesh/Mesh.h"
#include "Model3D/Model3D.h"
#include "Sound/Sound.h"

using namespace Templates;

enum _Templates {
	T_Shaders,
	T_Materials,
	T_Models3D,
	T_Sounds
};

static const std::unordered_map<_Templates, std::string> TemplatesToStr = {
	{ T_Shaders, "Shaders" },
	{ T_Materials, "Materials" },
	{ T_Models3D, "Models3D" },
	{ T_Sounds, "Sounds" },
};

#if defined(_EDITOR)
static const std::map<_Templates, std::function<std::vector<UUIDName>()>> GetTemplates =
{
	{ T_Materials, GetMaterialsUUIDsNames },
	{ T_Models3D, GetModels3DUUIDsNames },
	{ T_Shaders, GetShadersUUIDsNames },
	{ T_Sounds, GetSoundsUUIDsNames }
};

inline auto selectTemplate(std::string from, std::string& to) { to = from; }
inline auto getX(std::string s) { return s; }
inline auto deSelectTemplate(std::string& s) { s = ""; }

static const std::map<_Templates, std::function<void(std::string, std::string&)>> SetSelectedTemplate = {
	{ T_Materials, selectTemplate },
	{ T_Models3D, selectTemplate },
	{ T_Shaders, selectTemplate },
	{ T_Sounds, selectTemplate }
};

static const std::map<_Templates, std::function<void(std::string&)>> DeSelectTemplate = {
	{ T_Materials, deSelectTemplate },
	{ T_Models3D, deSelectTemplate },
	{ T_Shaders, deSelectTemplate },
	{ T_Sounds, deSelectTemplate }
};

static const std::map<_Templates, std::function<void(std::string, ImVec2, ImVec2, bool)>> DrawTemplatePanel = {
	{ T_Materials, DrawMaterialPanel },
	{ T_Models3D, DrawModel3DPanel },
	{ T_Shaders, DrawShaderPanel },
	{ T_Sounds, DrawSoundPanel }
};

static const std::map<_Templates, std::function<void()>> DrawTemplatesPopups = {
	{ T_Materials, DrawMaterialsPopups },
	{ T_Models3D, DrawModels3DsPopups },
	{ T_Shaders, DrawShadersPopups },
	{ T_Sounds, DrawSoundsPopups }
};

static const std::map<_Templates, std::function<std::string(std::string)>> GetTemplateName = {
	{ T_Materials, GetMaterialName },
	{ T_Models3D, GetModel3DName },
	{ T_Shaders, GetShaderName },
	{ T_Sounds, GetSoundName }
};

static const std::map<_Templates, std::function<void()>> CreateTemplate =
{
	{ T_Materials, CreateNewMaterial },
	{ T_Models3D, CreateNewModel3D },
	{ T_Shaders, CreateNewShader },
	{ T_Sounds, CreateNewSound }
};

static const std::map<_Templates, std::function<void(std::string)>> DeleteTemplate = {
	{ T_Materials, DeleteMaterial },
	{ T_Models3D, DeleteModel3D },
	{ T_Shaders, DeleteShader },
	{ T_Sounds, DeleteSound }
};

#endif

namespace Templates {

	using namespace nlohmann::literals;

	static const nlohmann::json systemShaders = R"(
	[
		{
			"name":"BoundingBox",
			"systemCreated" : true,
			"mappedValues": [
				{ "value": [ 1.0, 0.0, 0.0 ], "variable": "baseColor", "variableType": "RGB" }
			],
			"uuid":"ae7a35a5-f012-4eb6-bbe1-1f52e6203ccb"
		},
		{
			"name":"BaseLighting",
			"systemCreated" : true,
			"mappedValues": [
				{ "value": [ 0.11764706671237946, 0.5647059082984924, 1.0 ], "variable": "baseColor", "variableType": "RGB" },
				{ "value": 400.0, "variable": "specularExponent", "variableType": "FLOAT" }
			],
			"uuid":"bc331f48-6a40-4b48-b435-8276051d6993"
		},
		{
			"name":"Grid",
			"systemCreated" : true,
			"mappedValues": [
				{ "value": [ 1.0, 0.0, 1.0 ], "variable": "baseColor", "variableType": "RGB" },
				{ "value": 1024.0, "variable": "specularExponent", "variableType": "FLOAT" }
			],
			"uuid":"5af4ba59-a09c-41ef-bc1f-13a51fc68439"
		},
		{
			"name":"ShadowMap",
			"systemCreated" : true,
			"uuid":"0069d1e9-45b0-4fd3-a28f-1f7508503a91"
		},
		{
			"name":"DepthMinMax",
			"systemCreated" : true,
			"uuid":"2ad43d9e-8dec-421c-b8f2-bda3520748bd"
		},
		{
			"name":"DepthMinMaxToRGBA",
			"systemCreated" : true,
			"uuid":"9815152b-84ad-45e5-8b91-0642cfde0543"
		},
		{
			"name":"FullScreenQuad",
			"systemCreated" : true,
			"uuid":"8e26fbd4-3a2c-4c04-a628-d2f11d474d60"
		},
		{
			"name":"LoadingBar",
			"systemCreated" : true,
			"uuid":"d0192f97-a56a-469d-b6f1-07d403ae331a"
		}
	])"_json;

	static const nlohmann::json systemMaterials = R"(
	[
		{
			"name":"BoundingBox",
			"shader":"ae7a35a5-f012-4eb6-bbe1-1f52e6203ccb",
			"systemCreated":true,
			"uuid":"2e4d8bf0-0761-45d9-8313-17cdf9b5f8fc"
		},
		{
			"name":"BaseLighting",
			"shader":"bc331f48-6a40-4b48-b435-8276051d6993",
			"systemCreated":true,
			"samplers":[
				{
					"Filter":"MIN_MAG_MIP_LINEAR",
					"AddressU":"ADDRESS_MODE_BORDER",
					"AddressV":"ADDRESS_MODE_BORDER",
					"AddressW":"ADDRESS_MODE_BORDER",
					"MipLODBias":0,
					"MaxAnisotropy":0,
					"ComparisonFunc":"NEVER",
					"BorderColor":"OPAQUE_WHITE",
					"MinLOD":0.0,
					"MaxLOD":3.4028234663852886e+38,
					"ShaderRegister":0,
					"RegisterSpace":0,
					"ShaderVisibility":"PIXEL"
				}
			],
			"uuid":"4a5a2cb8-f2ea-4e15-8584-22bb675ae1bc"
		},
		{
			"name":"Floor",
			"shader":"5af4ba59-a09c-41ef-bc1f-13a51fc68439",
			"systemCreated":true,
			"samplers":[
				{
					"Filter":"MIN_MAG_MIP_LINEAR",
					"AddressU":"ADDRESS_MODE_BORDER",
					"AddressV":"ADDRESS_MODE_BORDER",
					"AddressW":"ADDRESS_MODE_BORDER",
					"MipLODBias":0,
					"MaxAnisotropy":0,
					"ComparisonFunc":"NEVER",
					"BorderColor":"OPAQUE_WHITE",
					"MinLOD":0.0,
					"MaxLOD":3.4028234663852886e+38,
					"ShaderRegister":0,
					"RegisterSpace":0,
					"ShaderVisibility":"PIXEL"
				}
			],
			"uuid":"ecd1688c-73d6-49d0-870f-ca916a417c49"
		},
		{
			"name":"ShadowMap",
			"shader":"0069d1e9-45b0-4fd3-a28f-1f7508503a91",
			"systemCreated":true,
			"samplers":[
				{
					"Filter":"MIN_MAG_MIP_LINEAR",
					"AddressU":"ADDRESS_MODE_BORDER",
					"AddressV":"ADDRESS_MODE_BORDER",
					"AddressW":"ADDRESS_MODE_BORDER",
					"MipLODBias":0,
					"MaxAnisotropy":0,
					"ComparisonFunc":"NEVER",
					"BorderColor":"OPAQUE_WHITE",
					"MinLOD":0.0,
					"MaxLOD":3.4028234663852886e+38,
					"ShaderRegister":0,
					"RegisterSpace":0,
					"ShaderVisibility":"PIXEL"
				}
			],
			"uuid":"3be1cf4e-cc15-41ae-97e1-6bb3e110271f"
		},
		{
			"name":"DepthMinMax",
			"shader":"2ad43d9e-8dec-421c-b8f2-bda3520748bd",
			"systemCreated":true,
			"twoSided": true,
			"samplers":[
				{
					"Filter":"MIN_MAG_MIP_POINT",
					"AddressU":"ADDRESS_MODE_BORDER",
					"AddressV":"ADDRESS_MODE_BORDER",
					"AddressW":"ADDRESS_MODE_BORDER",
					"MipLODBias":0,
					"MaxAnisotropy":0,
					"ComparisonFunc":"NEVER",
					"BorderColor":"OPAQUE_WHITE",
					"MinLOD":0.0,
					"MaxLOD":3.4028234663852886e+38,
					"ShaderRegister":0,
					"RegisterSpace":0,
					"ShaderVisibility":"PIXEL"
				}
			],
			"uuid":"35da9e7d-1ef8-4165-8e71-36d6cf599c3c"
		},
		{
			"name":"DepthMinMaxToRGBA",
			"shader":"9815152b-84ad-45e5-8b91-0642cfde0543",
			"systemCreated":true,
			"twoSided": true,
			"samplers":[
				{
					"Filter":"MIN_MAG_MIP_POINT",
					"AddressU":"ADDRESS_MODE_BORDER",
					"AddressV":"ADDRESS_MODE_BORDER",
					"AddressW":"ADDRESS_MODE_BORDER",
					"MipLODBias":0,
					"MaxAnisotropy":0,
					"ComparisonFunc":"NEVER",
					"BorderColor":"OPAQUE_WHITE",
					"MinLOD":0.0,
					"MaxLOD":3.4028234663852886e+38,
					"ShaderRegister":0,
					"RegisterSpace":0,
					"ShaderVisibility":"PIXEL"
				}
			],
			"uuid" : "84f0cabb-9b0c-4508-ac6e-d7a84dee696f"
		},
		{
			"name":"FullScreenQuad", 
			"shader":"8e26fbd4-3a2c-4c04-a628-d2f11d474d60",
			"systemCreated":true,
			"uuid" : "8e98708c-fe2e-4123-b1f0-5b80fabd1888"
		},
		{
			"name":"LoadingBar",
			"shader" : "d0192f97-a56a-469d-b6f1-07d403ae331a",
			"systemCreated" : true,
			"uuid":"28c4d879-6d21-408f-acbb-120f9fdc05b0"
		}
	])"_json;

#if defined(_EDITOR)
	void SaveTemplates(const std::string folder, const std::string fileName, std::function<void(nlohmann::json&)> writer);
#endif

	void LoadTemplates(nlohmann::json templates, std::function<void(nlohmann::json)> loader);
	void LoadTemplates(const std::string folder, const std::string fileName, std::function<void(nlohmann::json)> loader);

	void DestroyTemplates();
#if defined(_EDITOR)
	void DestroyTemplatesReferences();
#endif

	template<typename T>
	void WriteTemplateJson(nlohmann::json& json, std::map<std::string, T> Ts)
	{
		std::map<std::string, T> filtered;

		std::copy_if(Ts.begin(), Ts.end(), std::inserter(filtered, filtered.end()), [](auto pair)
			{
				nlohmann::json j = std::get<1>(pair.second);
				return !(j.contains("systemCreated") && j.at("systemCreated") == true);
			}
		);

		std::transform(filtered.begin(), filtered.end(), std::inserter(json, json.end()), [](const auto& pair)
			{
				nlohmann::json j = std::get<1>(pair.second);
				j["uuid"] = pair.first;
				j["name"] = std::get<0>(pair.second);
				return j;
			}
		);
	}

	inline std::vector<std::string> GetNames(auto& items)
	{
		std::vector<std::string> names;
		std::transform(items.begin(), items.end(), std::back_inserter(names), [](auto pair)
			{
				return std::get<0>(pair.second);
			}
		);
		return names;
	}

	inline std::vector<UUIDName> GetUUIDsNames(auto& items)
	{
		std::vector<std::tuple<std::string, std::string>> uuidsNames;
		std::transform(items.begin(), items.end(), std::back_inserter(uuidsNames), [](auto pair)
			{
				return std::make_tuple(pair.first, std::get<0>(pair.second));
			}
		);
		return uuidsNames;
	}
}
