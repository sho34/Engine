#pragma once
#include <memory>
#include <string>
#include <Audio.h>
#include <Application.h>
#include <nlohmann/json.hpp>
#include <Sound/SoundFX.h>
#include <UUID.h>
#include <JTemplate.h>
#include <Templates.h>
#include <TemplateDecl.h>
#include <JExposeTypes.h>

using namespace DirectX;

namespace Templates
{
#include <JExposeAttOrder.h>
#include <SoundAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttDrawersDecl.h>
#include <SoundAtt.h>
#include <JExposeEnd.h>

	struct SoundJson : JTemplate
	{
		TEMPLATE_DECL(Sound);

#include <JExposeAttFlags.h>
#include <SoundAtt.h>
#include <JExposeEnd.h>

#include <JExposeDecl.h>
#include <SoundAtt.h>
#include <JExposeEnd.h>
	};

	TEMPDECL_FULL(Sound);

#if defined(_EDITOR)
	enum SoundPopupModal
	{
		SoundPopupModal_CannotDelete = 1,
		SoundPopupModal_CreateNew = 2
	};
#endif

	namespace Sound
	{
		inline static const std::string templateName = "sounds.json";
	}

	void ReleaseSoundEffectsInstances();

	//EDITOR
#if defined(_EDITOR)
	void CreateNewSound();
	void DeleteSound(std::string uuid);
	void DrawSoundsPopups();
	void WriteSoundsJson(nlohmann::json& json);
#endif

	std::tuple<
		std::unique_ptr<DirectX::SoundEffect>,
		std::unique_ptr<DirectX::SoundEffectInstance>
	> GetSoundEffectInstance(std::string uuid, unsigned int flags);

	void DestroySoundEffectInstance(std::string uuid, std::tuple<
		std::unique_ptr<DirectX::SoundEffect>,
		std::unique_ptr<DirectX::SoundEffectInstance>
	>& soundEffectInstance);

};

