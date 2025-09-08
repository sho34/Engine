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
#include <Lights/Lights.h>
#include <Lights/ShadowMap.h>
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
};

const int defaultTableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX;

inline void drawAnimationController(
	std::function<bool()> animationsArePlaying,
	std::function<void(bool)> setPlayAnimation,
	std::function<void(float)> setAnimationTime,
	std::function<float()> getAnimationTimeFactor,
	std::function<void(float)> setAnimationTimeFactor,
	std::function<void()> gotoPrevAnimation,
	std::function<void()> gotoNextAnimation,
	std::function<bool()> animationsAreLooping,
	std::function<void(bool)> setAnimationLoop
)
{
	if (ImGui::Button(ICON_FA_BACKWARD))
	{
		gotoPrevAnimation();
	}

	ImGui::SameLine();
	if (animationsArePlaying())
	{
		if (ImGui::Button(ICON_FA_PAUSE)) { setPlayAnimation(false); }
	}
	else
	{
		if (ImGui::Button(ICON_FA_PLAY)) { setPlayAnimation(true); }
	}

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_STOP))
	{
		setPlayAnimation(false);
		setAnimationTime(0.0f);
	}

	float animationTimeFactor = getAnimationTimeFactor();
	ImGui::SameLine();
	if (animationTimeFactor > 0.0f)
	{
		if (ImGui::Button(ICON_FA_UNDO))
		{
			setAnimationTimeFactor(-animationTimeFactor);
		}
	}
	else if (animationTimeFactor < 0.0f)
	{
		if (ImGui::Button(ICON_FA_REDO))
		{
			setAnimationTimeFactor(-animationTimeFactor);
		}
	}
	else
	{
		if (ImGui::Button(ICON_FA_SYNC))
		{
			setAnimationTimeFactor(1.0f);
		}
	}

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_FORWARD))
	{
		gotoNextAnimation();
	}

	ImGui::SameLine();
	auto loop = animationsAreLooping();
	if (loop)
	{
		if (ImGui::Button(ICON_FA_PAUSE_CIRCLE))
		{
			setAnimationLoop(false);
		}
	}
	else
	{
		if (ImGui::Button(ICON_FA_CIRCLE))
		{
			setAnimationLoop(true);
		}
	}
}

inline void drawAudioController(
	std::function<bool()> isPlaying,
	std::function<bool()> isPaused,
	std::function<void()> play,
	std::function<void()> stop,
	std::function<void()> pause,
	std::function<float()> getTime,
	std::function<float()> getDuration
)
{
	if (!isPlaying())
	{
		if (ImGui::Button(ICON_FA_PLAY))
		{
			play();
		}
	}
	else
	{
		if (ImGui::Button(ICON_FA_PAUSE))
		{
			pause();
		}
	}

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_STOP))
	{
		stop();
	}

	std::ostringstream ossTime;
	ossTime << std::fixed << std::setprecision(2) << getTime();
	std::string roundedTimeStr = ossTime.str();
	std::ostringstream ossDuration;
	ossDuration << std::fixed << std::setprecision(2) << getDuration();
	std::string roundedDurationStr = ossDuration.str();

	ImGui::SameLine();
	std::string timeStr = roundedTimeStr + "s / " + roundedDurationStr + "s";
	ImGui::Text(timeStr.c_str());
}

#include <JTypes.h>
#include <Functions/JEdvEditorDrawer.h>
#include <Functions/JEdvCreatorDrawer.h>
#include <Functions/JEdvCreatorValidator.h>
