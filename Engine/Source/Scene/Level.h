#pragma once

namespace Scene::Renderable { struct Renderable; }

namespace Scene::Level
{

	bool isLoadingLevel();
	void callLoadLevelFn();

	//level handling
#if defined(_EDITOR)
	void LoadDefaultLevel();
#endif
	void LoadLevel(std::filesystem::path level);
	void LoadScene();
	
	//create scene objects
	void CreateRenderables(nlohmann::json renderables);
	void CreateCameras(nlohmann::json cameras);
	void CreateLights(nlohmann::json lights);
	void CreateSounds(nlohmann::json sounds);
	void CreateUI(nlohmann::json ui);

#if defined(_EDITOR)
	void PushRenderableToReloadQueue(std::shared_ptr<Scene::Renderable::Renderable> renderable);
	bool ReloadQueueIsEmpty();
	void ProcessReloadQueue();
#endif

	//destroy scene objects
	void DestroySceneObjects();

};

