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

static const std::unordered_map<_Templates, std::wstring> TemplatesToStr = {
	{ Materials, L"Materials" },
	{ Meshes,	L"Meshes" },
	{ Models3D, L"Models3D" },
	{ Shaders, L"Shaders" },
	{ Sounds, L"Sounds" },
};

static const std::map<_Templates, std::function<std::vector<std::wstring>()>> GetTemplates = {
	{ Materials, GetMaterialsNames },
	{ Meshes, GetMeshesNames },
	{ Models3D, GetModels3DNames },
	{ Shaders, GetShadersNames },
	{ Sounds, GetSoundsNames }
};