#include "pch.h"
#include "Engine.h"

#include "Common/StepTimer.h"
#include "Primitives/Primivites.h"
#include "Primitives/Cube.h"
#include "Primitives/Pentahedron.h"
#include "Primitives/UtahTeapot.h"
#include "Primitives/Floor.h"
#include "Primitives/Decal.h"
#include "Scene/Camera/Camera.h"
#include "Scene/Lights/Lights.h"
#include "Animation/Effects/Effects.h"
#include "Animation/Effects/DecalLoop.h"
#include "Animation/Effects/LightOscilation.h"
#include "2D/FPSText.h"
#include "2D/Label.h"

/*
#include "2D/XBoxOneButton.h"
#include "2D/XBoxOneViewButton.h"
#include "2D/XBoxOneTrigger.h"
#include "2D/XBoxOneBumper.h"
#include "2D/XBoxOneDPad.h"
#include "2D/KeyboardButton.h"
*/

using Microsoft::WRL::ComPtr;
using namespace Templates::Shader;
using namespace Templates::Material;
using namespace Templates::Mesh;
using namespace Templates::Model3D;
using namespace Templates::Audio;
using namespace Scene::Lights;
using namespace Animation::Effects;
using namespace Audio;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hWnd;                                      // hWnd
RECT hWndRect;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
bool inSizeMove = false;
bool inFullScreen = false;

std::shared_ptr<Renderer> renderer;

//FPS
std::shared_ptr<FPSText> fpsText;
DX::StepTimer timer;

//Mouse
std::unique_ptr<DirectX::Mouse> mouse;
//Keyboard
std::unique_ptr<DirectX::Keyboard> keyboard;
DirectX::Keyboard::KeyboardStateTracker keys;
//GamePad
std::unique_ptr<DirectX::GamePad> gamePad;
DirectX::GamePad::ButtonStateTracker buttons;

/*
std::shared_ptr<XBoxOneButton> xboxOneButtonA;
std::shared_ptr<XBoxOneButton> xboxOneButtonB;
std::shared_ptr<XBoxOneButton> xboxOneButtonX;
std::shared_ptr<XBoxOneButton> xboxOneButtonY;
std::shared_ptr<XBoxOneViewButton> xboxOneViewButton;
std::shared_ptr<XBoxOneTrigger> xboxOneButtonLT;
std::shared_ptr<XBoxOneTrigger> xboxOneButtonRT;
std::shared_ptr<XBoxOneBumper> xboxOneButtonLB;
std::shared_ptr<XBoxOneBumper> xboxOneButtonRB;
*/

/*
std::shared_ptr<KeyboardButton> keyboardButtonA;
std::shared_ptr<KeyboardButton> keyboardButtonS;
std::shared_ptr<KeyboardButton> keyboardButtonD;
std::shared_ptr<KeyboardButton> keyboardButtonP;
std::shared_ptr<KeyboardButton> keyboardButtonL;
std::shared_ptr<KeyboardButton> keyboardButtonM;
std::shared_ptr<KeyboardButton> keyboardButtonN;
std::shared_ptr<KeyboardButton> keyboardButtonPU;
std::shared_ptr<KeyboardButton> keyboardButtonPD;
std::shared_ptr<KeyboardButton> keyboardButtonPlus;
std::shared_ptr<KeyboardButton> keyboardButtonMinus;

//Light status labels
std::shared_ptr<Label> ambientLightLabel;
std::shared_ptr<Label> directionalLightLabel;
std::shared_ptr<Label> spotLightLabel;
std::shared_ptr<Label> pointLightLabel;
std::shared_ptr<Label> lightModelLabelCentered;
std::shared_ptr<Label> lightModelLabelLeft;
std::shared_ptr<Label> shadowMapsLabel;
std::shared_ptr<Label> normalMappingLabel;

//camera selection
*/
INT currentCamera = 0;
std::shared_ptr<Label> perspectiveSelectionLabel;

/*
std::shared_ptr<Label> perspectiveSelectionLabelPrevious;
std::shared_ptr<Label> perspectiveSelectionLabelNext;
*/
//animation
std::shared_ptr<Label> animationLabel;
/*
std::shared_ptr<Label> nextAnimationLabel;
std::shared_ptr<Label> previousAnimationLabel;
std::shared_ptr<XBoxOneDPad> xboxOneDPad;
*/
//camera properties
//propiedades de la camara
float cameraSpeed = 0.05f;
XMFLOAT2 mouseCameraRotationSensitivity = { 0.001f, 0.001f };
XMFLOAT2 gamePadCameraRotationSensitivity = { 0.02f, -0.02f };


//Audio
/*
std::unique_ptr<AudioEngine> audio;

std::unique_ptr<SoundEffect> fireplaceSound;

std::unique_ptr<SoundEffectInstance> fireplaceSoundInstance;
AudioEmitter fireplaceSoundEmitter;

std::unique_ptr<SoundEffect> musicSound;
std::unique_ptr<SoundEffectInstance> musicSoundInstance;
*/

