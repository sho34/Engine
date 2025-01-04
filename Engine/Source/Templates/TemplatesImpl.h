#pragma once
#include "Material/MaterialImpl.h"
#include "Mesh/MeshImpl.h"
#include "Model3D/Model3DImpl.h"
#include "Shader/ShaderImpl.h"
#include "Sound/SoundImpl.h"
#include "Templates.h"
using namespace Templates;

enum _Templates {
	Materials,
	Meshes,
	Models3D,
	Shaders,
	Sounds
};

static const std::unordered_map<_Templates, std::string> TemplatesToStr = {
	{ Materials, "Materials" },
	{ Meshes,	"Meshes" },
	{ Models3D, "Models3D" },
	{ Shaders, "Shaders" },
	{ Sounds, "Sounds" },
};

static const std::map<_Templates, std::function<std::vector<std::string>()>> GetTemplates = {
	{ Materials, GetMaterialsNames },
	{ Meshes, GetMeshesNames },
	{ Models3D, GetModels3DNames },
	{ Shaders, GetShadersNames },
	{ Sounds, GetSoundsNames }
};

static const std::map<_Templates, std::function<void(std::string,void*&)>> SetSelectedTemplate = {
	{ Materials, SelectMaterial },
	{ Meshes, SelectMesh },
	{ Models3D, SelectModel3D },
	{ Shaders, SelectShader },
	{ Sounds, SelectSound }
};

static const std::map<_Templates, std::function<void(void*&, ImVec2, ImVec2)>> DrawTemplatePanel = {
	{ Materials, DrawMaterialPanel },
	{ Meshes, DrawMeshPanel },
	{ Models3D, DrawModel3DPanel },
	{ Shaders, DrawShaderPanel },
	{ Sounds, DrawSoundPanel }
};

static const std::map<_Templates, std::function<std::string(void*)>> GetTemplateName = {
	{ Materials, GetMaterialName },
	{ Meshes, GetMeshName },
	{ Models3D, GetModel3DName },
	{ Shaders, GetShaderName },
	{ Sounds, GetSoundName }
};