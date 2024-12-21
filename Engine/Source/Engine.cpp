#include "pch.h"
#include "Engine.h"

#include "Common/StepTimer.h"
#include "Primitives/Primivites.h"
#include "Primitives/Cube.h"
#include "Primitives/Pentahedron.h"
#include "Primitives/UtahTeapot.h"
#include "Primitives/Floor.h"
#include "Primitives/Decal.h"
#include "Scene/Renderable/Renderable.h"
#include "Scene/Camera/Camera.h"
#include "Scene/Lights/Lights.h"
#include "Animation/Effects/Effects.h"
#include "Animation/Effects/DecalLoop.h"
#include "Animation/Effects/LightOscilation.h"
#include "UI/String2D.h"
#include "Renderer/DeviceUtils/D3D12Device/Interop.h"
#include "Renderer/DeviceUtils/Resources/Resources.h"
#include "Templates/Templates.h"

using Microsoft::WRL::ComPtr;
using namespace ShaderCompiler;
using namespace Templates::Shader;
using namespace Templates::Material;
using namespace Templates::Mesh;
using namespace Templates::Model3D;
using namespace Templates::Audio;
using namespace Scene::Camera;
using namespace Scene::Lights;
using namespace Scene::Renderable;
using namespace Animation::Effects;
using namespace Audio;
#if defined(_EDITOR)
using namespace Editor;
#endif

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hWnd;                                      // hWnd
RECT hWndRect;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
bool appDone = false;
bool inSizeMove = false;
bool inFullScreen = false;
bool loadingLevel = false;
nlohmann::json levelToLoad;
std::function<void()> loadLevelFn = nullptr;

std::shared_ptr<Renderer> renderer;

//FPS
DX::StepTimer timer;

//Mouse
std::unique_ptr<DirectX::Mouse> mouse;
//Keyboard
std::unique_ptr<DirectX::Keyboard> keyboard;
DirectX::Keyboard::KeyboardStateTracker keys;
//GamePad
std::unique_ptr<DirectX::GamePad> gamePad;
DirectX::GamePad::ButtonStateTracker buttons;

//camera properties
INT currentCamera = 0;
float cameraSpeed = 0.05f;
XMFLOAT2 mouseCameraRotationSensitivity = { 0.001f, 0.001f };
XMFLOAT2 gamePadCameraRotationSensitivity = { 0.02f, -0.02f };

RenderablePtr knight = nullptr;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
void LoadDefaultLevel();
void LoadScene();
void LoadLevel(std::filesystem::path level);
void DestroyInstance();
void LoadSystemTemplates();
void LoadTemplates();
void MapLightingResources();

void CreateRenderables(nlohmann::json renderables);
void CreateCameras(nlohmann::json cameras);
void CreateLights(nlohmann::json lights);

void CreateSounds();
void CreateUI();
void GameInputLoop();
void AnimableStep(double elapsedSeconds);
void AudioStep();
void UIStep();
void RenderLoop();
void DestroySceneObjects();
void ReleaseGPUResources();
void ReleaseTemplates();
void AppStep();

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // Initialize global strings
  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_WINDOWSPROJECT2, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance (hInstance, nCmdShow))
  {
    return FALSE;
  }

  // Main loop
  while (!appDone)
  {
    // Poll and handle messages (inputs, window resize, etc.)
    // See the WndProc() function below for our to dispatch events to the Win32 backend.
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
      if (msg.message == WM_QUIT)
        appDone = true;
    }
    if (appDone)
      break;

    AppStep();
  }
  DestroyInstance();

  return 0;
}

