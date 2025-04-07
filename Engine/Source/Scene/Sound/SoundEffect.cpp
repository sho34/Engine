#include "pch.h"
#include "SoundEffect.h"
#include "../Scene.h"
#include "../../Templates/Templates.h"
#include "../../Editor/Editor.h"
#include <Editor.h>
#include <NoStd.h>
#include <UUID.h>

#if defined(_EDITOR)
namespace Editor {
	extern _Templates tempTab;
	extern std::string selTemp;
}
#endif

using namespace Templates;
namespace Scene {

	std::map<std::string, std::shared_ptr<SoundEffect>> soundsEffects; //uuid -> SoundEffect
	std::vector<std::shared_ptr<SoundEffect>> sounds3DEffects;

#if defined(_EDITOR)
	nlohmann::json SoundEffect::creationJson;
	unsigned int SoundEffect::popupModalId = 0U;
#endif

	//CREATE
	std::shared_ptr<SoundEffect> CreateSoundEffect(nlohmann::json soundj)
	{
		std::shared_ptr<SoundEffect> fx = std::make_shared<SoundEffect>();
		fx->this_ptr = fx;
		fx->json = soundj;

		SetIfMissingJson(fx->json, "sound", "");
		SetIfMissingJson(fx->json, "volume", 1.0f);
		SetIfMissingJson(fx->json, "autoPlay", false);
		SetIfMissingJson(fx->json, "loop", false);
		SetIfMissingJson(fx->json, "instanceFlags", static_cast<int>(SoundEffectInstance_Default));
		SetIfMissingJson(fx->json, "position", XMFLOAT3({ 0.0f,0.0f,0.0f }));

		soundsEffects.insert_or_assign(fx->uuid(), fx);

		if (!fx->json.at("sound").empty())
		{
			fx->CreateSoundEffectInstance();

			if (nostd::bytesHas(fx->instanceFlags(), SoundEffectInstance_Use3D))
			{
				sounds3DEffects.push_back(fx);
			}

#if defined(_EDITOR)
			Templates::BindNotifications(fx->sound(), fx->this_ptr);
#endif
		}

		return fx;
	}

	std::string SoundEffect::uuid()
	{
		return json.at("uuid");
	}

	void SoundEffect::uuid(std::string uuid)
	{
		json.at("uuid") = uuid;
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
		json.at("autoPlay") = autoPlay;
	}

	bool SoundEffect::loop()
	{
		return json.at("loop");
	}

	void SoundEffect::loop(bool loop)
	{
		json.at("loop") = loop;
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

	void SoundEffect::DetachSoundEffectTemplate()
	{
		sound("");
	}

	void SoundEffect::CreateSoundEffectInstance()
	{
		soundEffectInstance = GetSoundEffectInstance(sound(), instanceFlags());
		soundEffectInstance->SetVolume(volume());
		if (autoPlay()) soundEffectInstance->Play(loop());

		if (nostd::bytesHas(instanceFlags(), SoundEffectInstance_Use3D))
		{
			audioEmitter.SetPosition(position());
		}
	}

	void SoundEffect::DestroySoundEffectInstance()
	{
		if (soundEffectInstance)
		{
			soundEffectInstance->Stop(true);
			soundEffectInstance = nullptr;
		}
	}

	//READ&GET
	std::shared_ptr<SoundEffect> GetSoundEffect(std::string uuid)
	{
		return soundsEffects.at(uuid);
	}

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

	std::vector<UUIDName> GetSoundEffectsUUIDNames()
	{
		return GetSceneObjectsUUIDsNames(soundsEffects);
	}

#if defined(_EDITOR)

	void SelectSoundEffect(std::string uuid, std::string& edSO)
	{
		edSO = uuid;
	}

	void DeSelectSoundEffect(std::string& edSO)
	{
		edSO = "";
	}

	void DrawSoundEffectPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop)
	{
		std::shared_ptr<SoundEffect> fx = soundsEffects.at(uuid);

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

	std::string GetSoundEffectName(std::string uuid)
	{
		std::shared_ptr<SoundEffect> fx = soundsEffects.at(uuid);
		return fx->name();
	}

	void CreateNewSoundEffect()
	{
		SoundEffect::popupModalId = SoundEffectPopupModal_CreateNew;
		SoundEffect::creationJson = R"(
		{
			"name": "",
			"sound":""
		})"_json;
	}

	void DeleteSoundEffect(std::string uuid)
	{
		std::shared_ptr<SoundEffect> soundEffect = soundsEffects.at(uuid);
		soundEffect->soundEffectUpdateFlags |= SoundEffectFlags_Destroy;
	}

