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
#if defined(_EDITOR)
	void LoadDefaultLevel();
#endif
	void LoadLevel(std::filesystem::path level);

	//destroy scene objects
	void DestroySceneObjects();

};

