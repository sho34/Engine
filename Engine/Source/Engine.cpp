// WindowsProject2.cpp : Defines the entry point for the application.
// Default OutDir:$(SolutionDir)$(Platform)\$(Configuration)\
//

#include "framework.h"
#include "Engine.h"
#include "Renderer/Renderer.h"
#include "Renderer/DeviceUtils.h"

#include "Primitives/Cube.h"
#include "Primitives/Pyramid.h"
#include "Primitives/Floor.h"
#include "Primitives/AnimatedQuad.h"
#include "3D/Static3DModel.h"
#include "3D/Animated3DModel.h"
#include "2D/FPSText.h"
#include "2D/XBoxOneButton.h"
#include "2D/XBoxOneViewButton.h"
#include "2D/XBoxOneTrigger.h"
#include "2D/XBoxOneBumper.h"
#include "2D/XBoxOneDPad.h"
#include "2D/KeyboardButton.h"
#include "2D/Label.h"
#include "Lights/DirectionalLight.h"
#include "Lights/SpotLight.h"
#include "Lights/PointLight.h"
#include "Lights/AmbientLight.h"
#include "Common/StepTimer.h"

using Microsoft::WRL::ComPtr;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hWnd;                                      // hWnd
RECT hWndRect;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
std::shared_ptr<Renderer> renderer;
bool inSizeMove = false;
bool inFullScreen = false;

//3d objects in scene
//objetos 3d en la escena
std::shared_ptr<Cube> cube;
std::shared_ptr<Pyramid> pyramid;
std::shared_ptr<Floor> sceneFloor;
std::shared_ptr<Static3DModel> scene;
std::shared_ptr<Animated3DModel> knight;
std::shared_ptr<AnimatedQuad> fire[2];
std::shared_ptr<AnimatedQuad> candleFlame[6];
//FPS
std::shared_ptr<FPSText> fpsText;
DX::StepTimer timer;

//Mouse
std::unique_ptr<DirectX::Mouse> mouse;
XMFLOAT2 lastMousePos;

//GamePad
std::unique_ptr<DirectX::GamePad> gamePad;
DirectX::GamePad::ButtonStateTracker buttons;
std::shared_ptr<XBoxOneButton> xboxOneButtonA;
std::shared_ptr<XBoxOneButton> xboxOneButtonB;
std::shared_ptr<XBoxOneButton> xboxOneButtonX;
std::shared_ptr<XBoxOneButton> xboxOneButtonY;
std::shared_ptr<XBoxOneViewButton> xboxOneViewButton;
std::shared_ptr<XBoxOneTrigger> xboxOneButtonLT;
std::shared_ptr<XBoxOneTrigger> xboxOneButtonRT;
std::shared_ptr<XBoxOneBumper> xboxOneButtonLB;
std::shared_ptr<XBoxOneBumper> xboxOneButtonRB;

//Keyboard
std::unique_ptr<DirectX::Keyboard> keyboard;
DirectX::Keyboard::KeyboardStateTracker keys;
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
INT currentPerspective = 0;
std::shared_ptr<Label> perspectiveSelectionLabel;
std::shared_ptr<Label> perspectiveSelectionLabelPrevious;
std::shared_ptr<Label> perspectiveSelectionLabelNext;

//animation
std::shared_ptr<Label> animationLabel;
std::shared_ptr<Label> nextAnimationLabel;
std::shared_ptr<Label> previousAnimationLabel;
std::shared_ptr<XBoxOneDPad> xboxOneDPad;

//camera properties
//propiedades de la camara
float cameraSpeed = 0.05f;
XMVECTOR	cameraPos = { -5.7f, 2.2f, 3.8f , 0.0f };
//yaw & pitch
XMFLOAT2	cameraRotations = { DirectX::XM_PIDIV2 , -DirectX::XM_PIDIV4 * 0.25f };
XMVECTOR	cameraFw() {
    return {
        sinf(cameraRotations.x) * cosf(cameraRotations.y),
        sinf(cameraRotations.y),
        cosf(cameraRotations.x) * cosf(cameraRotations.y)
    };
}
XMVECTOR	up = { 0.0f, 1.0f, 0.0f, 0.0f };
XMVECTOR	right = { 1.0f, 0.0f, 0.0f, 0.0f };

/*AMBIENT, DIRECTIONAL, SPOT, POINT*/
BOOL lightsEnabled[4] = { TRUE, TRUE, TRUE, TRUE };
/*Blinn-Phong:true*/
BOOL useBlinnPhong = TRUE;
/*SHADOW MAPS*/
BOOL shadowMapsEnabled = TRUE;
/*NORMAL MAPPING*/
BOOL normalMappingEnabled = TRUE;

//lighting
//iluminacion
std::shared_ptr<DirectionalLight> directionalLight;
std::shared_ptr<SpotLight> spotLight;
std::shared_ptr<PointLight> pointLight;
std::shared_ptr<AmbientLight> ambientLight;

//Audio
std::unique_ptr<AudioEngine> audio;
std::unique_ptr<SoundEffect> fireplaceSound;
std::unique_ptr<SoundEffectInstance> fireplaceSoundInstance;
AudioEmitter fireplaceSoundEmitter;

