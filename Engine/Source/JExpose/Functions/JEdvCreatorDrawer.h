
template<typename T, JsonToEditorValueType J>
JEdvCreatorDrawerFunction DrawCreatorValue() { return nullptr; }

template<typename T, JsonToEditorValueType J>
JEdvCreatorDrawerFunction DrawCreatorVector() { return nullptr; }

template<typename Ta, typename Tb>
JEdvCreatorDrawerFunction DrawCreatorMap() { return nullptr; }

template<typename E, JsonToEditorValueType J>
JEdvCreatorDrawerFunction DrawCreatorEnum(
	std::map<E, std::string>& EtoS,
	std::map<std::string, E>& StoE
) {
	if (J == jedv_t_hidden) return nullptr;
	return [&EtoS, &StoE](std::string attribute, nlohmann::json& json)
		{
			auto update = [attribute, &json](auto value)
				{
					nlohmann::json patch = { {attribute,value} };
					json.merge_patch(patch);
				};

			std::string selected = json.at(attribute);
			std::vector<std::string> options;
			std::transform(StoE.begin(), StoE.end(), std::back_inserter(options), [](auto& p) { return p.first; });

			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				ImGui::DrawComboSelection(selected, options, [update](std::string newOption)
					{
						update(newOption);
					}
				);
			}
			ImGui::PopID();
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_string>()
{
	return[](std::string attribute, nlohmann::json& json)
		{
			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				ImGui::DrawJsonInputText(json, attribute);
			}
			ImGui::PopID();
		};
}

inline JEdvCreatorDrawerFunction DrawUniqueName(std::string objectName, auto getNames)
{
	return[objectName, getNames](std::string attribute, nlohmann::json& json)
		{
			auto names = getNames();
			std::set<std::string> namesSet(names.begin(), names.end());
			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				ImGui::DrawJsonInputText(json, attribute);
			}
			ImGui::PopID();
			if (namesSet.contains(json.at(attribute)))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
				std::string text = "*";
				text += objectName;
				text += " with name ";
				text += json.at(attribute);
				text += " Already exists";
				ImGui::Text(text.c_str());
				ImGui::PopStyleColor();
			}
		};
}