RenderablePtr knight = nullptr;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
void CreateTemplates(std::shared_ptr<Renderer>& renderer);
void CreateCameras(std::shared_ptr<Renderer>& renderer);
void CreateLights(std::shared_ptr<Renderer>& renderer);
void CreateRenderables(std::shared_ptr<Renderer>& renderer);
void CreateSounds();
void InputLoop();
void AnimableStep(double elapsedSeconds);
void AudioStep();
void RenderLoop();
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

  // Main message loop
  MSG msg = {};
  while (WM_QUIT != msg.message)
  {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      timer.Tick([&]() {});
      InputLoop();
      EffectsStep(static_cast<FLOAT>(timer.GetElapsedSeconds()));
      AnimableStep(timer.GetElapsedSeconds());
      if (renderer) {
        RenderLoop();
      }
      AudioStep();
    }
  }
  return (int) msg.wParam;
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
  ShaderCompiler::BuildCompiler();
  ShaderCompiler::MonitorChanges(L"Shaders");

  //Initialize the audio system
  InitAudio();

  //initialize the render and reset the commands
  renderer = std::make_shared<Renderer>();
  renderer->Initialize(hWnd);
  renderer->ResetCommands();

  //create the objects
  CreateTemplates(renderer);
  CreateRenderables(renderer);
  CreateCameras(renderer);
  CreateLights(renderer);
  CreateSounds();

  //kick the audio listener update
  AudioStep();

  //initialize 2D objects
  //inicializar objetos 2D
  fpsText = std::make_shared<FPSText>();
  fpsText->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory);
  /*
  //initialize the labels
  //inicializar los labels
  ambientLightLabel = std::make_shared<Label>(); ambientLightLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
  directionalLightLabel = std::make_shared<Label>(); directionalLightLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
  spotLightLabel = std::make_shared<Label>(); spotLightLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
  pointLightLabel = std::make_shared<Label>(); pointLightLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
  lightModelLabelCentered = std::make_shared<Label>(); lightModelLabelCentered->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  lightModelLabelLeft = std::make_shared<Label>(); lightModelLabelLeft->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
  shadowMapsLabel = std::make_shared<Label>(); shadowMapsLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
  normalMappingLabel = std::make_shared<Label>(); normalMappingLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
  */
  perspectiveSelectionLabel = std::make_shared<Label>(); perspectiveSelectionLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 32, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR, D2D1::ColorF::White);
  /*
  perspectiveSelectionLabelPrevious = std::make_shared<Label>(); perspectiveSelectionLabelPrevious->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR, D2D1::ColorF::White);
  perspectiveSelectionLabelNext = std::make_shared<Label>(); perspectiveSelectionLabelNext->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR, D2D1::ColorF::White);
  */
  animationLabel = std::make_shared<Label>(); animationLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 22, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR, D2D1::ColorF::White);
  /*
  nextAnimationLabel = std::make_shared<Label>(); nextAnimationLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR, D2D1::ColorF::White);
  previousAnimationLabel = std::make_shared<Label>(); previousAnimationLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR, D2D1::ColorF::White);

  //initialize the keyboard buttons
  //inicializar los botones del teclado
  keyboardButtonA = std::make_shared<KeyboardButton>(); keyboardButtonA->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"A", D2D1::ColorF(0xffffff));
  keyboardButtonS = std::make_shared<KeyboardButton>(); keyboardButtonS->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"S", D2D1::ColorF(0xffffff));
  keyboardButtonD = std::make_shared<KeyboardButton>(); keyboardButtonD->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"D", D2D1::ColorF(0xffffff));
  keyboardButtonP = std::make_shared<KeyboardButton>(); keyboardButtonP->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"P", D2D1::ColorF(0xffffff));
  keyboardButtonL = std::make_shared<KeyboardButton>(); keyboardButtonL->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"L", D2D1::ColorF(0xffffff));
  keyboardButtonM = std::make_shared<KeyboardButton>(); keyboardButtonM->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"M", D2D1::ColorF(0xffffff));
  keyboardButtonPU = std::make_shared<KeyboardButton>(); keyboardButtonPU->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"Page Up", D2D1::ColorF(0xffffff), 10.0f);
  keyboardButtonPD = std::make_shared<KeyboardButton>(); keyboardButtonPD->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"Page Down", D2D1::ColorF(0xffffff), 10.0f);
  keyboardButtonN = std::make_shared<KeyboardButton>(); keyboardButtonN->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"N", D2D1::ColorF(0xffffff));
  keyboardButtonPlus = std::make_shared<KeyboardButton>(); keyboardButtonPlus->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"+", D2D1::ColorF(0xffffff), 10.0f);
  keyboardButtonMinus = std::make_shared<KeyboardButton>(); keyboardButtonMinus->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"-", D2D1::ColorF(0xffffff), 10.0f);

  //initialize the xbox buttons
  //inicializar los botones de xbox
  xboxOneButtonA = std::make_shared<XBoxOneButton>(); xboxOneButtonA->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"A", D2D1::ColorF(0x3cdb4e));
  xboxOneButtonB = std::make_shared<XBoxOneButton>(); xboxOneButtonB->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"B", D2D1::ColorF(0xd04242));
  xboxOneButtonX = std::make_shared<XBoxOneButton>(); xboxOneButtonX->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"X", D2D1::ColorF(0x40ccd0));
  xboxOneButtonY = std::make_shared<XBoxOneButton>(); xboxOneButtonY->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, L"Y", D2D1::ColorF(0xecdb33));
  xboxOneViewButton = std::make_shared<XBoxOneViewButton>(); xboxOneViewButton->Initialize(renderer->d2d1DeviceContext, renderer->d2d1Factory);
  xboxOneButtonLT = std::make_shared<XBoxOneTrigger>(); xboxOneButtonLT->Initialize(renderer->d2d1DeviceContext, renderer->d2d1Factory, renderer->dWriteFactory, L"LT", D2D1::ColorF::White);
  xboxOneButtonLB = std::make_shared<XBoxOneBumper>(); xboxOneButtonLB->Initialize(renderer->d2d1DeviceContext, renderer->d2d1Factory, renderer->dWriteFactory, L"LB", D2D1::ColorF::White);
  xboxOneButtonRB = std::make_shared<XBoxOneBumper>(); xboxOneButtonRB->Initialize(renderer->d2d1DeviceContext, renderer->d2d1Factory, renderer->dWriteFactory, L"RB", D2D1::ColorF::White);
  xboxOneButtonRT = std::make_shared<XBoxOneTrigger>(); xboxOneButtonRT->Initialize(renderer->d2d1DeviceContext, renderer->d2d1Factory, renderer->dWriteFactory, L"RT", D2D1::ColorF::White);
  xboxOneDPad = std::make_shared<XBoxOneDPad>(); xboxOneDPad->Initialize(renderer->d2d1DeviceContext, renderer->d2d1Factory);

  //initialize DirectXTK Audio Engine
  //inicializar el Audio Engine de DirectXTK
  AUDIO_ENGINE_FLAGS audioFlags = AudioEngine_Default;
  #ifdef _DEBUG
  audioFlags = audioFlags | AudioEngine_Debug;
  #endif
  audio = std::make_unique<AudioEngine>(audioFlags);

  //create the music and play and loop it
  //creamos la musica y la dejamos en loop
  musicSound = std::make_unique<SoundEffect>(audio.get(), L"Assets/sounds/music.wav");
  musicSoundInstance = musicSound->CreateInstance();
  musicSoundInstance->SetVolume(0.2f);
  musicSoundInstance->Play(true);

  //create the fireplace sound and loop it
  //creamos el sonido del fuego y lo dejamos en loop
  fireplaceSound = std::make_unique<SoundEffect>(audio.get(), L"Assets/sounds/fireplace.wav");
  fireplaceSoundInstance = fireplaceSound->CreateInstance(SoundEffectInstance_Use3D);
  fireplaceSoundInstance->SetVolume(2.0f);
  fireplaceSoundInstance->Play(true);

  //adjusts the position of the sound source
  //ajustamos la posicion de la fuente del sonido
  fireplaceSoundEmitter.SetPosition({ -2.2f, 0.7f, 7.1f, 0.0f });

  //adjust the listener position based on the camera properties
  //ajustamos la posicion del oyente basado en las propiedades de la camara
  AudioListener listener;
  listener.SetPosition(cameraPos);
  listener.SetOrientation(cameraFw(), up);
  fireplaceSoundInstance->Apply3D(listener, fireplaceSoundEmitter);
  */

  //execute the commands on the GPU and wait for it's completion
  //ejecuta los comandos de la GPU y esperar a que termine
  renderer->CloseCommandsAndFlush();
    
  /*
  scene->DestroyUploadResources();
  //knight->DestroyUploadResources();
    
  for (auto f : fire) {
      f->DestroyUploadResources();
  }
  for (auto f : candleFlame) {
      f->DestroyUploadResources();
  }
  */
  return TRUE;
}

