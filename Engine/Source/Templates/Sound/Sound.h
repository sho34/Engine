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
#include <JTypes.h>

using namespace DirectX;

namespace Templates
{
	namespace Sound
	{
		inline static const std::string templateName = "sounds.json";
	}

#include <Attributes/JOrder.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <SoundAtt.h>
#include <JEnd.h>

	struct SoundJson : JTemplate
	{
		TEMPLATE_DECL(Sound);

#include <Attributes/JFlags.h>
#include <SoundAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <SoundAtt.h>
#include <JEnd.h>
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

