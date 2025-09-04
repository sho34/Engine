#include "pch.h"
#include <Level.h>
//scene objects
#include <Scene.h>
#include <Camera/Camera.h>
#include <Renderable/Renderable.h>
#include <Lights/Lights.h>
#include <Sound/SoundFX.h>
//audio
#include <AudioSystem.h>
using namespace AudioSystem;
//effects
#include <Effects.h>
//#include <DecalLoop.h>
//#include <LightOscilation.h>
#include <Renderer.h>
#if defined(_EDITOR)
#include <Editor.h>
#include <DefaultLevel.h>

using namespace Editor;
namespace Editor {
	extern std::string currentLevelName;
}
#endif

extern std::shared_ptr<Renderer> renderer;

namespace Scene::Level {
	using namespace Scene;

	std::filesystem::path levelToLoad;

	void SetLevelToLoad(std::string levelName)
	{
		levelToLoad = levelName;
	}

	bool PendingLevelToLoad()
	{
		return !levelToLoad.empty();
	}

	void LoadPendingLevel()
	{
		DestroyEditorSceneObjectsReferences();
		DestroySceneObjects();
		LoadLevel(levelToLoad);
		levelToLoad = "";
	}

	template<typename T>
	void LoadSceneObjects(nlohmann::json j, std::string type/*, std::function<std::shared_ptr<T>(std::shared_ptr<T>&)> loadCallback*/)
	{
		if (j.contains(type))
		{
			std::for_each(j.at(type).begin(), j.at(type).end(), [](nlohmann::json& json)
				{
					std::shared_ptr<T> o = std::make_shared<T>(json);
					o->this_ptr = o;
					o->BindToScene();
				}
			);
		}
	}

#if defined(_EDITOR)

	void LoadDefaultLevel()
	{
		LoadSceneObjects<Renderable>(DefaultLevel::GetDefaultLevelRenderables(), "renderables");
		LoadSceneObjects<Camera>(DefaultLevel::GetDefaultLevelCameras(), "cameras");
		LoadSceneObjects<Light>(DefaultLevel::GetDefaultLevelLights(), "lights");
		LoadSceneObjects<SoundFX>(DefaultLevel::GetDefaultLevelSounds(), "sounds");
		CreateRenderablesCameraBinding();
	}
#endif

	void LoadLevel(std::filesystem::path level)
	{
		std::string pathStr = (std::filesystem::exists(level) ? level.generic_string() : (defaultLevelsFolder + level.generic_string() + ".json"));
		OutputDebugStringA(std::string("Loading level: " + pathStr + "\n").c_str());

		std::ifstream file(pathStr);
		nlohmann::json data = nlohmann::json::parse(file);

		DestroySceneObjects();

		LoadSceneObjects<Renderable>(data, "renderables");
		LoadSceneObjects<Camera>(data, "cameras");
		LoadSceneObjects<Light>(data, "lights");
		LoadSceneObjects<SoundFX>(data, "sounds");
		CreateRenderablesCameraBinding();

		file.close();
#if defined(_EDITOR)
		Editor::currentLevelName = level.filename().string();
		Editor::MarkScenePanelAssetsAsDirty();
#endif
	}

	void DestroySceneObjects()
	{
		using namespace Scene;
		using namespace Effects;
		using namespace Animation;

		//Destroy the cameras(this will destroy the cameras and the render passes)
		DestroyCameras();

		//Destroy the effects
		EffectsDestroy();

		//Destroy sound instances
		DestroySoundEffects();

		//Destroy animated(this will destroy constants buffers)
		DestroyAnimated();

		//Destroy the renderables(this will detach the renderables from the cameras and destroy the renderables, materials, cbv, meshes, etc)
		DestroyRenderables();

		//Destroy the shadowmaps(this will destroy the shadow map cameras and render to textures of the shadowmaps)
		DestroyShadowMaps();
		//Destroy the lights(this will destroy the lights and it's cbvs)
		DestroyLights();



	}
}