template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_camera_name>() { return DrawUniqueName("Camera", GetCamerasNames); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_light_name>() { return DrawUniqueName("Light", GetLightsNames); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_renderable_name>() { return DrawUniqueName("Renderable", GetRenderablesNames); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_so_soundeffect_name>() { return DrawUniqueName("SoundEffects", GetSoundEffectsNames); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_material_name>() { return DrawCreatorValue<std::string, jedv_t_string>(); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_model3d_name>() { return DrawCreatorValue<std::string, jedv_t_string>(); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_renderpass_name>() { return DrawCreatorValue<std::string, jedv_t_string>(); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_shader_name>() { return DrawCreatorValue<std::string, jedv_t_string>(); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_sound_name>() { return DrawCreatorValue<std::string, jedv_t_string>(); }
template<>inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_texture_name>() { return DrawCreatorValue<std::string, jedv_t_string>(); }

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<bool, jedv_t_boolean>()
{
	return[](std::string attribute, nlohmann::json& json)
		{
			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				ImGui::DrawJsonCheckBox(json, attribute);
			}
			ImGui::PopID();
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<unsigned int, jedv_t_sound_instance_flags>()
{
	return [](std::string attribute, nlohmann::json& json)
		{
			unsigned int currentValue = json.at(attribute);

			for (auto [flag, title] : SOUND_EFFECT_INSTANCE_FLAGSToString)
			{
				bool value = !!(flag & currentValue);
				ImGui::Text(title.c_str());
				ImGui::PushID(title.c_str());
				{
					if (ImGui::Checkbox("##", &value))
					{
						if (value)
							currentValue |= flag;
						else
							currentValue &= ~flag;
					}
				}
				ImGui::PopID();
			}

			json.at(attribute) = currentValue;
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorValue<std::string, jedv_t_te_sound>()
{
	return [](std::string attribute, nlohmann::json& json)
		{
			std::vector<UUIDName> selectables;
			UUIDName selected = std::make_tuple("", "");
			selectables.push_back(selected);

			if (json.at(attribute) != "")
			{
				std::string& uuid = std::get<0>(selected);
				std::string& name = std::get<1>(selected);
				uuid = json.at(attribute);
				name = Templates::GetSoundName(uuid);
			}
			std::vector<UUIDName> resources = Templates::GetSoundsUUIDsNames();
			nostd::AppendToVector(selectables, resources);

			ImGui::Text(attribute.c_str());

			ImGui::PushID(attribute.c_str());
			{
				ImGui::DrawComboSelection(selected, selectables, [attribute, &json](UUIDName option)
					{
						json.at(attribute) = std::get<0>(option);
					}
				);
			}
			ImGui::PopID();
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorVector<MeshMaterial, jedv_t_vector>()
{
	return [](std::string attribute, nlohmann::json& json)
		{
			unsigned int size = static_cast<unsigned int>(json.at("meshMaterials").size());
			bool hasModel = json.contains("model") && json.at("model") != "";

			std::vector<UUIDName> selectablesMeshes = { std::make_tuple("","") };
			std::vector<UUIDName> selectablesMaterials = { std::make_tuple("","") };
			std::vector<UUIDName> meshesUUIDs = Templates::GetMeshesUUIDsNames();
			std::vector<UUIDName> materialsUUIDs = Templates::GetMaterialsUUIDsNames();

			selectablesMeshes.insert(selectablesMeshes.end(), meshesUUIDs.begin(), meshesUUIDs.end());
			selectablesMaterials.insert(selectablesMaterials.end(), materialsUUIDs.begin(), materialsUUIDs.end());

			auto clearMeshMaterial = [&json]
				{
					json.at("meshMaterials").clear();
				};
			auto eraseModel = [&json]
				{
					json.erase("model");
				};
			auto setMesh = [&json, eraseModel](unsigned int index, std::string uuid)
				{
					json.at("meshMaterials").at(index)["mesh"] = uuid;
					if (json.contains("model")) json.erase("model");
					eraseModel();
				};
			auto setMaterial = [&json, eraseModel](unsigned int index, std::string uuid)
				{
					json.at("meshMaterials").at(index)["material"] = uuid;
					if (json.contains("model")) json.erase("model");
					eraseModel();
				};
			auto setModel = [&json, clearMeshMaterial](std::string uuid)
				{
					json["model"] = uuid;
					clearMeshMaterial();
				};
			auto drawRow = [setMesh, setMaterial, meshesUUIDs, materialsUUIDs, selectablesMeshes, selectablesMaterials, &json](unsigned int index)
				{
					std::string mesh = json.at("meshMaterials").at(index).at("mesh");
					std::string material = json.at("meshMaterials").at(index).at("material");
					std::string meshName = mesh.empty() ? "" : Templates::GetMeshName(mesh);
					std::string materialName = material.empty() ? "" : Templates::GetMaterialName(material);

					UUIDName meshUN = std::make_tuple(mesh, meshName);
					UUIDName matUN = std::make_tuple(material, materialName);

					ImGui::PushID((std::string("mesh-") + std::to_string(index)).c_str());
					ImGui::DrawComboSelection(meshUN, meshesUUIDs, [index, setMesh](UUIDName option)
						{
							std::string& nuuid = std::get<0>(option);
							setMesh(index, nuuid);
						}
					);
					ImGui::PopID();

					ImGui::SameLine();
					ImGui::PushID((std::string("material-") + std::to_string(index)).c_str());
					ImGui::DrawComboSelection(matUN, materialsUUIDs, [index, setMaterial](UUIDName option)
						{
							std::string& nuuid = std::get<0>(option);
							setMaterial(index, nuuid);
						}
					);
					ImGui::PopID();

				};
			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				for (unsigned int i = 0; i < size; i++)
				{
					drawRow(i);
				}

				if (ImGui::Button(ICON_FA_PLUS, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f)))
				{
					json.at("meshMaterials").push_back({
							{ "mesh", "" },
							{ "material", "" },
						}
						);
				}

				ImGui::Text("model");
				std::vector<UUIDName> selectablesModels = { std::make_tuple("","") };
				std::vector<UUIDName> model3DsUUIDs = Templates::GetModel3DsUUIDsNames();
				selectablesModels.insert(selectablesModels.end(), model3DsUUIDs.begin(), model3DsUUIDs.end());

				std::string model = json.contains("model") ? json.at("model") : "";
				std::string modelName = model.empty() ? "" : Templates::GetModel3DName(model);

				UUIDName modelUN = std::make_tuple(model, modelName);

				ImGui::PushID(std::string("model").c_str());
				ImGui::DrawComboSelection(modelUN, model3DsUUIDs, [setModel](UUIDName option)
					{
						std::string& nuuid = std::get<0>(option);
						setModel(nuuid);
					}
				);
				ImGui::PopID();

			}
			ImGui::PopID();
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorVector<std::string, jedv_t_so_camera_vector>()
{
	return [](std::string attribute, nlohmann::json& json)
		{
			unsigned int size = static_cast<unsigned int>(json.at(attribute).size());
			std::set<std::string> currentUUIDs;
			for (unsigned int i = 0; i < size; i++)
			{
				currentUUIDs.insert(json.at(attribute).at(i));
			}
			std::vector<UUIDName> camsUUIDNames = GetCamerasUUIDNames();

			auto setCamera = [&json, attribute](unsigned int index, std::string uuid)
				{
					json.at(attribute)[index] = uuid;
				};

			auto drawRow = [&json, attribute, &camsUUIDNames, &currentUUIDs, setCamera](unsigned int index)
				{
					std::string uuid = json.at(attribute).at(index);
					std::vector<UUIDName> selectablesCams = { std::make_tuple("","") };
					std::copy_if(camsUUIDNames.begin(), camsUUIDNames.end(), std::back_inserter(selectablesCams), [uuid, &currentUUIDs](UUIDName cam)
						{
							std::string camUUID = std::get<0>(cam);
							return !currentUUIDs.contains(camUUID) || uuid == camUUID;
						}
					);
					UUIDName selected = std::make_tuple("", "");
					if (uuid != "")
					{
						for (unsigned int i = 0; i < selectablesCams.size(); i++)
						{
							if (std::get<0>(selectablesCams[i]) == uuid)
							{
								selected = selectablesCams[i];
							}
						}
					}
					ImGui::PushID((std::string("camera-") + std::to_string(index)).c_str());
					ImGui::DrawComboSelection(selected, selectablesCams, [index, setCamera](UUIDName option)
						{
							std::string& nuuid = std::get<0>(option);
							setCamera(index, nuuid);
						}
					);
					ImGui::PopID();
				};

			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());

				for (unsigned int i = 0; i < size; i++)
				{
					drawRow(i);
				}
			}
			ImGui::PopID();
			if (json.at(attribute).size() < camsUUIDNames.size())
			{
				if (ImGui::Button(ICON_FA_PLUS, ImVec2(ImGui::GetContentRegionAvail().x, 20.0f)))
				{
					json.at(attribute).push_back("");
				}
			}
		};
}

template<>
inline JEdvCreatorDrawerFunction DrawCreatorEnum<LightType, jedv_t_lighttype>(
	std::map<LightType, std::string>& EtoS,
	std::map<std::string, LightType>& StoE
) {
	return [&EtoS, &StoE](std::string attribute, nlohmann::json& json)
		{
			DrawCreatorValue<std::string, jedv_t_so_light_name>()("name", json);

			auto update = [attribute, &json](auto value)
				{
					nlohmann::json patch = { {attribute,value} };
					json.merge_patch(patch);
				};
			std::string selected = json.at(attribute);
			std::vector<std::string> options;
			std::transform(StoE.begin(), StoE.end(), std::back_inserter(options), [](auto& p) { return p.first; });

			ImGui::PushID(attribute.c_str());
			{
				ImGui::Text(attribute.c_str());
				ImGui::DrawComboSelection(selected, options, [update](std::string newOption)
					{
						update(newOption);
					}
				);
			}
			ImGui::PopID();

			float color[3];
			for (unsigned int i = 0; i < 3; i++)
			{
				color[i] = json.at("color").at(i);
			}
			ImGui::PushID("color");
			{
				ImGui::Text("color");
				if (ImGui::ColorEdit3("##", color))
				{
					nlohmann::json patch = { {"color",nlohmann::json::array({color[0],color[1],color[2]})} };
					json.merge_patch(patch);
				}
			}
			ImGui::PopID();
		};
}