std::unique_ptr<SoundEffect> musicSound;
std::unique_ptr<SoundEffectInstance> musicSoundInstance;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
void                RenderLoop();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
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
            if (renderer) {
                RenderLoop();
            }
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
    
    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    
    if (!hWnd)
    {
       return FALSE;
    }
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    GetWindowRect(hWnd, &hWndRect);
    
    //intialize xbox gamepad
    //inicializar el control de xbox
    gamePad = std::make_unique<GamePad>();
    buttons.Reset();
    
    //initialize keyboard
    //inicializar el teclado
    keyboard = std::make_unique<Keyboard>();
    //keyboard->SetWindow(window);

    mouse = std::make_unique<Mouse>();
    mouse->SetWindow(hWnd);
    
    renderer = std::make_shared<Renderer>();
    renderer->Initialize(hWnd);
    
    //reset the GPU command list to start recording cube initialization commands
    //reinicia la lista de comandos de la GPU para poder cargar el cubo
    renderer->ResetCommands();

    ShaderCompiler::MonitorChanges(L"Shaders",renderer);

    //intialize the lights on the scene
    //inicializar las luces de la escena
    directionalLight = std::make_shared<DirectionalLight>();
    directionalLight->Initialize(
        //{ 0.3f, 0.3f, 0.4f, 0.0f }, //color
        { 1.0f, 1.0f, 1.0f, 0.0f }, //color
        //{ 0.03f, -0.58f }, //rotation
        { 1.54499948f, -0.300000399f },
        renderer->d3dDevice,
        4096U,
        4096U,
        32.0f, 32.0f, 1000.0f
    );

    spotLight = std::make_shared<SpotLight>();
    spotLight->Initialize(
        //{ 0.3f, 0.3f, 0.4f, 0.0f }, //color
        DirectX::Colors::WhiteSmoke,
        { -6.52f, 2.16f, 0.65f, 0.0f }, //position
        { 1.46f, -0.37f }, //rotation
        DirectX::XM_PIDIV4 * 0.5f, //fov
        { 0.0f, 0.01f, 0.02f, 0.0f }, //attenuation
        renderer->d3dDevice,
        1024U,
        1024U,
        30.0f
    );

    pointLight = std::make_shared<PointLight>();
    pointLight->Initialize(
        { 234.0f / 255.0f, 81.0f / 255.0f, 4.0f / 255.0f, 0.0f }, //color
        { -2.2f, 0.7f, 7.1f, 0.0f }, //position
        { 0.0f, 0.1f, 0.02f, 0.0f }, //attenuation
        renderer->d3dDevice,
        1024U,
        1024U,
        20.0f
    );

    ambientLight = std::make_shared<AmbientLight>();
    ambientLight->Initialize(
        { 0.2f, 0.2f, 0.2f, 0.0f }
    );
    
    //intialize 3d Objects on the scene
    //incializar los objetos 3D de la escena
    cube = std::make_shared<Cube>();
    cube->Initialize(renderer->frameCount, renderer->d3dDevice, renderer->commandList, directionalLight->shadowMap, spotLight->shadowMap, pointLight->shadowMap);
    
    pyramid = std::make_shared<Pyramid>();
    pyramid->Initialize(renderer->frameCount, renderer->d3dDevice, renderer->commandList, directionalLight->shadowMap, spotLight->shadowMap, pointLight->shadowMap);
    pyramid->position = XMVectorSetX(pyramid->position, 2.5f);
    
    sceneFloor = std::make_shared<Floor>();
    sceneFloor->Initialize(renderer->frameCount, renderer->d3dDevice, renderer->commandList, directionalLight->shadowMap, spotLight->shadowMap, pointLight->shadowMap);
    
    scene = std::make_shared<Static3DModel>();
    scene->Initialize(renderer->frameCount, renderer->d3dDevice, renderer->commandList, directionalLight->shadowMap, spotLight->shadowMap, pointLight->shadowMap, "Assets/messy_tavern/scene.gltf", { 0 });
    
    //swap axis(rotation), scaling and translating
    //rot = XMMatrixRotationRollPitchYaw(-DirectX::XM_PIDIV2, DirectX::XM_PI, DirectX::XM_PIDIV2);
    XMMATRIX world = XMMatrixSet(
        0.0f, 0.0f, 0.1f, 0.0f,
        0.1f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.1f, 0.0f, 0.0f,
        -14.0f, -1.1f, 0.0f, 1.0f
    );
    scene->world = world;

    knight = std::make_shared<Animated3DModel>();
    knight->Initialize(renderer->frameCount, renderer->d3dDevice, renderer->commandList, directionalLight->shadowMap, spotLight->shadowMap, pointLight->shadowMap, "Assets/knight/scene.gltf");
    XMMATRIX knightScaling = XMMatrixScaling(2.0f, 2.0f, 2.0f);
    XMMATRIX knightRotation = XMMatrixRotationRollPitchYaw(-DirectX::XM_PIDIV2, -DirectX::XM_PIDIV2, 0.0f);
    XMMATRIX knightPosition = XMMatrixTranslation(0.0f, -0.95f, 3.0f);
    knight->world = XMMatrixMultiply(XMMatrixMultiply(knightRotation, knightScaling), knightPosition);

    for (auto& f : fire) {
        f = std::make_shared<AnimatedQuad>();
        f->Initialize(renderer->frameCount, renderer->d3dDevice, renderer->commandList, L"Assets/fx/fire-65.dds", 50U, 0.04f, 100.0f / 255.0f);
    }
    XMMATRIX firePosition = XMMatrixTranslation(-2.2f, 0.3f, 8.0f);
    XMMATRIX fireScaling = XMMatrixScaling(1.5f, 1.5f, 1.5f);
    XMMATRIX yaw90DegRotation = XMMatrixRotationRollPitchYaw(0.0f, DirectX::XM_PIDIV2, 0.0f);
    fire[0]->world = XMMatrixMultiply(fireScaling, firePosition);
    fire[1]->world = XMMatrixMultiply(yaw90DegRotation, fire[0]->world);
    //move the time a little bit so the two textures indexes are not the same
    //movemos el tiempo un poco para que las dos indices de la texture no sean iguales
    fire[1]->currentTime = 3.0f;

    for (auto& f : candleFlame) {
        f = std::make_shared<AnimatedQuad>();
        f->Initialize(renderer->frameCount, renderer->d3dDevice, renderer->commandList, L"Assets/fx/flame.dds", 7U, 0.1f, 250.0f / 255.0f);
    }

    XMMATRIX candleScaling = XMMatrixScaling(0.075f, 0.33f, 0.075f);

    XMMATRIX candle1Position = XMMatrixTranslation(-4.55f, 4.38f, 7.65f);
    candleFlame[0]->world = XMMatrixMultiply(candleScaling, candle1Position);
    candleFlame[1]->world = XMMatrixMultiply(yaw90DegRotation, candleFlame[0]->world);

    XMMATRIX candle2Position = XMMatrixTranslation(2.24f, 1.8f, 7.39f);
    candleFlame[2]->world = XMMatrixMultiply(candleScaling, candle2Position);
    candleFlame[3]->world = XMMatrixMultiply(yaw90DegRotation, candleFlame[2]->world);

    XMMATRIX candle3Position = XMMatrixTranslation(10.29f, 1.26f, 12.36f);
    candleFlame[4]->world = XMMatrixMultiply(candleScaling, candle3Position);
    candleFlame[5]->world = XMMatrixMultiply(yaw90DegRotation, candleFlame[4]->world);
    
    //initialize 2D objects
    //inicializar objetos 2D
    fpsText = std::make_shared<FPSText>();
    fpsText->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory);
    
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
    perspectiveSelectionLabel = std::make_shared<Label>(); perspectiveSelectionLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 32, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR, D2D1::ColorF::White);
    perspectiveSelectionLabelPrevious = std::make_shared<Label>(); perspectiveSelectionLabelPrevious->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR, D2D1::ColorF::White);
    perspectiveSelectionLabelNext = std::make_shared<Label>(); perspectiveSelectionLabelNext->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 10, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR, D2D1::ColorF::White);
    animationLabel = std::make_shared<Label>(); animationLabel->Initialize(renderer->d2d1DeviceContext, renderer->dWriteFactory, 22, DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR, D2D1::ColorF::White);
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

    //execute the commands on the GPU and wait for it's completion
    //ejecuta los comandos de la GPU y esperar a que termine
    renderer->CloseCommandsAndFlush();
    
    cube->DestroyUploadResources();
    pyramid->DestroyUploadResources();
    sceneFloor->DestroyUploadResources();
    scene->DestroyUploadResources();
    //knight->DestroyUploadResources();
    
    for (auto f : fire) {
        f->DestroyUploadResources();
    }
    for (auto f : candleFlame) {
        f->DestroyUploadResources();
    }
    return TRUE;
}

