#include "pch.h"
#include "Level.h"
//scene objects
#include "Renderable/RenderableImpl.h"
#include "Camera/CameraImpl.h"
#include "Lights/LightsImpl.h"
#include "Sound/SoundEffectImpl.h"
//audio
#include "../Audio/Audio.h"
using namespace Audio;
//effects
#include "../Animation/Effects/Effects.h"
#include "../Animation/Effects/DecalLoop.h"
#include "../Animation/Effects/LightOscilation.h"
//renderer
#include "../Renderer/Renderer.h"
//editor stuff
#if defined(_EDITOR)
#include "../Editor/Editor.h"
#include "../Editor/DefaultLevel.h"
using namespace Editor;
#endif

extern std::shared_ptr<Renderer> renderer;
std::queue<std::shared_ptr<Scene::Renderable::Renderable>> renderablesToReload;

namespace Scene::Level {

  nlohmann::json levelToLoad;

  bool loadingLevel = false;
  bool isLoadingLevel() { return loadingLevel; }
  std::function<void()> loadLevelFn = nullptr;
  void callLoadLevelFn() { loadLevelFn(); }

#if defined(_EDITOR)
  void LoadDefaultLevel()
  {
    loadingLevel = true;

    CreateRenderables(DefaultLevel::renderables);
    CreateCameras(DefaultLevel::cameras);
    CreateLights(DefaultLevel::lights);

    loadingLevel = false;
  }
#endif

  void LoadLevel(std::filesystem::path level)
  {
    std::string pathStr = level.generic_string();
    std::ifstream file(pathStr);
    nlohmann::json data = nlohmann::json::parse(file);
    file.close();

    levelToLoad = data;
    loadLevelFn = LoadScene;
    loadingLevel = true;

  }

  void LoadScene() {
    renderer->ResetCommands();
    renderer->SetCSUDescriptorHeap();

    renderer->SetRenderTargets();

    DestroySceneObjects();

    if (levelToLoad.contains("renderables")) {
      CreateRenderables(levelToLoad["renderables"]);
    }

    if (levelToLoad.contains("cameras")) {
      CreateCameras(levelToLoad["cameras"]);
    }

    if (levelToLoad.contains("lights")) {
      CreateLights(levelToLoad["lights"]);
    }

    if (levelToLoad.contains("sounds")) {
      CreateSounds(levelToLoad["sounds"]);
    }

    loadingLevel = false;
    loadLevelFn = nullptr;

#if defined(_EDITOR)
    Editor::DrawEditor();
#endif

    //before switching to 2D mode the commandQueue must be executed
    renderer->ExecuteCommands();
    renderer->Set2DRenderTarget();
    renderer->Present();
  }

  void CreateRenderables(nlohmann::json renderables) {
    for (auto& renderable : renderables) {
      CreateRenderable(renderable);
    }
  }

  void CreateCameras(nlohmann::json cameras) {
    for (auto& cam : cameras) {
      CreateCamera(cam);
    }
  }

  void CreateLights(nlohmann::json lights) {
    for (auto& light : lights) {
      CreateLight(light);
    }
  }

  void CreateSounds(nlohmann::json sounds) {
    for (auto& sound : sounds) {
      CreateSoundEffect(sound);
    }
  }

  void CreateUI(nlohmann::json ui) {
    /*
    using namespace UI;

    CreateUI2DString(L"fpsText", renderer->d2d1DeviceContext, renderer->dWriteFactory, {
      .text = L"FPS:",
      .fontSize = 32.0f,
      .textAlignment = DWRITE_TEXT_ALIGNMENT_LEADING,
      .paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR,
      .drawRect = { 10.0f, 10.0f, 250.0f, 30.0f },
      });

    CreateUI2DString(L"currentCamera", renderer->d2d1DeviceContext, renderer->dWriteFactory, {
      .fontSize = 32.0f,
      .textAlignment = DWRITE_TEXT_ALIGNMENT_LEADING,
      .paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR,
      .drawRect = { 200.0f, 10.0f, 700.0f, 310.0f }
      });

    FLOAT WinHeight = static_cast<FLOAT>(hWndRect.bottom - hWndRect.top);
    CreateUI2DString(L"animation", renderer->d2d1DeviceContext, renderer->dWriteFactory, {
      .fontSize = 22,
      .textAlignment = DWRITE_TEXT_ALIGNMENT_LEADING,
      .paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR,
      .drawRect = { 60.0f, WinHeight - 90, 560.0f, WinHeight - 90 + 30.0f }
      });
      */
  }

  void PushRenderableToReloadQueue(std::shared_ptr<Scene::Renderable::Renderable> renderable)
  {
    renderablesToReload.push(renderable);
  }

  bool ReloadQueueIsEmpty() {
    return renderablesToReload.empty();
  }

  void ProcessReloadQueue() {

    renderer->ResetCommands();
    renderer->SetCSUDescriptorHeap();

    renderer->SetRenderTargets();

    while (!renderablesToReload.empty()) {
      RenderablePtr renderable = renderablesToReload.front();
      renderable->Initialize(Renderer::GetPtr());
      renderable->loading = false;
      renderablesToReload.pop();
    }

#if defined(_EDITOR)
    Editor::DrawEditor();
#endif

    //before switching to 2D mode the commandQueue must be executed
    renderer->ExecuteCommands();
    renderer->Set2DRenderTarget();
    renderer->Present();

  }

  void DestroySceneObjects()
  {
    using namespace Animation::Effects;
    EffectsDestroy();

    //Destroy sound instances
    using namespace Scene::SoundEffect;
    DestroySoundEffects();

    //Destroy the lights
    using namespace Scene::Lights;
    DestroyLights();

    //Destroy the cameras
    using namespace Scene::Camera;
    DestroyCameras();

    //Destroy animated
    using namespace Animation;
    DestroyAnimated();

    //Destroy the renderables
    using namespace Scene::Renderable;
    DestroyRenderables();
  }

}