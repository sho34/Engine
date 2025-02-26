#include "pch.h"
#include "SoundEffect.h"
#include "../Scene.h"
#include "../../Templates/Sound/Sound.h"

using namespace Templates;
namespace Scene {

	std::map<std::string, std::shared_ptr<SoundEffect>> soundsEffects;
	std::vector<std::shared_ptr<SoundEffect>> sounds3DEffects;

	//CREATE
	std::shared_ptr<SoundEffect> CreateSoundEffect(nlohmann::json soundj)
	{
		std::shared_ptr<SoundEffect> fx = std::make_shared<SoundEffect>();

		ReplaceFromJson(fx->name, soundj, "name");
		ReplaceFromJson(fx->sound, soundj, "sound");
		ReplaceFromJson(fx->volume, soundj, "volume");
		ReplaceFromJson(fx->autoPlay, soundj, "autoPlay");
		nostd::ReplaceFromJsonUsingMap(fx->instanceFlags, stringToSoundEffectInstanceFlag, soundj, "instanceFlags");
		JsonToFloat3(fx->position, soundj, "position");

		fx->this_ptr = fx;
		fx->CreateSoundEffectInstance();

		soundsEffects.insert_or_assign(fx->name, fx);
		if (nostd::bytesHas(fx->instanceFlags, SoundEffectInstance_Use3D))
		{
			sounds3DEffects.push_back(fx);
		}

		return fx;
	}

	void SoundEffect::CreateSoundEffectInstance()
	{
		soundEffectInstance = GetSoundEffectInstance(sound, instanceFlags);
		soundEffectInstance->SetVolume(volume);
		if (autoPlay) soundEffectInstance->Play(true);

		if (nostd::bytesHas(instanceFlags, SoundEffectInstance_Use3D))
		{
			audioEmitter.SetPosition(position);
		}
	}

	void SoundEffect::DestroySoundEffectInstance()
	{
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
			if (nostd::bytesHas(fx->instanceFlags, SoundEffectInstance_Use3D))
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
		return soundEffect->name;
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
			std::string currentName = name;
			if (ImGui::InputText("name", &currentName))
			{
				if (!soundsEffects.contains(currentName))
				{
					soundsEffects[currentName] = soundsEffects[name];
					soundsEffects.erase(name);
				}
				name = currentName;
			}
			ImGui::EndTable();
		}
	}

	void SoundEffect::DrawEditorWorldAttributes()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "sound-effect-world-atts";
		if (ImGui::BeginTable(tableName.c_str(), 4, ImGuiTableFlags_NoSavedSettings))
		{
			bool updatePos = false;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::PushID("position");
			ImGui::Text("position");
			ImGui::TableSetColumnIndex(1);
			if (ImGui::InputFloat("x", &position.x)) { updatePos = true; }
			ImGui::TableSetColumnIndex(2);
			if (ImGui::InputFloat("y", &position.y)) { updatePos = true; }
			ImGui::TableSetColumnIndex(3);
			if (ImGui::InputFloat("z", &position.z)) { updatePos = true; }
			ImGui::PopID();

			if (updatePos) { audioEmitter.SetPosition(position); }

			ImGui::EndTable();
		}
	}

	void SoundEffect::DrawEditorSoundAttributes()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		std::vector<std::string> selectables = nostd::GetKeysFromMap(strToSoundEffectInstanceFlags);
		std::string selected = soundEffectInstanceFlagsToStr.at(instanceFlags);
		DrawComboSelection(selected, selectables, [this](std::string soundEffectInstanceFlag)
			{
				SOUND_EFFECT_INSTANCE_FLAGS newSoundInstanceFlags = strToSoundEffectInstanceFlags.at(soundEffectInstanceFlag);
				DestroySoundEffectInstance();
				if (nostd::bytesHas(instanceFlags, SoundEffectInstance_Use3D) && !nostd::bytesHas(newSoundInstanceFlags, SoundEffectInstance_Use3D))
				{
					nostd::vector_erase(sounds3DEffects, this_ptr);
				}
				else if (!nostd::bytesHas(instanceFlags, SoundEffectInstance_Use3D) && nostd::bytesHas(newSoundInstanceFlags, SoundEffectInstance_Use3D))
				{
					sounds3DEffects.push_back(this_ptr);
				}
				instanceFlags = newSoundInstanceFlags;
				CreateSoundEffectInstance();
			}
		);
		std::string tableName = "sound-effect-sound-atts";
		if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::PushID("volume");
			ImGui::Text("volume");
			ImGui::TableSetColumnIndex(1);
			if (ImGui::InputFloat("", &volume))
			{
				soundEffectInstance->SetVolume(volume);
			}
			ImGui::PopID();

			ImGui::EndTable();
		}
	}
#endif

	void SoundEffect::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox)
	{
		bbox->position = position;
		bbox->scale = { 0.3f, 0.3f, 0.3f };
		bbox->rotation = { 0.0f, 0.0f, 0.0f };
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