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
	//T_Meshes,
	T_Models3D,
	T_Sounds
};

static const std::unordered_map<_Templates, std::string> TemplatesToStr = {
	{ T_Shaders, "Shaders" },
	{ T_Materials, "Materials" },
	//{ T_Meshes,	"Meshes" },
	{ T_Models3D, "Models3D" },
	{ T_Sounds, "Sounds" },
};

#if defined(_EDITOR)
static const std::map<_Templates, std::function<std::vector<std::string>()>> GetTemplates = {
	{ T_Materials, GetMaterialsNames },
	//{ T_Meshes, GetMeshesNames },
	{ T_Models3D, GetModels3DNames },
	{ T_Shaders, GetShadersNames },
	{ T_Sounds, GetSoundsNames }
};

inline auto selectTemplate(std::string from, std::string& to) { to = from; }
inline auto getX(std::string s) { return s; }
inline auto deSelectTemplate(std::string& s) { s = ""; }

static const std::map<_Templates, std::function<void(std::string, std::string&)>> SetSelectedTemplate = {
	{ T_Materials, selectTemplate },
	//{ T_Meshes, selectTemplate },
	{ T_Models3D, selectTemplate },
	{ T_Shaders, selectTemplate },
	{ T_Sounds, selectTemplate }
};

static const std::map<_Templates, std::function<void(std::string&)>> DeSelectTemplate = {
	{ T_Materials, deSelectTemplate },
	//{ T_Meshes, deSelectTemplate },
	{ T_Models3D, deSelectTemplate },
	{ T_Shaders, deSelectTemplate },
	{ T_Sounds, deSelectTemplate }
};

static const std::map<_Templates, std::function<void(std::string&, ImVec2, ImVec2, bool)>> DrawTemplatePanel = {
	{ T_Materials, DrawMaterialPanel },
	//{ T_Meshes, DrawMeshPanel },
	{ T_Models3D, DrawModel3DPanel },
	{ T_Shaders, DrawShaderPanel },
	{ T_Sounds, DrawSoundPanel }
};

static const std::map<_Templates, std::function<std::string(std::string)>> GetTemplateName = {
	{ T_Materials, getX },
	//{ T_Meshes, getX },
	{ T_Models3D, getX },
	{ T_Shaders, getX },
	{ T_Sounds, getX }
};

#endif

namespace Templates {

#if defined(_EDITOR)
	void SaveTemplates(const std::string folder, const std::string fileName, nlohmann::json data);
#endif

	void LoadTemplates(const std::string folder, const std::string fileName, std::function<void(std::string, nlohmann::json)> loader);

	void DestroyTemplates();
#if defined(_EDITOR)
	void DestroyTemplatesReferences();
#endif
}
