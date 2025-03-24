#include "pch.h"
#include "SoundEffect.h"
#include "../Scene.h"
#include "../../Templates/Templates.h"

#if defined(_EDITOR)
namespace Editor {
	extern _Templates tempTab;
	extern std::string selTemp;
}
#endif

using namespace Templates;
namespace Scene {

	std::map<std::string, std::shared_ptr<SoundEffect>> soundsEffects;
	std::vector<std::shared_ptr<SoundEffect>> sounds3DEffects;

	//CREATE
	std::shared_ptr<SoundEffect> CreateSoundEffect(nlohmann::json soundj)
	{
		std::shared_ptr<SoundEffect> fx = std::make_shared<SoundEffect>();
		fx->this_ptr = fx;
		fx->json = soundj;

		SetIfMissingJson(fx->json, "sound", "");
		SetIfMissingJson(fx->json, "volume", 1.0f);
		SetIfMissingJson(fx->json, "autoPlay", false);
		SetIfMissingJson(fx->json, "instanceFlags", static_cast<int>(SoundEffectInstance_Default));
		SetIfMissingJson(fx->json, "position", XMFLOAT3({ 0.0f,0.0f,0.0f }));

		fx->CreateSoundEffectInstance();

		soundsEffects.insert_or_assign(fx->name(), fx);
		if (nostd::bytesHas(fx->instanceFlags(), SoundEffectInstance_Use3D))
		{
			sounds3DEffects.push_back(fx);
		}

#if defined(_EDITOR)
		Templates::BindNotifications(fx->sound(), fx->this_ptr);
#endif

		return fx;
	}

	std::string SoundEffect::name()
	{
		return json.at("name");
	}

	void SoundEffect::name(std::string name)
	{
		json.at("name") = name;
	}

	std::string SoundEffect::sound()
	{
		return json.at("sound");
	}

	void SoundEffect::sound(std::string sound)
	{
		json.at("sound") = sound;
	}

	float SoundEffect::volume()
	{
		return json.at("volume");
	}

	void SoundEffect::volume(float volume)
	{
		json.at("volume") = volume;
	}

	bool SoundEffect::autoPlay()
	{
		return json.at("autoPlay");
	}

	void SoundEffect::autoPlay(bool autoPlay)
	{
		json.at("volume") = autoPlay;
	}

	XMFLOAT3 SoundEffect::position()
	{
		return XMFLOAT3(json.at("position").at(0), json.at("position").at(1), json.at("position").at(2));
	}

	void SoundEffect::position(XMFLOAT3 f3)
	{
		nlohmann::json& j = json.at("position");
		j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
	}

	void SoundEffect::position(nlohmann::json f3)
	{
		json.at("position") = f3;
	}

	SOUND_EFFECT_INSTANCE_FLAGS SoundEffect::instanceFlags()
	{
		return static_cast<SOUND_EFFECT_INSTANCE_FLAGS>(json.at("instanceFlags"));
	}

	void SoundEffect::instanceFlags(SOUND_EFFECT_INSTANCE_FLAGS instanceFlags)
	{
		json.at("instanceFlags") = static_cast<int>(instanceFlags);
	}

	void SoundEffect::CreateSoundEffectInstance()
	{
		soundEffectInstance = GetSoundEffectInstance(sound(), instanceFlags());
		soundEffectInstance->SetVolume(volume());
		if (autoPlay()) soundEffectInstance->Play(true);

		if (nostd::bytesHas(instanceFlags(), SoundEffectInstance_Use3D))
		{
			audioEmitter.SetPosition(position());
		}
	}

	void SoundEffect::DestroySoundEffectInstance()
	{
		soundEffectInstance->Stop(true);
		soundEffectInstance = nullptr;
	}

	//READ&GET
	std::map<std::string, std::shared_ptr<SoundEffect>> GetSoundsEffects()
	{
		return soundsEffects;
	}

	std::vector<std::shared_ptr<SoundEffect>> Get3DSoundsEffects()
	{
		return sounds3DEffects;
	}

	std::vector<std::string> GetSoundEffectsNames()
	{
		return nostd::GetKeysFromMap(soundsEffects);
	}

#if defined(_EDITOR)
	/*
	nlohmann::json SoundEffect::json() {
		nlohmann::json soundj = {
			{ "name", name },
			{ "sound", soundTemplate->name },
			{ "volume", volume },
			{ "autoPlay", autoPlay },
			{ "position", { position.x, position.y, position.z } },
			{ "instanceFlags", soundEffectInstanceFlagToString[instanceFlags] }
		};
		return soundj;
	}
	*/

	void SelectSoundEffect(std::string soundEffectName, void*& ptr) {
		ptr = soundsEffects.at(soundEffectName).get();
	}

	void DeSelectSoundEffect(void*& ptr)
	{
		ptr = nullptr;
	}

