#pragma once

#include <regex>
#include <Material/SamplerDesc.h>
#include <Material/BlendDesc.h>
#include <ShaderMaterials.h>
#include <ImEditor.h>
#include <Shader/ShaderInstance.h>
#include <NoStd.h>
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>
#include <Light/Light.h>
#include <Light/ShadowMap.h>
#include <Sound/SoundFX.h>

namespace Templates
{
	struct ShaderInstance;
	extern std::vector<UUIDName> GetMaterialsUUIDsNames();
	extern std::vector<UUIDName> GetMeshesUUIDsNames();
	extern std::vector<UUIDName> GetModel3DsUUIDsNames();
	extern std::vector<UUIDName> GetRenderPasssUUIDsNames();
	extern std::vector<UUIDName> GetShadersUUIDsNames();
	extern std::vector<UUIDName> GetSoundsUUIDsNames();
	extern std::vector<UUIDName> GetTexturesUUIDsNames();
	extern std::string GetMeshName(std::string uuid);
	extern std::string GetModel3DName(std::string uuid);
	extern std::string GetMaterialName(std::string uuid);
	extern std::string GetShaderName(std::string uuid);
	extern std::string GetSoundName(std::string uuid);
	extern std::string GetTextureName(std::string uuid);
	extern std::shared_ptr<ShaderInstance> FindShaderInstance(std::string uuid);
};

namespace Scene
{
	extern std::vector<UUIDName> GetCamerasUUIDNames();
	extern std::vector<UUIDName> GetLightsUUIDNames();
	extern std::vector<UUIDName> GetRenderablesUUIDNames();
	extern std::vector<UUIDName> GetSoundEffectsUUIDNames();
};

namespace Editor
{
	extern void MarkTemplatesPanelAssetsAsDirty();
	extern void MarkScenePanelAssetsAsDirty();
	extern void OpenAnimationSequencer(std::string uuid);
};

namespace Game
{
	extern std::vector<std::string> GetGameControllers();
};

const int defaultTableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX;


#include <JTypes.h>
#include <Functions/JEdvEditorDrawer.h>
#include <Functions/JEdvCreatorDrawer.h>
#include <Functions/JEdvCreatorValidator.h>
