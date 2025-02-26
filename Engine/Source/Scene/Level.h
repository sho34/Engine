#pragma once

namespace Scene::Level
{
	//level handling
#if defined(_EDITOR)
	void LoadDefaultLevel();
#endif
	void LoadLevel(std::filesystem::path level);

	//destroy scene objects
	void DestroySceneObjects();

};