	void DrawSoundEffectsPopups()
	{
		Editor::DrawCreateWindow(SoundEffect::popupModalId, SoundEffectPopupModal_CreateNew, "New SoundEffect", [](auto OnCancel)
			{
				ImGui::PushID("sfx-name");
				{
					ImGui::Text("Name");
					ImDrawJsonInputText(SoundEffect::creationJson, "name");
				}
				ImGui::PopID();

				std::vector<UUIDName> soundsUUIDNames = GetSoundsUUIDsNames();
				std::vector<UUIDName> selectables = { std::tie("", " ") };
				nostd::AppendToVector(selectables, soundsUUIDNames);

				int current_item = FindSelectableIndex(selectables, SoundEffect::creationJson, "sound");

				ImGui::PushID("sfx-sound");
				{
					ImGui::Text("Sound");
					DrawComboSelection(selectables[current_item], selectables, [](UUIDName uuidName)
						{
							SoundEffect::creationJson.at("sound") = std::get<0>(uuidName);
						}, "sound"
					);
				}
				ImGui::PopID();

				if (ImGui::Button("Cancel")) { OnCancel(); }
				ImGui::SameLine();
				bool disabledCreate = (
					current_item == 0 ||
					SoundEffect::creationJson.at("name") == "" ||
					NameCollideWithSceneObjects(soundsEffects, SoundEffect::creationJson)
					);

				if (disabledCreate)
				{
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}

				if (ImGui::Button("Create"))
				{
					SoundEffect::popupModalId = 0;
					nlohmann::json r = {
						{ "uuid", getUUID() },
						{ "name", SoundEffect::creationJson.at("name") },
						{ "sound", SoundEffect::creationJson.at("sound") }
					};
					CreateSoundEffect(r);
				}

				if (disabledCreate)
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}
			}
		);
	}

	void WriteSoundEffectsJson(nlohmann::json& json)
	{
		std::map<std::string, std::shared_ptr<SoundEffect>> filtered;
		std::copy_if(soundsEffects.begin(), soundsEffects.end(), std::inserter(filtered, filtered.end()), [](const auto& pair)
			{
				auto& [uuid, sfx] = pair;
				return !sfx->hidden();
			}
		);
		std::transform(filtered.begin(), filtered.end(), std::back_inserter(json), [](const auto& pair)
			{
				auto& [uuid, sfx] = pair;
				nlohmann::json ret = sfx->json;
				ret["uuid"] = uuid;
				return ret;
			}
		);
	}
#endif

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
			for (auto& [uuid, soundEffect] : soundEffectsToDestroy)
			{
				soundEffect->DestroySoundEffectInstance();
				soundsEffects.erase(uuid);
				nostd::vector_erase(sounds3DEffects, soundEffect);
			}
		}
#endif
	}

#if defined(_EDITOR)
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
			std::vector<UUIDName> selectables = { std::tie(" ", " ") };
			std::vector<UUIDName> soundUUIDNames = GetSoundsUUIDsNames();
			nostd::AppendToVector(selectables, soundUUIDNames);

			UUIDName selected;
			std::string& soundUUID = std::get<0>(selected);
			std::string& soundName = std::get<1>(selected);

			if (sound() != "")
			{
				soundUUID = sound();
				soundName = GetSoundName(soundUUID);
			}
			else
			{
				soundUUID = " ";
				soundName = " ";
			}

			if (!sound().empty())
			{
				if (ImGui::Button(ICON_FA_FILE_AUDIO))
				{
					Editor::tempTab = T_Sounds;
					Editor::selTemp = sound();
				}
				ImGui::SameLine();
			}

			DrawComboSelection(selected, selectables, [this, selected, soundUUID](UUIDName sound)
				{
					if (soundUUID != " ")
					{
						Templates::UnbindNotifications(soundUUID, this_ptr);
						DestroySoundEffectInstance();
						nostd::vector_erase(sounds3DEffects, this_ptr);
					}
					std::string newSoundUUID = std::get<0>(sound);
					this->sound((newSoundUUID == " ") ? "" : newSoundUUID);
					if (newSoundUUID != " ")
					{
						CreateSoundEffectInstance();
						Templates::BindNotifications(newSoundUUID, this_ptr);
					}
				}
			);
		}
		ImGui::PopID();

		ImGui::PushID("sound-effect-control-buttons");
		if (soundEffectInstance->GetState() == SoundState::PLAYING)
		{
			if (ImGui::Button(ICON_FA_PAUSE))
			{
				soundEffectInstance->Pause();
			}
		}
		else
		{
			if (ImGui::Button(ICON_FA_PLAY))
			{
				soundEffectInstance->Play();
			}
		}
		ImGui::SameLine();

		if (ImGui::Button(ICON_FA_STOP))
		{
			soundEffectInstance->Stop();
		}
		ImGui::PopID();

		ImGui::PushID("sound-effect-instance-flag");
		{
			ImGui::Text("Instance Flags");

			SOUND_EFFECT_INSTANCE_FLAGS flags = instanceFlags();
			SOUND_EFFECT_INSTANCE_FLAGS newFlags = flags;
			bool flagsChanged = false;

			std::for_each(soundEffectInstanceFlagsToStr.begin(), soundEffectInstanceFlagsToStr.end(), [flags, &newFlags, &flagsChanged](auto& pair)
				{
					bool value = !!(pair.first & flags);
					if (ImGui::Checkbox(pair.second.c_str(), &value))
					{
						flagsChanged = true;
						if (value)
						{
							newFlags |= pair.first;
						}
						else
						{
							newFlags &= ~pair.first;
						}
					}
				}
			);
			if (flagsChanged)
			{
				instanceFlags(newFlags);
			}
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

			ImGui::TableNextRow();
			ImGui::PushID("autoplay");
			{
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("autoplay");
				ImGui::TableSetColumnIndex(1);
				bool aplay = autoPlay();
				if (ImGui::Checkbox("", &aplay))
				{
					autoPlay(aplay);
					if (aplay)
					{
						soundEffectInstance->Play();
					}
					else
					{
						soundEffectInstance->Stop();
					}
				}
			}
			ImGui::PopID();

			ImGui::TableNextRow();
			ImGui::PushID("looped");
			{
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("looped");
				ImGui::TableSetColumnIndex(1);
				bool looped = loop();
				if (ImGui::Checkbox("", &looped))
				{
					loop(looped);
					soundEffectInstance->Play(looped);
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