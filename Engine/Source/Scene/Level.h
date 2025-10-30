#pragma once

namespace Scene::Level
{
	void SetLevelToLoad(std::string levelName);
#if defined(_EDITOR)
	void SetDefaultLevelToLoad();
#endif
	bool PendingLevelToLoad();
	void LoadPendingLevel();

	//level handling
	void LoadSceneObjects(nlohmann::json& j, std::string type, std::function<void(nlohmann::json&)> loader);
#if defined(_EDITOR)
	void LoadDefaultLevel();
#endif
	void LoadLevel(std::filesystem::path level);

	//destroy scene objects
	void DestroySceneObjects();
};

