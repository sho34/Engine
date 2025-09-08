template<typename T, JsonToEditorValueType J>
JEdvCreatorValidatorFunction CreatorValidValue() {
	return[](std::string attribute, nlohmann::json& json) {return true; };
}

template<typename T, JsonToEditorValueType J>
JEdvCreatorValidatorFunction CreatorValidVector() {
	return[](std::string attribute, nlohmann::json& json) {return true; };
}

template<typename Ta, typename Tb>
JEdvCreatorValidatorFunction CreatorValidMap() { return[](std::string attribute, nlohmann::json& json) {return true; }; }

template<typename E, JsonToEditorValueType J>
JEdvCreatorValidatorFunction CreatorValidEnum(
	std::map<E, std::string>& EtoS,
	std::map<std::string, E>& StoE
) {
	return [&EtoS, &StoE](std::string attribute, nlohmann::json& json)
		{
			return true;
		};
}

template<>
inline JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_so_camera_name>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}

template<>
inline JEdvCreatorValidatorFunction CreatorValidVector<std::string, jedv_t_so_camera_vector>()
{
	return [](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute).size() > 0 && json.at(attribute).at(0) != "";
		};
}

template<>
inline JEdvCreatorValidatorFunction CreatorValidValue<std::string, jedv_t_te_sound>() {
	return[](std::string attribute, nlohmann::json& json)
		{
			return json.at(attribute) != "";
		};
}