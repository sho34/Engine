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
	namespace Sound
	{
		inline static const std::string templateName = "sounds.json";
	}

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

	void SoundJsonStep();
	void ReleaseSoundEffectsInstances();

	//EDITOR
	std::tuple<
		std::unique_ptr<DirectX::SoundEffect>,
		std::unique_ptr<DirectX::SoundEffectInstance>
	> GetSoundEffectInstance(
		std::string uuid,
		unsigned int flags,
		std::string objectUUID = "",
		JObjectChangeCallback cb = nullptr,
		JObjectChangePostCallback postCb = nullptr
	);

	void DestroySoundEffectInstance(std::string uuid, std::tuple<
		std::unique_ptr<DirectX::SoundEffect>,
		std::unique_ptr<DirectX::SoundEffectInstance>
	>& soundEffectInstance);

};