	void DrawSoundEffectPanel(void*& ptr, ImVec2 pos, ImVec2 size, bool pop)
	{
		SoundEffect* fx = (SoundEffect*)ptr;

		std::string tableName = "sound-effect-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			fx->DrawEditorInformationAttributes();
			if (nostd::bytesHas(fx->instanceFlags(), SoundEffectInstance_Use3D))
			{
				fx->DrawEditorWorldAttributes();
			}
		}
		fx->DrawEditorSoundAttributes();
		ImGui::EndTable();
	}

	std::string GetSoundEffectName(void* ptr)
	{
		SoundEffect* soundEffect = (SoundEffect*)ptr;
		return soundEffect->name();
	}

	void DeleteSoundEffect(std::string name)
	{
		std::shared_ptr<SoundEffect> soundEffect = soundsEffects.at(name);
		soundEffect->soundEffectUpdateFlags |= SoundEffectFlags_Destroy;
	}

	void SoundEffectsStep()
	{
#if defined(_EDITOR)
		std::map<std::string, std::shared_ptr<SoundEffect>> soundEffectsToDestroy;
		std::copy_if(soundsEffects.begin(), soundsEffects.end(), std::inserter(soundEffectsToDestroy, soundEffectsToDestroy.end()), [](const auto& pair)
			{
				return pair.second->soundEffectUpdateFlags & SoundEffectFlags_Destroy;
			}
		);

		if (soundEffectsToDestroy.size() > 0ULL)
		{
			for (auto& [name, soundEffect] : soundEffectsToDestroy)
			{
				soundEffect->DestroySoundEffectInstance();
				soundsEffects.erase(name);
				nostd::vector_erase(sounds3DEffects, soundEffect);
			}
		}
#endif
	}

	void SoundEffect::DrawEditorInformationAttributes()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "sound-effect-information-atts";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string currentName = name();
			if (ImGui::InputText("name", &currentName))
			{
				if (!soundsEffects.contains(currentName))
				{
					soundsEffects[currentName] = soundsEffects.at(name());
					soundsEffects.erase(name());
				}
				name(currentName);
			}
			ImGui::EndTable();
		}
	}

	void SoundEffect::DrawEditorWorldAttributes()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		XMFLOAT3 posV = position();
		ImDrawFloatValues<XMFLOAT3>("sound-effect-position", { "x","y","z" }, posV, [this](XMFLOAT3 pos)
			{
				position(pos);
				audioEmitter.SetPosition(pos);
			}
		);
	}

	void SoundEffect::DrawEditorSoundAttributes()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		ImGui::PushID("sound-effect-template");
		{
			std::vector<std::string> selectables = GetSoundsNames();
			std::string selected = sound();

			if (!selected.empty())
			{
				if (ImGui::Button(ICON_FA_FILE_AUDIO))
				{
					Editor::tempTab = T_Sounds;
					Editor::selTemp = selected;
				}
				ImGui::SameLine();
			}

			DrawComboSelection(selected, selectables, [this, selected](std::string sound)
				{
					Templates::UnbindNotifications(selected, this_ptr);
					DestroySoundEffectInstance();
					this->sound(sound);
					CreateSoundEffectInstance();
					Templates::BindNotifications(sound, this_ptr);
				}
			);
		}
		ImGui::PopID();

		ImGui::PushID("sound-effect-instance-flag");
		{
			std::vector<std::string> selectables = nostd::GetKeysFromMap(strToSoundEffectInstanceFlags);
			std::string selected = soundEffectInstanceFlagsToStr.at(instanceFlags());
			DrawComboSelection(selected, selectables, [this](std::string soundEffectInstanceFlag)
				{
					SOUND_EFFECT_INSTANCE_FLAGS newSoundInstanceFlags = strToSoundEffectInstanceFlags.at(soundEffectInstanceFlag);
					DestroySoundEffectInstance();
					if (nostd::bytesHas(instanceFlags(), SoundEffectInstance_Use3D) && !nostd::bytesHas(newSoundInstanceFlags, SoundEffectInstance_Use3D))
					{
						nostd::vector_erase(sounds3DEffects, this_ptr);
					}
					else if (!nostd::bytesHas(instanceFlags(), SoundEffectInstance_Use3D) && nostd::bytesHas(newSoundInstanceFlags, SoundEffectInstance_Use3D))
					{
						sounds3DEffects.push_back(this_ptr);
					}
					instanceFlags(newSoundInstanceFlags);
					CreateSoundEffectInstance();
				}
			);
		}
		ImGui::PopID();

		std::string tableName = "sound-effect-sound-atts";
		if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::PushID("volume");
			{
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("volume");
				ImGui::TableSetColumnIndex(1);
				float vol = volume();
				if (ImGui::InputFloat("", &vol))
				{
					volume(vol);
					soundEffectInstance->SetVolume(vol);
				}
			}
			ImGui::PopID();

			ImGui::EndTable();
		}
	}
#endif

	void SoundEffect::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox)
	{
		bbox->position(position());
		bbox->scale(XMFLOAT3({ 0.3f, 0.3f, 0.3f }));
		bbox->rotation(XMFLOAT3({ 0.0f, 0.0f, 0.0f }));
	}

	void DestroySoundEffects()
	{
		for (auto& [name, fx] : soundsEffects) {
			fx->DestroySoundEffectInstance();
		}

		soundsEffects.clear();
		sounds3DEffects.clear();
	}
}