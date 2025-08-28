#pragma once

namespace Scene::Level
{
	void SetLevelToLoad(std::string levelName);
	bool PendingLevelToLoad();
	void LoadPendingLevel();

	//level handling
#if defined(_EDITOR)
	void LoadDefaultLevel();
#endif
	void LoadLevel(std::filesystem::path level);

	//destroy scene objects
	void DestroySceneObjects();

};