void CreateTemplates(std::shared_ptr<Renderer>& renderer) {
  auto loadingTasks = {
    concurrency::create_task([] {
      auto createShaderTasks = {
        CreateShaderTemplate(L"Teapot", {
          .shaderFileName = L"BaseLighting",
          .mappedValues = {
            { L"specularExponent", { MaterialVariablesTypes::MAT_VAR_FLOAT, 400.0f }},
            { L"baseColor" , { MaterialVariablesTypes::MAT_VAR_FLOAT3, DirectX::Colors::DodgerBlue }}
          }
        }),
        CreateShaderTemplate(L"TexturedLighting", {
          .mappedValues = {
            { L"specularExponent", { MaterialVariablesTypes::MAT_VAR_FLOAT, 1024.0f }},
          }
        }),
        CreateShaderTemplate(L"NormalMappingLighting", {
          .mappedValues = {
            { L"specularExponent", { MaterialVariablesTypes::MAT_VAR_FLOAT, 1024.0f }},
          }
        }),
        CreateShaderTemplate(L"Floor", {
          .shaderFileName = L"Grid",
          .mappedValues = {
            { L"specularExponent", { MaterialVariablesTypes::MAT_VAR_FLOAT, 1024.0f }},
            { L"baseColor" , { MaterialVariablesTypes::MAT_VAR_FLOAT3, DirectX::Colors::Black }}
          }
        }),
        CreateShaderTemplate(L"ShadowMap"),
        CreateShaderTemplate(L"ShadowMapAlpha"),
        CreateShaderTemplate(L"ShadowMapAlphaSkinning"),
        CreateShaderTemplate(L"Model3D"),
        CreateShaderTemplate(L"SkinningPBRLighting"),
        CreateShaderTemplate(L"AnimatedDecal", {
          .mappedValues = {
            { L"frameIndex", { MaterialVariablesTypes::MAT_VAR_UNSIGNED_INTEGER, 0U }},
          },
        }),
      };
      return when_all(std::begin(createShaderTasks), std::end(createShaderTasks));
    }).then([] {
      auto createMaterialTasks = {
        CreateMaterialTemplate(L"Teapot", {
          .shaderTemplate = L"Teapot"
        }),
        CreateMaterialTemplate(L"Crate", {
          .shaderTemplate = L"TexturedLighting",
          .textures = {
            { L"Assets/crate/crate.dds" }
          },
          .samplers = { MaterialSamplerDesc() },
        }),
        CreateMaterialTemplate(L"Pyramid",{
          .shaderTemplate = L"NormalMappingLighting",
          .mappedValues = {
            { L"specularExponent", { MaterialVariablesTypes::MAT_VAR_FLOAT, 1024.0f }},
          },
          .textures = {
            { L"Assets/pyramid/pyramid.dds" },
            { L"Assets/pyramid/pyramidNormalMap.dds" },
          },
          .samplers = { MaterialSamplerDesc() },
        }),
        CreateMaterialTemplate(L"Floor",{
          .shaderTemplate = L"Floor",
        }),
        CreateMaterialTemplate(L"ShadowMap",{
          .shaderTemplate = L"ShadowMap"
        }),
        CreateMaterialTemplate(L"ShadowMapAlpha",{
          .shaderTemplate = L"ShadowMapAlpha"
        }),
        CreateMaterialTemplate(L"ShadowMapAlphaSkinning",{
          .shaderTemplate = L"ShadowMapAlphaSkinning"
        }),
        CreateMaterialTemplate(L"Model3D",{
          .shaderTemplate = L"Model3D"
        }),
        CreateMaterialTemplate(L"Fireplace",{
          .shaderTemplate = L"AnimatedDecal",
          .mappedValues = {
            { L"alphaCut", { MaterialVariablesTypes::MAT_VAR_FLOAT, 100.0f/255.0f }},
          },
          .textures = {
            { L"Assets/fx/fire-65.dds", DXGI_FORMAT_BC7_UNORM_SRGB , 50U }
          },

          .twoSided = true,
        }),
        CreateMaterialTemplate(L"CandleFlame",{
          .shaderTemplate = L"AnimatedDecal",
          .mappedValues = {
            { L"alphaCut", { MaterialVariablesTypes::MAT_VAR_FLOAT, 250.0f/255.0f }},
          },
          .textures = {
            { L"Assets/fx/flame.dds", DXGI_FORMAT_BC7_UNORM_SRGB , 7U }
          },

          .twoSided = true,
        }),
      };
      return when_all(std::begin(createMaterialTasks), std::end(createMaterialTasks));
    }).then([&renderer]() {
      auto createMeshesTasks = {
        CreateMeshTemplate(L"utahteapot",renderer,Primitives::LoadPrimitiveIntoMesh<UtahTeapot>),
        CreateMeshTemplate(L"cube",renderer,Primitives::LoadPrimitiveIntoMesh<Cube>),
        CreateMeshTemplate(L"pyramid",renderer,Primitives::LoadPrimitiveIntoMesh<Pentahedron>),
        CreateMeshTemplate(L"floor", renderer, Primitives::LoadPrimitiveIntoMesh<Floor>),
        CreateModel3DTemplate(L"messy_tavern",L"messy_tavern/scene.gltf",renderer, {
          .materialsShader = L"NormalMappingLighting"
        }),
        CreateModel3DTemplate(L"knight",L"knight/scene.gltf",renderer,{
          .materialsShader = L"SkinningPBRLighting"
        }),
        CreateMeshTemplate(L"decal", renderer, Primitives::LoadPrimitiveIntoMesh<Decal>,{
        }),
      };
      return when_all(std::begin(createMeshesTasks), std::end(createMeshesTasks));
    }).then([]{
      auto createSoundsTasks = {
        CreateSoundTemplate(L"music",L"Assets/sounds/music.wav"),
        CreateSoundTemplate(L"fireplace",L"Assets/sounds/fireplace.wav")
      };
      return when_all(std::begin(createSoundsTasks), std::end(createSoundsTasks));
    })
  };

  when_all(std::begin(loadingTasks), std::end(loadingTasks)).wait();
}