void AppStep() {
  if (loadingLevel) {
    loadLevelFn();
  } else {
    timer.Tick([&]() {});
    GameInputLoop();
    EffectsStep(static_cast<FLOAT>(timer.GetElapsedSeconds()));
    AnimableStep(timer.GetElapsedSeconds());
    if (renderer) {
      RenderLoop();
    }
    AudioStep();
#if !defined(_EDITOR)
    UIStep();
#endif
  }
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    //wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  hInst = hInstance; // Store instance handle in our global variable

  HWND desktopHwnd = GetDesktopWindow();
  RECT desktopRect;
  GetClientRect(desktopHwnd, &desktopRect);
    
  int winWidth = desktopRect.right;
  int winHeight = desktopRect.bottom - 40;

  hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 0, 0, winWidth, winHeight, nullptr, nullptr, hInstance, nullptr);
  if (!hWnd) return FALSE;
    
  SetWindowLong(hWnd, GWL_STYLE, 0);

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  GetWindowRect(hWnd, &hWndRect);

  //initialize input helpers
  mouse = std::make_unique<Mouse>();
  mouse->SetWindow(hWnd);
  
  keyboard = std::make_unique<Keyboard>();

  gamePad = std::make_unique<GamePad>();
  buttons.Reset();
  
  //initialize the shader compiler and changes monitor
  BuildShaderCompiler();
  MonitorShaderChanges(L"Shaders");

  //Initialize the audio system
  InitAudio();

  //initialize the render and reset the commands
  renderer = std::make_shared<Renderer>();
  renderer->Initialize(hWnd);
  renderer->ResetCommands();

#if defined(_EDITOR)
  InitEditor();
#endif

  //create the templates
  LoadSystemTemplates();
  LoadTemplates();
  MapLightingResources();

  //create the basic scene objects
  LoadDefaultLevel();

  //CreateSounds();
  //CreateUI();

  //kick the audio listener update
  AudioStep();

  //execute the commands on the GPU and wait for it's completion
  renderer->CloseCommandsAndFlush();
    
  return TRUE;
}

void LoadDefaultLevel()
{
  loadingLevel = true;

  CreateRenderables(DefaultLevel::renderables);
  CreateCameras(DefaultLevel::cameras);
  CreateLights(DefaultLevel::lights);

  loadingLevel = false;
}

std::mutex loadLevelMutex;
void LoadScene() {
  std::lock_guard<std::mutex> lock(loadLevelMutex);
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

void DestroyInstance()
{
  using namespace DeviceUtils::D3D12Device;

  Flush(renderer->commandQueue, renderer->fence, renderer->fenceValue, renderer->fenceEvent);

  using namespace UI;
  DestroyUI2DStrings();

  DestroySceneObjects();

  DestroyLightsResources();
  
  using namespace DeviceUtils::Resources;
  DestroyTextureResources();

  ReleaseTemplates();
  
  ReleaseGPUResources();
  
  ShutdownAudio();
  
  gamePad.reset();
  keyboard.reset();

#if defined(_EDITOR)
  DestroyEditor();
#endif

  renderer->Destroy();
  renderer = nullptr;

}

void LoadSystemTemplates() {
  auto loadingTasks = {
  concurrency::create_task([] {
    auto createShaderTasks = {
      Templates::Shader::json(L"Teapot", R"( {
        "systemCreated" : true,
        "mappedValues": [
          { "value": [ 0.11764706671237946, 0.5647059082984924, 1.0 ], "variable": "baseColor", "variableType": "FLOAT3" },
          { "value": 400.0, "variable": "specularExponent", "variableType": "FLOAT" }
        ],
        "shaderFileName": "BaseLighting"
      })"_json),
      Templates::Shader::json(L"Floor", R"( {
        "systemCreated" : true,
        "mappedValues": [
          { "value": [ 0.0, 0.0, 0.0 ], "variable": "baseColor", "variableType": "FLOAT3" },
          { "value": 1024.0, "variable": "specularExponent", "variableType": "FLOAT" }
        ],
        "shaderFileName": "Grid"
      })"_json),
      Templates::Shader::json(L"ShadowMap", R"( { "systemCreated" : true })"_json),
      Templates::Shader::json(L"ShadowMapAlpha", R"( { "systemCreated" : true })"_json),
      Templates::Shader::json(L"ShadowMapAlphaSkinning", R"( { "systemCreated" : true })"_json),
    };
    return when_all(std::begin(createShaderTasks), std::end(createShaderTasks));
  }).then([] {
    auto createMaterialTasks = {
      Templates::Material::json(L"Teapot", R"({ "shaderTemplate":"Teapot", "systemCreated":true })"_json),
      Templates::Material::json(L"Floor", R"({ "shaderTemplate":"Floor", "systemCreated":true })"_json),
      Templates::Material::json(L"ShadowMap", R"({ "shaderTemplate":"ShadowMap", "systemCreated":true })"_json),
      Templates::Material::json(L"ShadowMapAlpha", R"({ "shaderTemplate":"ShadowMapAlpha", "systemCreated":true })"_json),
      Templates::Material::json(L"ShadowMapAlphaSkinning", R"({ "shaderTemplate":"ShadowMapAlphaSkinning", "systemCreated":true })"_json),
    };
    return when_all(std::begin(createMaterialTasks), std::end(createMaterialTasks));
  }).then([]() {
    auto createMeshesTasks = {
      CreatePrimitiveMeshTemplate(L"floor"),
      CreatePrimitiveMeshTemplate(L"utahteapot"),
      CreatePrimitiveMeshTemplate(L"cube"),
      CreatePrimitiveMeshTemplate(L"pyramid"),
      CreatePrimitiveMeshTemplate(L"decal"),
    };
    return when_all(std::begin(createMeshesTasks), std::end(createMeshesTasks));
  })
  };

  when_all(std::begin(loadingTasks), std::end(loadingTasks)).wait();
}

