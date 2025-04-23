#include "pch.h"
#include "Level.h"
//scene objects
#include "Scene.h"
//audio
#include "../Audio/AudioSystem.h"
using namespace AudioSystem;
//effects
#include "../Effects/Effects.h"
//#include "../Effects/DecalLoop.h"
//#include "../Effects/LightOscilation.h"
//renderer
#include "../Renderer/Renderer.h"
#if defined(_EDITOR)
#include "../Editor/Editor.h"
#include "../Editor/DefaultLevel.h"
using namespace Editor;
namespace Editor {
	extern std::string currentLevelName;
}
#endif

extern std::shared_ptr<Renderer> renderer;

namespace Scene::Level {
	using namespace Scene;

	void LoadSceneObjects(nlohmann::json j, std::string type, std::function<void(nlohmann::json)> loadCallback)
	{
		if (j.contains(type)) std::for_each(j[type].begin(), j[type].end(), loadCallback);
	}

#if defined(_EDITOR)
	void LoadDefaultLevel()
	{
		LoadSceneObjects(DefaultLevel::renderables, "renderables", CreateRenderable);
		LoadSceneObjects(DefaultLevel::cameras, "cameras", CreateCamera);
		LoadSceneObjects(DefaultLevel::lights, "lights", CreateLight);
	}
#endif

	void LoadLevel(std::filesystem::path level)
	{
		std::string pathStr = (std::filesystem::exists(level) ? level.generic_string() : (defaultLevelsFolder + level.generic_string() + ".json"));
		std::string debug = "Loading level: " + pathStr + "\n";
		OutputDebugStringA(debug.c_str());
		std::ifstream file(pathStr);
		nlohmann::json data = nlohmann::json::parse(file);

		DestroySceneObjects();

		LoadSceneObjects(data, "renderables", CreateRenderable);
		LoadSceneObjects(data, "cameras", CreateCamera);
		LoadSceneObjects(data, "lights", CreateLight);
		LoadSceneObjects(data, "sounds", CreateSoundEffect);

		file.close();
#if defined(_EDITOR)
		Editor::currentLevelName = level.filename().string();
#endif
	}

	void DestroySceneObjects()
	{
		using namespace Scene;
		using namespace Effects;
		using namespace Animation;

		//Destroy the effects
		EffectsDestroy();

		//Destroy sound instances
		DestroySoundEffects();

		//Destroy the lights
		DestroyLights();
		DestroyShadowMaps();

		//Destroy the cameras
		DestroyCameras();

		//Destroy animated
		DestroyAnimated();

		//Destroy the renderables
		DestroyRenderables();
	}
}