void CreateCameras(std::shared_ptr<Renderer>& renderer) {
  UINT winWidth = hWndRect.right - hWndRect.left;
  UINT winHeight = hWndRect.bottom - hWndRect.top;

  CameraPtr camera = Scene::Camera::CreateCamera({
    .position = { -5.7f, 2.2f, 3.8f },
    .rotation = { DirectX::XM_PIDIV2 , -DirectX::XM_PIDIV4 * 0.25f },
  });
  camera->perspective.updateProjectionMatrix(winWidth, winHeight);
  camera->CreateConstantsBufferView(renderer);
}

void CreateLights(std::shared_ptr<Renderer>& renderer) {

  CreateLightsResources(renderer).then([&renderer] {
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
    return CreateShadowMapPipeline(renderer, shadowMapsInputLayoutMaterial);
  }).wait();
  CreateLight(renderer, {
    .name = L"light0(amb)",
    .lightType = LightType::Ambient,
    .ambient = {
      .color = { 0.05f, 0.05f, 0.05f }
    }
  });
  CreateLight(renderer, {
    .name = L"light1(dir)",
    .lightType = LightType::Directional,
    .directional = {
      //.color = { 0.3f, 0.3f, 0.4f },
      .color = *((XMFLOAT3*)&DirectX::Colors::White),
      .rotation = { 1.54499948f, -0.300000399f },
    },
    .hasShadowMap = true,
    .directionalLightShadowMapParams = {
      .shadowMapWidth = 4096U,
      .shadowMapHeight = 4096U,
      .viewWidth = 32.0f,
      .viewHeight = 32.0f,
      .nearZ = 0.01f,
      .farZ = 1000.0f
    },
  });
  CreateLight(renderer, {
    .name = L"light2(spot)",
    .lightType = LightType::Spot,
    .spot = {
      .color = *((XMFLOAT3*)&DirectX::Colors::WhiteSmoke),
      .position = { -6.52f, 2.16f, 0.65f}, //position
      //.rotation = { 1.46f, -0.37f }, //rotation
      .azimuthalAngle = 1.46f,
      .polarAngle = -0.37f,
      .coneAngle = DirectX::XM_PIDIV4*0.25f,
      .attenuation = { 0.0f, 0.01f, 0.02f },
    },
    .hasShadowMap = true,
    .spotLightShadowMapParams = {
      .shadowMapWidth = 1024U,
      .shadowMapHeight = 1024U,
      .viewWidth = 32.0f,
      .viewHeight = 32.0f,
      .nearZ = 0.01f,
      .farZ = 100.0f,
    }
  });
  CreateLight(renderer, {
    .name = L"light3(point)",
    .lightType = LightType::Point,
    .point = {
      .color = { 234.0f / 255.0f, 81.0f / 255.0f, 4.0f / 255.0f },
      .position = { -2.2f, 0.7f, 7.1f },
      .attenuation = { 0.0f, 0.1f, 0.02f },
    },
    .hasShadowMap = true,
    .pointLightShadowMapParams = {
      .shadowMapWidth = 1024U,
      .shadowMapHeight = 1024U,
      .nearZ = 0.01f,
      .farZ = 20.0f,
    },
  },[](LightPtr light){
      LightOscilation fireplace = {
        .originalValue = light->point.color,
        .target = &light->point.color,
        .offset = 0.9f,
        .amplitude = 0.1f,
        .angularFrequency = 45.0f,
      };
    CreateLightEffects[L"LightOscilation"](light,&fireplace);
  });
}