void LoadTemplates() {

  using namespace Templates;

  Templates::LoadTemplates(defaultTemplatesFolder, Templates::Shader::templateName, Templates::Shader::json);
  Templates::LoadTemplates(defaultTemplatesFolder, Templates::Material::templateName, Templates::Material::json);
  Templates::LoadTemplates(defaultTemplatesFolder, Templates::Model3D::templateName, Templates::Model3D::json);
  Templates::LoadTemplates(defaultTemplatesFolder, Templates::Audio::templateName, Templates::Audio::json);

}

void MapLightingResources() {
  CreateLightsResources().then([] {
    const std::map<VertexClass, const MaterialPtr> shadowMapsInputLayoutMaterial = {
      { VertexClass::POS, GetMaterialTemplate(L"ShadowMap") },
      { VertexClass::POS_COLOR, GetMaterialTemplate(L"ShadowMap") },
      { VertexClass::POS_TEXCOORD0, GetMaterialTemplate(L"ShadowMapAlpha") },
      { VertexClass::POS_TEXCOORD0_SKINNING, GetMaterialTemplate(L"ShadowMapAlphaSkinning") },
      { VertexClass::POS_NORMAL, GetMaterialTemplate(L"ShadowMap") },
      { VertexClass::POS_NORMAL_TEXCOORD0, GetMaterialTemplate(L"ShadowMapAlpha") },
      { VertexClass::POS_NORMAL_TANGENT_TEXCOORD0, GetMaterialTemplate(L"ShadowMapAlpha") },
      { VertexClass::POS_NORMAL_TANGENT_TEXCOORD0_SKINNING, GetMaterialTemplate(L"ShadowMapAlphaSkinning") },
      { VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0, GetMaterialTemplate(L"ShadowMapAlpha") },
    };
    return CreateShadowMapPipeline(shadowMapsInputLayoutMaterial);
  }).wait();
}