void RenderLoop()
{
    if (!inFullScreen)
    {
        GetWindowRect(hWnd, &hWndRect);
    }

    timer.Tick([&]() {});

    //get the gamepad and keyboard states and update their trackers
    //obtener los estados del gamepad y del teclado y actualizar sus trackers
    auto gamePadState = gamePad->GetState(0);
    auto keyboardState = keyboard->GetState();
    auto mouseState = mouse->GetState();
    buttons.Update(gamePadState);
    keys.Update(keyboardState);

    //check if xbox gamepad is connected
    //chequear si esta conectado el gamepad de la xbox
    if (gamePadState.IsConnected()) {
        if (buttons.a == GamePad::ButtonStateTracker::RELEASED) { lightsEnabled[0] = !lightsEnabled[0]; }
        if (buttons.b == GamePad::ButtonStateTracker::RELEASED) { lightsEnabled[1] = !lightsEnabled[1]; }
        if (buttons.x == GamePad::ButtonStateTracker::RELEASED) { lightsEnabled[2] = !lightsEnabled[2]; }
        if (buttons.y == GamePad::ButtonStateTracker::RELEASED) { lightsEnabled[3] = !lightsEnabled[3]; }
        if (buttons.view == GamePad::ButtonStateTracker::RELEASED) { useBlinnPhong = !useBlinnPhong; }
        if (buttons.leftTrigger == GamePad::ButtonStateTracker::RELEASED) { shadowMapsEnabled = !shadowMapsEnabled; }
        if (buttons.rightTrigger == GamePad::ButtonStateTracker::RELEASED) { normalMappingEnabled = !normalMappingEnabled; }
        if (buttons.rightShoulder == GamePad::ButtonStateTracker::RELEASED) { currentPerspective = (currentPerspective + 1) % 4; }
        if (buttons.leftShoulder == GamePad::ButtonStateTracker::RELEASED) { currentPerspective = (currentPerspective != 0) ? (currentPerspective - 1) : 3; }
        if (buttons.dpadUp == GamePad::ButtonStateTracker::RELEASED) { PreviousAnimation(knight.get()); }
        if (buttons.dpadDown == GamePad::ButtonStateTracker::RELEASED) { NextAnimation(knight.get()); }

        //control the camera using the gamepad
        //controlar la camara usando el gamepad
        if (currentPerspective % 4 == 0) {
            XMVECTOR fw = cameraFw();
            if (gamePadState.thumbSticks.leftY > 0) { cameraPos = XMVectorAdd(cameraPos, fw * cameraSpeed); }
            if (gamePadState.thumbSticks.leftY < 0) { cameraPos = XMVectorAdd(cameraPos, fw * -cameraSpeed); }
            if (gamePadState.thumbSticks.leftX < 0) { cameraPos = XMVectorAdd(cameraPos, XMVector3Cross(fw, up) * -cameraSpeed); }
            if (gamePadState.thumbSticks.leftX > 0) { cameraPos = XMVectorAdd(cameraPos, XMVector3Cross(fw, up) * cameraSpeed); }
        }
        else if (currentPerspective % 4 == 2) {
            XMVECTOR fw = spotLight->directionAndAngle(); XMVectorSetW(fw, 0.0f);
            if (gamePadState.thumbSticks.leftY > 0) { spotLight->position = XMVectorAdd(spotLight->position, fw * cameraSpeed); }
            if (gamePadState.thumbSticks.leftY < 0) { spotLight->position = XMVectorAdd(spotLight->position, fw * -cameraSpeed); }
            if (gamePadState.thumbSticks.leftX < 0) { spotLight->position = XMVectorAdd(spotLight->position, XMVector3Cross(fw, up) * -cameraSpeed); }
            if (gamePadState.thumbSticks.leftX > 0) { spotLight->position = XMVectorAdd(spotLight->position, XMVector3Cross(fw, up) * cameraSpeed); }
        }
        else if (currentPerspective % 4 == 3) {
            XMVECTOR fw = cameraFw();
            if (gamePadState.thumbSticks.leftY > 0) { pointLight->position = XMVectorAdd(pointLight->position, fw * cameraSpeed); }
            if (gamePadState.thumbSticks.leftY < 0) { pointLight->position = XMVectorAdd(pointLight->position, fw * -cameraSpeed); }
            if (gamePadState.thumbSticks.leftX < 0) { pointLight->position = XMVectorAdd(pointLight->position, XMVector3Cross(fw, up) * -cameraSpeed); }
            if (gamePadState.thumbSticks.leftX > 0) { pointLight->position = XMVectorAdd(pointLight->position, XMVector3Cross(fw, up) * cameraSpeed); }
        }
    } else {
        //otherwise use the keyboard
        //en caso contrario usamos el teclado
        if (keys.pressed.A) { lightsEnabled[0] = !lightsEnabled[0]; }
        if (keys.pressed.D) { lightsEnabled[1] = !lightsEnabled[1]; }
        if (keys.pressed.S) { lightsEnabled[2] = !lightsEnabled[2]; }
        if (keys.pressed.P) { lightsEnabled[3] = !lightsEnabled[3]; }
        if (keys.pressed.L) { useBlinnPhong = !useBlinnPhong; }
        if (keys.pressed.M) { shadowMapsEnabled = !shadowMapsEnabled; }
        if (keys.pressed.N) { normalMappingEnabled = !normalMappingEnabled; }
        if (keys.pressed.PageDown) { currentPerspective = (currentPerspective + 1) % 4; }
        if (keys.pressed.PageUp) { currentPerspective = (currentPerspective != 0) ? (currentPerspective - 1) : 3; }

        //cycle animations
        //ciclar las animaciones
        if (keys.pressed.OemPlus) {
            NextAnimation(knight.get());
        }
        if (keys.pressed.OemMinus) {
            PreviousAnimation(knight.get());
        }

        //control the camera using the keyboard
        //controlar la camara usando el teclado
        if (currentPerspective % 4 == 0) {
            XMVECTOR fw = cameraFw();
            if (keyboardState.Up) { cameraPos = XMVectorAdd(cameraPos, fw * cameraSpeed); }
            if (keyboardState.Down) { cameraPos = XMVectorAdd(cameraPos, fw * -cameraSpeed); }
            if (keyboardState.Left) { cameraPos = XMVectorAdd(cameraPos, XMVector3Cross(fw, up) * -cameraSpeed); }
            if (keyboardState.Right) { cameraPos = XMVectorAdd(cameraPos, XMVector3Cross(fw, up) * cameraSpeed); }
        }
        else if (currentPerspective % 4 == 2) {
            XMVECTOR fw = spotLight->directionAndAngle(); XMVectorSetW(fw, 0.0f);
            if (keyboardState.Up) { spotLight->position = XMVectorAdd(spotLight->position, fw * cameraSpeed); }
            if (keyboardState.Down) { spotLight->position = XMVectorAdd(spotLight->position, fw * -cameraSpeed); }
            if (keyboardState.Left) { spotLight->position = XMVectorAdd(spotLight->position, XMVector3Cross(fw, up) * -cameraSpeed); }
            if (keyboardState.Right) { spotLight->position = XMVectorAdd(spotLight->position, XMVector3Cross(fw, up) * cameraSpeed); }
        }
        else if (currentPerspective % 4 == 3) {
            XMVECTOR fw = cameraFw();
            if (keyboardState.Up) { pointLight->position = XMVectorAdd(pointLight->position, fw * cameraSpeed); }
            if (keyboardState.Down) { pointLight->position = XMVectorAdd(pointLight->position, fw * -cameraSpeed); }
            if (keyboardState.Left) { pointLight->position = XMVectorAdd(pointLight->position, XMVector3Cross(fw, up) * -cameraSpeed); }
            if (keyboardState.Right) { pointLight->position = XMVectorAdd(pointLight->position, XMVector3Cross(fw, up) * cameraSpeed); }
        }
    }

    //track the current mouse position
    //hacer seguimiento de la posicion actual del mouse
    XMFLOAT2 currentMousePos = { static_cast<FLOAT>(mouseState.x) , static_cast<FLOAT>(mouseState.y) };

    //only rotate the camera if the left button is pressed and there is no gamepad
    //solo rota la camara si el boton izquierdo esta presionado y no hay un gamepad
    if (!gamePadState.IsConnected() && mouseState.leftButton) {
        XMFLOAT2 diff = { currentMousePos.x - lastMousePos.x , currentMousePos.y - lastMousePos.y };
        if (currentPerspective % 4 == 0 || currentPerspective % 4 == 3) {
            cameraRotations.x -= diff.x * 0.001f;
            cameraRotations.y -= diff.y * 0.001f;
        }
        else if (currentPerspective % 4 == 1) {
            directionalLight->rotation.x -= diff.x * 0.001f;
            directionalLight->rotation.y -= diff.y * 0.001f;
        }
        else if (currentPerspective % 4 == 2) {
            spotLight->rotation.x -= diff.x * 0.001f;
            spotLight->rotation.y -= diff.y * 0.001f;
        }
    }
    else if (gamePadState.IsConnected()) {
        if (currentPerspective % 4 == 0 || currentPerspective % 4 == 3) {
            cameraRotations.x -= gamePadState.thumbSticks.rightX * 0.05f;
            cameraRotations.y += gamePadState.thumbSticks.rightY * 0.05f;
        }
        else if (currentPerspective % 4 == 1) {
            directionalLight->rotation.x -= gamePadState.thumbSticks.rightX * 0.05f;
            directionalLight->rotation.y += gamePadState.thumbSticks.rightY * 0.05f;
        }
        else if (currentPerspective % 4 == 2) {
            spotLight->rotation.x -= gamePadState.thumbSticks.rightX * 0.05f;
            spotLight->rotation.y += gamePadState.thumbSticks.rightY * 0.05f;
        }
    }

    //store the current mouse position for the next frame
    //guarda la posicion del mouse para el siguiente frame
    lastMousePos = currentMousePos;

    //update cube && pyramid
    cube->Step();
    pyramid->Step();
    //update the animated meshes
    //assimp animations (at least for this model) are in miliseconds
    //las animaciones de assimp (por lo menos en este modelo) estan en milisegundos
    knight->Step(static_cast<FLOAT>(timer.GetElapsedSeconds() * 1000.0f));

    for (auto f : fire) {
        f->Step(static_cast<FLOAT>(timer.GetElapsedSeconds()));
    }
    for (auto f : candleFlame) {
        f->Step(static_cast<FLOAT>(timer.GetElapsedSeconds()));
    }
    renderer->ResetCommands();

    PIXBeginEvent(renderer->commandQueue.Get(), 0, L"Render");
    {
        XMMATRIX directionalLightShadowMapViewProjection;
        XMMATRIX spotLightShadowMapViewProjection;
        XMMATRIX pointLightShadowMapViewProjection[6];
        XMVECTOR shadowMapCameraPosition;

        PIXBeginEvent(renderer->commandList.Get(), 0, L"Directional Light ShadowMap");
        {
            renderer->SetShadowMapTarget(
                directionalLight->shadowMap,
                directionalLight->shadowMapDSVDescriptorHeap,
                directionalLight->shadowMapScissorRect,
                directionalLight->shadowMapViewport
            );

            XMVECTOR directionalShadowMapDirection = directionalLight->direction();
            shadowMapCameraPosition = XMVectorScale(XMVector3Normalize(directionalShadowMapDirection), -30.0f);
            XMMATRIX shadowMapView = XMMatrixLookAtRH(shadowMapCameraPosition, XMVectorZero(), (abs(directionalShadowMapDirection.m128_f32[2]) < 0.9999f) ? up : right);
            directionalLightShadowMapViewProjection = XMMatrixMultiply(shadowMapView, directionalLight->shadowMapPerspectiveMatrix);

            {
                std::lock_guard<std::mutex> lock(cube->shaderMutex);
                cube->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, directionalLightShadowMapViewProjection, cube->directionalLightShadowMapCbvData);
                cube->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, cube->directionalLightShadowMapCbvData);
            }
            
            {
                std::lock_guard<std::mutex> lock(pyramid->shaderMutex);
                pyramid->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, directionalLightShadowMapViewProjection, pyramid->directionalLightShadowMapCbvData);
                pyramid->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, pyramid->directionalLightShadowMapCbvData);
            }
            
            {
                std::lock_guard<std::mutex> lock(scene->shaderMutex);
                scene->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, directionalLightShadowMapViewProjection, scene->directionalLightShadowMapCbvData);
                scene->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, scene->directionalLightShadowMapCbvData);
            }
            
            {
                std::lock_guard<std::mutex> lock(knight->shaderMutex);
                knight->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, directionalLightShadowMapViewProjection, knight->directionalLightShadowMapCbvData);
                knight->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, knight->directionalLightShadowMapCbvData);
            }
        }
        PIXEndEvent(renderer->commandList.Get());

        PIXBeginEvent(renderer->commandList.Get(), 0, L"Spot Light ShadowMap");
        {
            renderer->SetShadowMapTarget(
                spotLight->shadowMap,
                spotLight->shadowMapDSVDescriptorHeap,
                spotLight->shadowMapScissorRect,
                spotLight->shadowMapViewport
            );

            XMMATRIX view = XMMatrixLookToRH(spotLight->position, spotLight->directionAndAngle(), up);
            spotLightShadowMapViewProjection = XMMatrixMultiply(view, spotLight->shadowMapPerspectiveMatrix);

            {
                std::lock_guard<std::mutex> lock(cube->shaderMutex);
                cube->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, spotLightShadowMapViewProjection, cube->spotLightShadowMapCbvData);
                cube->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, cube->spotLightShadowMapCbvData);
            }
            
            {
                std::lock_guard<std::mutex> lock(pyramid->shaderMutex);
                pyramid->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, spotLightShadowMapViewProjection, pyramid->spotLightShadowMapCbvData);
                pyramid->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, pyramid->spotLightShadowMapCbvData);
            }
            
            {
                std::lock_guard<std::mutex> lock(scene->shaderMutex);
                scene->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, spotLightShadowMapViewProjection, scene->spotLightShadowMapCbvData);
                scene->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, scene->spotLightShadowMapCbvData);
            }

            {
                std::lock_guard<std::mutex> lock(knight->shaderMutex);
                knight->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, spotLightShadowMapViewProjection, knight->spotLightShadowMapCbvData);
                knight->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, knight->spotLightShadowMapCbvData);
            }
        }
        PIXEndEvent(renderer->commandList.Get());

        PIXBeginEvent(renderer->commandList.Get(), 0, L"Point Light ShadowMap");
        {
            renderer->SetShadowMapTarget(
                pointLight->shadowMap,
                pointLight->shadowMapDSVDescriptorHeap,
                pointLight->shadowMapClearScissorRect,
                pointLight->shadowMapClearViewport
            );

            for (UINT i = 0; i < 6; i++) {
                renderer->commandList->RSSetViewports(1, &pointLight->shadowMapViewport[i]);
                renderer->commandList->RSSetScissorRects(1, &pointLight->shadowMapScissorRect[i]);

                XMMATRIX view = XMMatrixLookToRH(pointLight->position, pointLight->direction[i], pointLight->up[i]);
                pointLightShadowMapViewProjection[i] = XMMatrixMultiply(view, pointLight->shadowMapPerspectiveMatrix);

                {
                    std::lock_guard<std::mutex> lock(cube->shaderMutex);
                    cube->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, pointLightShadowMapViewProjection[i], cube->pointLightShadowMapCbvData[i]);
                    cube->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, cube->pointLightShadowMapCbvData[i]);
                }
                
                {
                    std::lock_guard<std::mutex> lock(pyramid->shaderMutex);
                    pyramid->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, pointLightShadowMapViewProjection[i], pyramid->pointLightShadowMapCbvData[i]);
                    pyramid->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, pyramid->pointLightShadowMapCbvData[i]);
                }
                
                {
                    std::lock_guard<std::mutex> lock(scene->shaderMutex);
                    scene->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, pointLightShadowMapViewProjection[i], scene->pointLightShadowMapCbvData[i]);
                    scene->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, scene->pointLightShadowMapCbvData[i]);
                }

                {
                    std::lock_guard<std::mutex> lock(knight->shaderMutex);
                    knight->UpdateShadowMapConstantsBuffer(renderer->backBufferIndex, pointLightShadowMapViewProjection[i], knight->pointLightShadowMapCbvData[i]);
                    knight->RenderShadowMap(renderer->commandList, renderer->backBufferIndex, knight->pointLightShadowMapCbvData[i]);
                }
            }
        }
        PIXEndEvent(renderer->commandList.Get());

        PIXBeginEvent(renderer->commandList.Get(), 0, L"Render Scene");
        {
            renderer->SetRenderTargets();

            XMMATRIX view, viewProjection;
            XMVECTOR cameraPosition;

            if (currentPerspective % 4 == 0) {
                view = XMMatrixLookToRH(cameraPos, cameraFw(), up);
                viewProjection = XMMatrixMultiply(view, renderer->perspectiveMatrix);
                cameraPosition = cameraPos;
            }
            else if (currentPerspective % 4 == 1) {
                viewProjection = directionalLightShadowMapViewProjection;
                cameraPosition = shadowMapCameraPosition;
            }
            else if (currentPerspective % 4 == 2) {
                viewProjection = spotLightShadowMapViewProjection;
                cameraPosition = spotLight->position;
            }
            else if (currentPerspective % 4 == 3) {
                view = XMMatrixLookToRH(pointLight->position, cameraFw(), up);
                viewProjection = XMMatrixMultiply(view, pointLight->shadowMapPerspectiveMatrix);
                cameraPosition = pointLight->position;
            }
            float flameLightScale = 0.9f + 0.1f * cosf(static_cast<FLOAT>(timer.GetTotalSeconds()) * 45.0f);

            //update cube constants and draw
            //actualizar las constantes del cubo y dibujarlo
            {
                std::lock_guard<std::mutex> lock(cube->shaderMutex);
                cube->UpdateConstantsBuffer(
                    renderer->backBufferIndex,
                    useBlinnPhong,
                    viewProjection,
                    cameraPosition,
                    lightsEnabled[0] ? ambientLight->color : XMVectorZero(),
                    directionalLight->direction(),
                    lightsEnabled[1] ? directionalLight->color : XMVectorZero(),
                    spotLight->position,
                    lightsEnabled[2] ? spotLight->color : XMVectorZero(),
                    spotLight->directionAndAngle(),
                    spotLight->attenuation,
                    lightsEnabled[3] ? pointLight->color : XMVectorZero(),
                    pointLight->position,
                    pointLight->attenuation,
                    shadowMapsEnabled,
                    directionalLightShadowMapViewProjection,
                    directionalLight->shadowMapTexelInvSize,
                    spotLightShadowMapViewProjection,
                    spotLight->shadowMapTexelInvSize,
                    pointLightShadowMapViewProjection
                );
                cube->Render(renderer->commandList, renderer->backBufferIndex);
            }

            //update pyramid constants and draw
            //actualizar las constantes de la piramide y dibujarla
            {
                std::lock_guard<std::mutex> lock(pyramid->shaderMutex);
                pyramid->UpdateConstantsBuffer(
                    renderer->backBufferIndex,
                    useBlinnPhong,
                    viewProjection,
                    cameraPosition,
                    lightsEnabled[0] ? ambientLight->color : XMVectorZero(),
                    directionalLight->direction(),
                    lightsEnabled[1] ? directionalLight->color : XMVectorZero(),
                    spotLight->position,
                    lightsEnabled[2] ? spotLight->color : XMVectorZero(),
                    spotLight->directionAndAngle(),
                    spotLight->attenuation,
                    lightsEnabled[3] ? pointLight->color : XMVectorZero(),
                    pointLight->position,
                    pointLight->attenuation,
                    shadowMapsEnabled,
                    directionalLightShadowMapViewProjection,
                    directionalLight->shadowMapTexelInvSize,
                    spotLightShadowMapViewProjection,
                    spotLight->shadowMapTexelInvSize,
                    pointLightShadowMapViewProjection,
                    normalMappingEnabled
                );
                pyramid->Render(renderer->commandList, renderer->backBufferIndex);
            }

            {
                //update floor constants and draw
                //actualizar las constantes del suelo y dibujarlo
                std::lock_guard<std::mutex> lock(sceneFloor->shaderMutex);
                sceneFloor->UpdateConstantsBuffer(
                    renderer->backBufferIndex,
                    useBlinnPhong,
                    viewProjection,
                    cameraPosition,
                    lightsEnabled[0] ? ambientLight->color : XMVectorZero(),
                    directionalLight->direction(),
                    lightsEnabled[1] ? directionalLight->color : XMVectorZero(),
                    spotLight->position,
                    lightsEnabled[2] ? spotLight->color : XMVectorZero(),
                    spotLight->directionAndAngle(),
                    spotLight->attenuation,
                    lightsEnabled[3] ? pointLight->color : XMVectorZero(),
                    pointLight->position,
                    pointLight->attenuation,
                    shadowMapsEnabled,
                    directionalLightShadowMapViewProjection,
                    directionalLight->shadowMapTexelInvSize,
                    spotLightShadowMapViewProjection,
                    spotLight->shadowMapTexelInvSize,
                    pointLightShadowMapViewProjection
                );
                sceneFloor->Render(renderer->commandList, renderer->backBufferIndex);
            }

            {
                std::lock_guard<std::mutex> lock(scene->shaderMutex);
                scene->UpdateConstantsBuffer(
                    renderer->backBufferIndex,
                    useBlinnPhong,
                    viewProjection,
                    cameraPosition,
                    lightsEnabled[0] ? ambientLight->color : XMVectorZero(),
                    directionalLight->direction(),
                    lightsEnabled[1] ? directionalLight->color : XMVectorZero(),
                    spotLight->position,
                    lightsEnabled[2] ? spotLight->color : XMVectorZero(),
                    spotLight->directionAndAngle(),
                    spotLight->attenuation,
                    lightsEnabled[3] ? XMVectorScale(pointLight->color, flameLightScale) : XMVectorZero(),
                    pointLight->position,
                    pointLight->attenuation,
                    shadowMapsEnabled,
                    directionalLightShadowMapViewProjection,
                    directionalLight->shadowMapTexelInvSize,
                    spotLightShadowMapViewProjection,
                    spotLight->shadowMapTexelInvSize,
                    pointLightShadowMapViewProjection,
                    normalMappingEnabled
                );
                scene->Render(renderer->commandList, renderer->backBufferIndex);
            }

            {
                std::lock_guard<std::mutex> lock(knight->shaderMutex);
                knight->UpdateConstantsBuffer(
                    renderer->backBufferIndex,
                    viewProjection,
                    cameraPosition,
                    lightsEnabled[0] ? ambientLight->color : XMVectorZero(),
                    directionalLight->direction(),
                    lightsEnabled[1] ? directionalLight->color : XMVectorZero(),
                    spotLight->position,
                    lightsEnabled[2] ? spotLight->color : XMVectorZero(),
                    spotLight->directionAndAngle(),
                    spotLight->attenuation,
                    lightsEnabled[3] ? XMVectorScale(pointLight->color, flameLightScale) : XMVectorZero(),
                    pointLight->position,
                    pointLight->attenuation,
                    shadowMapsEnabled,
                    directionalLightShadowMapViewProjection,
                    directionalLight->shadowMapTexelInvSize,
                    spotLightShadowMapViewProjection,
                    spotLight->shadowMapTexelInvSize,
                    pointLightShadowMapViewProjection,
                    normalMappingEnabled
                );
                knight->Render(renderer->commandList, renderer->backBufferIndex);
            }

            for (auto f : fire) {
                {
                    std::lock_guard<std::mutex> lock(f->shaderMutex);
                    f->UpdateConstantsBuffer(renderer->backBufferIndex, viewProjection);
                    f->Render(renderer->commandList, renderer->backBufferIndex);
                }
            }
            for (auto f : candleFlame) {
                {
                    std::lock_guard<std::mutex> lock(f->shaderMutex);
                    f->UpdateConstantsBuffer(renderer->backBufferIndex, viewProjection);
                    f->Render(renderer->commandList, renderer->backBufferIndex);
                }
            }
        }
        PIXEndEvent(renderer->commandList.Get());

        //before switching to 2D mode the commandQueue must be executed
        //antes de pasar a modo 2D el commandQueue debe ser ejecutado
        renderer->ExecuteCommands();

        //PIXBeginEvent(renderer->commandQueue.Get(), 0, L"Render UI");
        {
            //switch to 2d mode
            //cambiar a modo 2D
            renderer->Set2DRenderTarget();

            //draw the framerate
            //dibujar el framerate
            fpsText->Render(renderer->d2d1DeviceContext, timer.GetFramesPerSecond());

            const LPWSTR perspectiveSelection[] = {
                (LPWSTR)L"Scene Camera",
                (LPWSTR)L"Directional Light",
                (LPWSTR)L"Spot Light",
                (LPWSTR)L"Point Light"
            };
            perspectiveSelectionLabel->Render(renderer->d2d1DeviceContext, 200, 10, 500, 300, perspectiveSelection[currentPerspective % _countof(perspectiveSelection)]);

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

            nextAnimationLabel->Render(renderer->d2d1DeviceContext, 10, WinHeight - 160, 100.0f, 20.0f, L"Next Animation");
            previousAnimationLabel->Render(renderer->d2d1DeviceContext, 10, WinHeight - 60, 100.0f, 20.0f, L"Prev Animation");

            std::string animStr = "Animation:" + knight->currentAnimation;
            animationLabel->Render(renderer->d2d1DeviceContext, 60, WinHeight - 90, 500, 30, std::wstring(animStr.begin(), animStr.end()));
        }
        //PIXEndEvent(renderer->commandQueue.Get());

        renderer->Present();
    }
    PIXEndEvent(renderer->commandQueue.Get());

    if (audio->Update()) {
        //adjust the listener position based on the camera properties
        //ajustamos la posicion del oyente basado en las propiedades de la camara
        AudioListener listener;
        XMVECTOR listenerPosition;
        XMVECTOR fw;
        if (currentPerspective % 4 == 0) {
            listenerPosition = cameraPos;
            fw = cameraFw();
        }
        else if (currentPerspective % 4 == 1) {
            listenerPosition = XMVectorScale(XMVector3Normalize(directionalLight->direction()), -30.0f);
            fw = directionalLight->direction();
        }
        else if (currentPerspective % 4 == 2) {
            listenerPosition = spotLight->position;
            fw = spotLight->directionAndAngle();
        }
        else if (currentPerspective % 4 == 3) {
            listenerPosition = pointLight->position;
            fw = cameraFw();
        }
        listener.SetPosition(listenerPosition);
        listener.SetOrientation(fw, up);
        fireplaceSoundInstance->Apply3D(listener, fireplaceSoundEmitter);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto Destroy = []() -> void {
        if (renderer) {
            Flush(renderer->commandQueue, renderer->fence, renderer->fenceValue, renderer->fenceEvent);
            cube->Destroy();
            cube = nullptr;
            
            pyramid->Destroy();
            pyramid = nullptr;

            sceneFloor->Destroy();
            sceneFloor = nullptr;

            scene->Destroy();
            scene = nullptr;

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

            gamePad.reset();
            keyboard.reset();
            
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
            RenderLoop();
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