void CreateRenderables(std::shared_ptr<Renderer>& renderer) {

  using namespace Renderable;

  CreateRenderable(renderer, {
    .name = L"utahteapot",
    .meshMaterials = { { GetMeshTemplate(L"utahteapot"), GetMaterialTemplate(L"Teapot")} },
    .meshMaterialsShadowMap = { { GetMeshTemplate(L"utahteapot"), GetMaterialTemplate(L"ShadowMap") } },
    .position = { -2.0f, -0.6f, 2.0f },
    .scale = { 0.01f, 0.01f, 0.01f }
  });
  CreateRenderable(renderer, {
    .name = L"crate",
    .meshMaterials = { { GetMeshTemplate(L"cube"), GetMaterialTemplate(L"Crate")} },
    .meshMaterialsShadowMap = { { GetMeshTemplate(L"cube"), GetMaterialTemplate(L"ShadowMapAlpha")} },
  });
  CreateRenderable(renderer, {
    .name = L"pyramid",
    .meshMaterials = { { GetMeshTemplate(L"pyramid"), GetMaterialTemplate(L"Pyramid") } },
    .meshMaterialsShadowMap = { { GetMeshTemplate(L"pyramid"), GetMaterialTemplate(L"ShadowMapAlpha") } },
    .position = { 6.0f, -1.0f, 2.0f, 1.0f }
  });
  CreateRenderable(renderer, {
    .name = L"floor",
    .meshMaterials = { { GetMeshTemplate(L"floor"), GetMaterialTemplate(L"Floor")} },
    .position = { 0.0f, -1.0f, 0.0f },
    .scale = { 20.0f, 1.0f, 20.0f },
  });
  CreateRenderable(renderer, {
    .name = L"tavern",
    .modelMaterials = { GetModel3DTemplate(L"messy_tavern"), {
      GetMaterialTemplate(L"mat.messy_tavern.0"),
      GetMaterialTemplate(L"mat.messy_tavern.1"),
      GetMaterialTemplate(L"mat.messy_tavern.2"),
      GetMaterialTemplate(L"mat.messy_tavern.3"),
      GetMaterialTemplate(L"mat.messy_tavern.4"),
      GetMaterialTemplate(L"mat.messy_tavern.5"),
      GetMaterialTemplate(L"mat.messy_tavern.6"),
    }},
    .modelMaterialsShadowMap = { GetModel3DTemplate(L"messy_tavern"), {
      GetMaterialTemplate(L"ShadowMapAlpha"),
      GetMaterialTemplate(L"ShadowMapAlpha"),
      GetMaterialTemplate(L"ShadowMapAlpha"),
      GetMaterialTemplate(L"ShadowMapAlpha"),
      GetMaterialTemplate(L"ShadowMapAlpha"),
      GetMaterialTemplate(L"ShadowMapAlpha"),
      GetMaterialTemplate(L"ShadowMapAlpha"),
    }},
    .position = { -14.0f, -1.1f, 0.0f},
    .scale = { 0.1f, 0.1f, 0.1f },
    .rotation = { -DirectX::XM_PIDIV2, DirectX::XM_PI, DirectX::XM_PIDIV2 },
    .skipMeshes = { 0 }
  });
  knight = CreateRenderable(renderer, {
    .name = L"knight",
    .modelMaterials = { GetModel3DTemplate(L"knight"), {
      GetMaterialTemplate(L"mat.knight.0"),
      GetMaterialTemplate(L"mat.knight.1"),
      GetMaterialTemplate(L"mat.knight.2"),
      GetMaterialTemplate(L"mat.knight.3"),
      GetMaterialTemplate(L"mat.knight.4"),
      GetMaterialTemplate(L"mat.knight.5"),
      GetMaterialTemplate(L"mat.knight.6"),
      GetMaterialTemplate(L"mat.knight.7"),
      GetMaterialTemplate(L"mat.knight.8"),
      GetMaterialTemplate(L"mat.knight.9"),
      GetMaterialTemplate(L"mat.knight.10"),
      GetMaterialTemplate(L"mat.knight.11"),
    }},
    .modelMaterialsShadowMap = { GetModel3DTemplate(L"knight"), {
      GetMaterialTemplate(L"ShadowMapAlphaSkinning"),
      GetMaterialTemplate(L"ShadowMapAlphaSkinning"),
      GetMaterialTemplate(L"ShadowMapAlphaSkinning"),
      GetMaterialTemplate(L"ShadowMapAlphaSkinning"),
      GetMaterialTemplate(L"ShadowMapAlphaSkinning"),
      GetMaterialTemplate(L"ShadowMapAlphaSkinning"),
      GetMaterialTemplate(L"ShadowMapAlphaSkinning"),
      GetMaterialTemplate(L"ShadowMapAlphaSkinning"),
      GetMaterialTemplate(L"ShadowMapAlphaSkinning"),
      GetMaterialTemplate(L"ShadowMapAlphaSkinning"),
      GetMaterialTemplate(L"ShadowMapAlphaSkinning"),
      GetMaterialTemplate(L"ShadowMapAlphaSkinning"),
    }},
    .position = { 0.0f, -0.95f, 3.0f},
    .scale = { 2.0f, 2.0f, 2.0f },
    .rotation = { -DirectX::XM_PIDIV2, -DirectX::XM_PIDIV2, 0.0f },
  });

  for (UINT flameIndex = 0U; flameIndex < 2U; flameIndex++) {
    std::wstring fireplaceName = L"fireplace(" + std::to_wstring(flameIndex) + L")";
    float flameRot = DirectX::XM_PIDIV2 * static_cast<float>(flameIndex);

    CreateRenderable(renderer, {
      .name = fireplaceName,
      .meshMaterials = { { GetMeshTemplate(L"decal"), GetMaterialTemplate(L"Fireplace")} },
      .position = { -2.2f, 0.3f, 8.0f },
      .scale = { 1.5f, 1.5f, 1.5f },
      .rotation = { 0.0f , flameRot, 0 }
    },[](RenderablePtr renderable) {
        DecalLoopT decal = {
          .numFrames = 50U,
          .timePerFrames = 0.04f,
        };
        Animation::Effects::CreateRenderableEffects[L"DecalLoop"](renderable, &decal);
    });
  }

  for (UINT candleFlameIndex = 0U; candleFlameIndex < 6U; candleFlameIndex++) {
    XMVECTOR candlePos[] = { { -4.55f, 4.38f,  7.65f }, {  2.24f, 1.80f,  7.39f }, { 10.29f, 1.26f, 12.36f }, };
    std::wstring candleFlameName = L"candleFlame(" + std::to_wstring(candleFlameIndex) + L")";
    float candleFlameRot = DirectX::XM_PIDIV2 * static_cast<float>(candleFlameIndex %2);
    CreateRenderable(renderer, {
      .name = candleFlameName,
      .meshMaterials = { { GetMeshTemplate(L"decal"), GetMaterialTemplate(L"CandleFlame")} },
      .position = candlePos[candleFlameIndex>>1],
      .scale = { 0.075f, 0.33f, 0.075f },
      .rotation = { 0.0f , candleFlameRot, 0 }
      }, [](RenderablePtr renderable) {
        DecalLoopT decal = {
          .numFrames = 7U,
          .timePerFrames = 0.1f
        };
        Animation::Effects::CreateRenderableEffects[L"DecalLoop"](renderable, &decal);
      });
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

void InputLoop()
{
  //get the gamepad and keyboard states and update their trackers
  auto keyboardState = keyboard->GetState();
  keys.Update(keyboardState);
  auto mouseState = mouse->GetState();
  auto gamePadState = gamePad->GetState(0);
  buttons.Update(gamePadState);

  CameraPtr camera = Scene::Camera::GetCamera(currentCamera);

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
    renderable->currentAnimation = it->first;
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
  using namespace Renderable;

  for (auto& [name, r] : GetAnimables()) {
    r->StepAnimation(elapsedSeconds);
  }
}

void AudioStep() {
  CameraPtr camera = Scene::Camera::GetCamera(currentCamera);
  XMVECTOR fw = camera->CameraFw();
  XMVECTOR up = camera->CameraUp();
  UpdateListener(camera->position, *(XMFLOAT3*)&fw.m128_f32, *(XMFLOAT3*)up.m128_f32);
  UpdateAudio();
}

void RenderLoop()
{
    if (!inFullScreen)
    {
        GetWindowRect(hWnd, &hWndRect);
    }

    CameraPtr camera = Scene::Camera::GetCamera(currentCamera);
    
    renderer->ResetCommands();

    renderer->SetCSUDescriptorHeap();

    using namespace Renderable;

    for (auto& [name,r] : GetRenderables()) {
      r->WriteConstantsBuffer(renderer->backBufferIndex);
    }

    WriteEffectsConstantsBuffer(renderer->backBufferIndex);

    PIXBeginEvent(renderer->commandQueue.Get(), 0, L"Render");
    {

        for (auto& l : Scene::Lights::GetLights()) {
            if (!l->hasShadowMaps) continue;

            auto renderSceneShadowMap = [&l](UINT cameraIndex) {
                l->shadowMapCameras[cameraIndex]->UpdateConstantsBuffer(renderer->backBufferIndex);
                for (auto& [name,r] : GetRenderables()) {
                    r->RenderShadowMap(renderer, l, cameraIndex);
                }
            };

            std::wstring shadowMapEvent = L"ShadowMap:"+l->name;
            PIXBeginEvent(renderer->commandList.Get(), 0, shadowMapEvent.c_str());

            RenderShadowMap(l, renderer, renderSceneShadowMap);

            PIXEndEvent(renderer->commandList.Get());
        }

        PIXBeginEvent(renderer->commandList.Get(), 0, L"Render Scene");
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

            for (auto& [name,r] : GetRenderables()) {
              r->Render(renderer,camera);
            }
        }
        PIXEndEvent(renderer->commandList.Get());

        //before switching to 2D mode the commandQueue must be executed
        renderer->ExecuteCommands();

        //PIXBeginEvent(renderer->commandQueue.Get(), 0, L"Render UI");
        {
            //switch to 2d mode
            //cambiar a modo 2D
            renderer->Set2DRenderTarget();

            //draw the framerate
            //dibujar el framerate
            fpsText->Render(renderer->d2d1DeviceContext, timer.GetFramesPerSecond());

            perspectiveSelectionLabel->Render(renderer->d2d1DeviceContext, 200, 10, 500, 300, camera->name.c_str());

            /*
            FLOAT WinWidth = (FLOAT)(hWndRect.right - hWndRect.left);
            FLOAT WinHeight = (FLOAT)(hWndRect.bottom - hWndRect.top);
            if (gamePadState.IsConnected()) {
                //draw the XBoxOne buttons
                //dibujar los botones de la XBoxOne
                const D2D1_POINT_2F gamePadOffsets[] = {
                    D2D1::Point2F(100.0f, 40.0f),
                    D2D1::Point2F( 60.0f, 80.0f),
                    D2D1::Point2F(140.0f, 80.0f),
                    D2D1::Point2F(100.0f,120.0f),
                    D2D1::Point2F(200.0f,120.0f),
                    D2D1::Point2F(200.0f, 45.0f),
                    D2D1::Point2F(400.0f, 45.0f),
                    D2D1::Point2F( 40.0f,230.0f),
                    D2D1::Point2F( 40.0f,230.0f)
                };
                xboxOneButtonA->Render(renderer->d2d1DeviceContext, WinWidth - gamePadOffsets[0].x, WinHeight - gamePadOffsets[0].y, buttons.a == GamePad::ButtonStateTracker::HELD);
                xboxOneButtonB->Render(renderer->d2d1DeviceContext, WinWidth - gamePadOffsets[1].x, WinHeight - gamePadOffsets[1].y, buttons.b == GamePad::ButtonStateTracker::HELD);
                xboxOneButtonX->Render(renderer->d2d1DeviceContext, WinWidth - gamePadOffsets[2].x, WinHeight - gamePadOffsets[2].y, buttons.x == GamePad::ButtonStateTracker::HELD);
                xboxOneButtonY->Render(renderer->d2d1DeviceContext, WinWidth - gamePadOffsets[3].x, WinHeight - gamePadOffsets[3].y, buttons.y == GamePad::ButtonStateTracker::HELD);
                xboxOneViewButton->Render(renderer->d2d1DeviceContext, WinWidth - gamePadOffsets[4].x, WinHeight - gamePadOffsets[4].y, buttons.view == GamePad::ButtonStateTracker::HELD);
                xboxOneButtonLB->Render(renderer->d2d1DeviceContext, gamePadOffsets[5].x, gamePadOffsets[5].y, buttons.leftShoulder == GamePad::ButtonStateTracker::HELD, TRUE);
                xboxOneButtonRB->Render(renderer->d2d1DeviceContext, gamePadOffsets[6].x, gamePadOffsets[6].y, buttons.rightShoulder == GamePad::ButtonStateTracker::HELD, FALSE);
                xboxOneButtonLT->Render(renderer->d2d1DeviceContext, gamePadOffsets[7].x, WinHeight - gamePadOffsets[7].y, buttons.leftTrigger == GamePad::ButtonStateTracker::HELD, FALSE);
                xboxOneButtonRT->Render(renderer->d2d1DeviceContext, WinWidth - gamePadOffsets[8].x, WinHeight - gamePadOffsets[8].y, buttons.rightTrigger == GamePad::ButtonStateTracker::HELD, TRUE);

                ambientLightLabel->Render(renderer->d2d1DeviceContext, WinWidth - 130, WinHeight - 20, 100.0f, 20.0f, lightsEnabled[0] ? L"Ambient:ON" : L"Ambient:OFF");
                directionalLightLabel->Render(renderer->d2d1DeviceContext, WinWidth - 80, WinHeight - 60, 100.0f, 20.0f, lightsEnabled[1] ? L"Directional:ON" : L"Directional:OFF");
                spotLightLabel->Render(renderer->d2d1DeviceContext, WinWidth - 170, WinHeight - 60, 100.0f, 20.0f, lightsEnabled[2] ? L"Spot:ON" : L"Spot:OFF");
                pointLightLabel->Render(renderer->d2d1DeviceContext, WinWidth - 125, WinHeight - 155, 100.0f, 20.0f, lightsEnabled[3] ? L"Point:ON" : L"Point:OFF");
                lightModelLabelCentered->Render(renderer->d2d1DeviceContext, WinWidth - 250, WinHeight - 110, 100.0f, 20.0f, useBlinnPhong ? L"Blinn-Phong" : L"Phong");
                shadowMapsLabel->Render(renderer->d2d1DeviceContext, 10, WinHeight - 190, 100.0f, 20.0f, shadowMapsEnabled ? L"Shadow Maps:ON" : L"Shadow Maps:OFF");
                perspectiveSelectionLabelPrevious->Render(renderer->d2d1DeviceContext, 200, 70, 100.0f, 20.0f, ((currentPerspective % 4) == 0) ? perspectiveSelection[3] : perspectiveSelection[((currentPerspective - 1) % 4)]);
                perspectiveSelectionLabelNext->Render(renderer->d2d1DeviceContext, 360, 70, 100.0f, 20.0f, ((currentPerspective % 4) == 3) ? perspectiveSelection[0] : perspectiveSelection[((currentPerspective + 1) % 4)]);
                normalMappingLabel->Render(renderer->d2d1DeviceContext, WinWidth - 100, WinHeight - 190, 100.0f, 20.0f, normalMappingEnabled ? L"Normal Maps:ON" : L"Normal Maps:OFF");
                xboxOneDPad->Render(renderer->d2d1DeviceContext, 5, WinHeight - 140, 0.4f, buttons.dpadUp == GamePad::ButtonStateTracker::HELD, buttons.dpadDown == GamePad::ButtonStateTracker::HELD, buttons.dpadLeft == GamePad::ButtonStateTracker::HELD, buttons.dpadRight == GamePad::ButtonStateTracker::HELD);
            }
            else {
                //draw the keyboard buttons (S<->D swapped because it looks better)
                //dibujar los botones del teclado (S<->D cambiados por que se ve mejor)
                keyboardButtonA->Render(renderer->d2d1DeviceContext, WinWidth * (1.0f / 8.0f), WinHeight - 40.0f, keyboardState.A);
                keyboardButtonD->Render(renderer->d2d1DeviceContext, WinWidth * (3.0f / 8.0f), WinHeight - 40.0f, keyboardState.D);
                keyboardButtonS->Render(renderer->d2d1DeviceContext, WinWidth * (2.0f / 8.0f), WinHeight - 40.0f, keyboardState.S);
                keyboardButtonP->Render(renderer->d2d1DeviceContext, WinWidth * (4.0f / 8.0f), WinHeight - 40.0f, keyboardState.P);
                keyboardButtonL->Render(renderer->d2d1DeviceContext, WinWidth * (5.0f / 8.0f), WinHeight - 40.0f, keyboardState.L);
                keyboardButtonM->Render(renderer->d2d1DeviceContext, WinWidth * (6.0f / 8.0f), WinHeight - 40.0f, keyboardState.M);
                keyboardButtonN->Render(renderer->d2d1DeviceContext, WinWidth * (7.0f / 8.0f), WinHeight - 40.0f, keyboardState.N);
                keyboardButtonPU->Render(renderer->d2d1DeviceContext, 200, 65, keyboardState.PageUp);
                keyboardButtonPD->Render(renderer->d2d1DeviceContext, 400, 65, keyboardState.PageDown);
                keyboardButtonPlus->Render(renderer->d2d1DeviceContext, 30, WinHeight - 125, keyboardState.OemPlus);
                keyboardButtonMinus->Render(renderer->d2d1DeviceContext, 30, WinHeight - 80, keyboardState.OemMinus);

                ambientLightLabel->Render(renderer->d2d1DeviceContext, WinWidth * (1.0f / 8.0f) - 30.0f, WinHeight - 20, 100.0f, 20.0f, lightsEnabled[0] ? L"Ambient:ON" : L"Ambient:OFF");
                directionalLightLabel->Render(renderer->d2d1DeviceContext, WinWidth * (3.0f / 8.0f) - 30.0f, WinHeight - 20, 100.0f, 20.0f, lightsEnabled[1] ? L"Directional:ON" : L"Directional:OFF");
                spotLightLabel->Render(renderer->d2d1DeviceContext, WinWidth * (2.0f / 8.0f) - 20.0f, WinHeight - 20, 100.0f, 20.0f, lightsEnabled[2] ? L"Spot:ON" : L"Spot:OFF");
                pointLightLabel->Render(renderer->d2d1DeviceContext, WinWidth * (4.0f / 8.0f) - 20.0f, WinHeight - 20, 100.0f, 20.0f, lightsEnabled[3] ? L"Point:ON" : L"Point:OFF");
                lightModelLabelLeft->Render(renderer->d2d1DeviceContext, WinWidth * (5.0f / 8.0f) - 20.0f, WinHeight - 20, 100.0f, 20.0f, useBlinnPhong ? L"Blinn-Phong" : L"Phong");
                shadowMapsLabel->Render(renderer->d2d1DeviceContext, WinWidth* (6.0f / 8.0f) - 40.0f, WinHeight - 20, 100.0f, 20.0f, shadowMapsEnabled ? L"Shadow Maps:ON" : L"Shadow Maps:OFF");
                normalMappingLabel->Render(renderer->d2d1DeviceContext, WinWidth* (7.0f / 8.0f) - 40.0f, WinHeight - 20, 100.0f, 20.0f, normalMappingEnabled ? L"Normal Maps:ON" : L"Normal Maps:OFF");
                perspectiveSelectionLabelPrevious->Render(renderer->d2d1DeviceContext, 180, 85, 100.0f, 20.0f, ((currentPerspective % 4) == 0) ? perspectiveSelection[3] : perspectiveSelection[((currentPerspective - 1) % 4)]);
                perspectiveSelectionLabelNext->Render(renderer->d2d1DeviceContext, 380, 85, 100.0f, 20.0f, ((currentPerspective % 4) == 3) ? perspectiveSelection[0] : perspectiveSelection[((currentPerspective + 1) % 4)]);
            }
            */

            /*
            nextAnimationLabel->Render(renderer->d2d1DeviceContext, 10, WinHeight - 160, 100.0f, 20.0f, L"Next Animation");
            previousAnimationLabel->Render(renderer->d2d1DeviceContext, 10, WinHeight - 60, 100.0f, 20.0f, L"Prev Animation");
            */
            FLOAT WinHeight = static_cast<FLOAT>(hWndRect.bottom - hWndRect.top);
            std::wstring animStr = L"Animation:" + knight->currentAnimation;
            animationLabel->Render(renderer->d2d1DeviceContext, 60, WinHeight - 90, 500, 30, animStr);
        }
        //PIXEndEvent(renderer->commandQueue.Get());
        
        renderer->Present();
    }
    PIXEndEvent(renderer->commandQueue.Get());

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto Destroy = []() -> void {
        if (renderer) {
          using namespace DeviceUtils::D3D12Device;

            Flush(renderer->commandQueue, renderer->fence, renderer->fenceValue, renderer->fenceEvent);

            /*

            knight->Destroy();
            knight = nullptr;

            for (auto &f : fire) {
                f->Destroy();
                f = nullptr;
            }
            for (auto &f : candleFlame) {
                f->Destroy();
                f = nullptr;
            }
            
            fpsText->Destroy();
            fpsText = nullptr;
            */

            gamePad.reset();
            keyboard.reset();
            /*
            xboxOneButtonA->Destroy();
            xboxOneButtonA = nullptr;
            xboxOneButtonB->Destroy();
            xboxOneButtonB = nullptr;
            xboxOneButtonX->Destroy();
            xboxOneButtonX = nullptr;
            xboxOneButtonY->Destroy();
            xboxOneButtonY = nullptr;
            xboxOneViewButton->Destroy();
            xboxOneViewButton = nullptr;
            //xboxOneDPad->Destroy();
            xboxOneDPad = nullptr;

            keyboardButtonA->Destroy();
            keyboardButtonA = nullptr;
            keyboardButtonS->Destroy();
            keyboardButtonS = nullptr;
            keyboardButtonD->Destroy();
            keyboardButtonD = nullptr;
            keyboardButtonP->Destroy();
            keyboardButtonP = nullptr;
            keyboardButtonL->Destroy();
            keyboardButtonL = nullptr;

            ambientLightLabel->Destroy();
            ambientLightLabel = nullptr;
            directionalLightLabel->Destroy();
            directionalLightLabel = nullptr;
            spotLightLabel->Destroy();
            spotLightLabel = nullptr;
            pointLightLabel->Destroy();
            pointLightLabel = nullptr;
            lightModelLabelCentered->Destroy();
            lightModelLabelCentered = nullptr;
            lightModelLabelLeft->Destroy();
            lightModelLabelLeft = nullptr;
            */

            renderer->Destroy();
            renderer = nullptr;
        }
    };

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
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

                ShowWindow(hWnd, SW_SHOWNORMAL);

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
            timer.Tick([&]() {});
            InputLoop();
            EffectsStep(static_cast<FLOAT>(timer.GetElapsedSeconds()));
            AnimableStep(timer.GetElapsedSeconds());;
            RenderLoop();
            AudioStep();
        } else {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
    }
        break;
    case WM_DESTROY:
        Destroy();
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
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}