void CreateRenderables(nlohmann::json renderables) {

  for(auto& renderable: renderables){
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

void CreateSounds() {
  CreateInstance(L"music", L"music", {
    .autoPlay = true,
    .volume = 0.3f,
  });
  CreateInstance(L"fireplace", L"fireplace", {
    .instanceFlags = SoundEffectInstance_Use3D,
    .autoPlay = true,
    .volume = 2.0f,
    .position = { -2.2f, 0.7f, 7.1f }
  });
}

void CreateUI() {
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

}

void GameInputLoop()
{
#if defined(_EDITOR)
  return;
#endif
  //get the gamepad and keyboard states and update their trackers
  auto keyboardState = keyboard->GetState();
  keys.Update(keyboardState);
  auto mouseState = mouse->GetState();
  auto gamePadState = gamePad->GetState(0);
  buttons.Update(gamePadState);

  CameraPtr camera = GetCamera(currentCamera);

  auto ChangeAnimation = [](auto renderable, bool forward) {
    auto& animLen = renderable->animations->animationsLength;
    auto currAnim = renderable->currentAnimation;
    auto it = animLen.find(currAnim);
    if (!forward) {
      if (it == animLen.begin()) {
        it = animLen.end();
      }
      it--;
    }
    else
    {
      it++;
      if (it == animLen.end()) {
        it = animLen.begin();
      }
    }
    renderable->SetCurrentAnimation(it->first);
  };

  if (!gamePadState.IsConnected()) {

    if (keys.pressed.OemPlus) { ChangeAnimation(knight, true); }
    if (keys.pressed.OemMinus) { ChangeAnimation(knight, false); }
    if (keys.pressed.PageDown) { currentCamera = (currentCamera + 1) % static_cast<UINT>(Scene::Camera::GetNumCameras()); }
    if (keys.pressed.PageUp) { currentCamera = (currentCamera != 0) ? (currentCamera - 1) : (static_cast<UINT>(Scene::Camera::GetNumCameras()) - 1); }

    //control the camera using the keyboard
    camera->ProcessKeyboardInput(keys, keyboardState);
    camera->ProcessMouseInput(mouseState, mouseCameraRotationSensitivity);

  }
  else
  {
    if (buttons.dpadUp == GamePad::ButtonStateTracker::RELEASED) { ChangeAnimation(knight, true); }
    if (buttons.dpadDown == GamePad::ButtonStateTracker::RELEASED) { ChangeAnimation(knight, false); }
    if (buttons.rightShoulder == GamePad::ButtonStateTracker::RELEASED) { currentCamera = (currentCamera + 1) % static_cast<UINT>(Scene::Camera::GetNumCameras()); }
    if (buttons.leftShoulder == GamePad::ButtonStateTracker::RELEASED) { currentCamera = (currentCamera != 0) ? (currentCamera - 1) : (static_cast<UINT>(Scene::Camera::GetNumCameras()) - 1); }

    camera->ProcessGamepadInput(gamePadState, gamePadCameraRotationSensitivity);

  }
}

void AnimableStep(double elapsedSeconds)
{
  using namespace Scene::Renderable;

  for (auto& [name, r] : GetAnimables()) {
    r->StepAnimation(elapsedSeconds);
  }
}

void AudioStep() {
  if (GetNumCameras() == 0) return;
  CameraPtr camera = GetCamera(currentCamera);
  XMVECTOR fw = camera->CameraFw();
  XMVECTOR up = camera->CameraUp();
  UpdateListener(camera->position, *(XMFLOAT3*)&fw.m128_f32, *(XMFLOAT3*)up.m128_f32);
  UpdateAudio();
}

void UIStep() {
  using namespace UI;

  GetUI2DString(L"fpsText")->text = L"FPS:" + std::to_wstring(timer.GetFramesPerSecond());

  CameraPtr camera = Scene::Camera::GetCamera(currentCamera);

  GetUI2DString(L"currentCamera")->text = camera->name;

  if (knight != nullptr) {
    GetUI2DString(L"animation")->text = L"Animation:" + knight->currentAnimation;
  }
}

void RenderLoop()
{
  std::lock_guard<std::mutex> lock(loadLevelMutex);
  if (!inFullScreen)
  {
    GetWindowRect(hWnd, &hWndRect);
  }

  renderer->ResetCommands();
  renderer->SetCSUDescriptorHeap();

  if (GetNumCameras() == 0) {
    renderer->SetRenderTargets();
#if defined(_EDITOR)
    Editor::DrawEditor();
#endif
    //before switching to 2D mode the commandQueue must be executed
    renderer->ExecuteCommands();
    //switch to 2d mode
    renderer->Set2DRenderTarget();
    renderer->Present();
    return;
  }

  CameraPtr camera = GetCamera(currentCamera);

  for (auto& [name, r] : GetRenderables()) {
    r->WriteConstantsBuffer(renderer->backBufferIndex);
  }

  WriteEffectsConstantsBuffer(renderer->backBufferIndex);

  PIXBeginEvent(renderer->commandQueue.p, 0, L"Render");
  {

    for (auto& l : Scene::Lights::GetLights()) {
      if (!l->hasShadowMaps) continue;

      auto renderSceneShadowMap = [&l](UINT cameraIndex) {
        l->shadowMapCameras[cameraIndex]->UpdateConstantsBuffer(renderer->backBufferIndex);
        for (auto& [name, r] : GetRenderables()) {
          r->RenderShadowMap(renderer, l, cameraIndex);
        }
      };

      std::wstring shadowMapEvent = L"ShadowMap:" + l->name;
      PIXBeginEvent(renderer->commandList.p, 0, shadowMapEvent.c_str());

      RenderShadowMap(l, renderSceneShadowMap);

      PIXEndEvent(renderer->commandList.p);
    }

    PIXBeginEvent(renderer->commandList.p, 0, L"Render Scene");
    {
      renderer->SetRenderTargets();

      camera->UpdateConstantsBuffer(renderer->backBufferIndex);

      UINT lightIndex = 0U;
      UINT shadowMapIndex = 0U;
      for (auto l : Scene::Lights::GetLights()) {
        UpdateConstantsBufferLightAttributes(l, renderer->backBufferIndex, lightIndex++);
        if (l->hasShadowMaps && l->lightType != LightType::Ambient) {
          UpdateConstantsBufferShadowMapAttributes(l, renderer->backBufferIndex, shadowMapIndex++);
        }
      }
      UpdateConstantsBufferNumLights(renderer->backBufferIndex, lightIndex);

      for (auto& [name, r] : Scene::Renderable::GetRenderables()) {
        r->Render(renderer, camera);
      }
    }
    PIXEndEvent(renderer->commandList.p);

#if defined(_EDITOR)
    Editor::DrawEditor();
#endif

    //before switching to 2D mode the commandQueue must be executed
    renderer->ExecuteCommands();

    //PIXBeginEvent(renderer->commandQueue.Get(), 0, L"Render UI");
    {
      //switch to 2d mode
      renderer->Set2DRenderTarget();

#if !defined(_EDITOR)
      using namespace UI;
      DrawUI2DStrings(renderer->d2d1DeviceContext);
#endif
    }
    //PIXEndEvent(renderer->commandQueue.Get());

    renderer->Present();
  }
  //PIXEndEvent(renderer->commandQueue.Get());
  PIXEndEvent(renderer->commandQueue.p);

}

void DestroySceneObjects()
{
  using namespace Animation::Effects;
  EffectsDestroy();

  //Destroy sound instances
  using namespace Audio;
  DestroySounds();
  
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

void ReleaseTemplates() {
  
  using namespace Templates::Audio;
  ReleaseSoundTemplates();

  using namespace Templates::Model3D;
  ReleaseModel3DTemplates();

  using namespace Templates::Mesh;
  ReleaseMeshTemplates();

  using namespace Templates::Material;
  ReleaseMaterialTemplates();

  using namespace Templates::Shader;
  ReleaseShaderTemplates();
}

void ReleaseGPUResources()
{
  using namespace DeviceUtils::ConstantsBuffer;
  DestroyConstantsBufferViewData();

  using namespace Scene::Lights;
  DestroyShadowMapAttributes();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#if defined(_EDITOR)
  if (WndProcHandlerEditor(hWnd, message, wParam, lParam))
    return true;
#endif

    switch (message)
    {
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        Keyboard::ProcessMessage(message, wParam, lParam);
        Mouse::ProcessMessage(message, wParam, lParam);
        break;
    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        Mouse::ProcessMessage(message, wParam, lParam);
        break;
    case WM_SYSKEYDOWN:
        if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
        {
            //implement fullscreen with ALT+ENTER
            if (inFullScreen) {
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                //SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);
                SetWindowLongPtr(hWnd, GWL_STYLE, 0);

                ShowWindow(hWnd, SW_SHOWNORMAL);
                //SetWindowLong(hWnd, GWL_STYLE, 0);
                SetWindowPos(hWnd, HWND_TOP, hWndRect.left, hWndRect.top, hWndRect.right - hWndRect.left, hWndRect.bottom - hWndRect.top, SWP_NOZORDER | SWP_FRAMECHANGED);
            } else {
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                ShowWindow(hWnd, SW_SHOWMAXIMIZED);
            }
            inFullScreen = !inFullScreen;
        }
        Keyboard::ProcessMessage(message, wParam, lParam);
        break;
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        Keyboard::ProcessMessage(message, wParam, lParam);
        break;
    case WM_PAINT:
    {
        if (inSizeMove && renderer) {
          AppStep();
        } else {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
    }
        break;
    case WM_DESTROY:
        DestroyInstance();
        PostQuitMessage(0);
        break;
    case WM_ENTERSIZEMOVE:
        inSizeMove = true;
        break;
    case WM_EXITSIZEMOVE:
        inSizeMove = false;
        if (renderer) {
            renderer->Resize(hWndRect.right - hWndRect.left, hWndRect.bottom - hWndRect.top);
        }
        break;
    case WM_NCCALCSIZE:
      if (wParam == TRUE)
      {
        return DefWindowProc(hWnd, message, wParam, lParam);
      }
      break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}