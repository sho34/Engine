#include "pch.h"
#include "Engine.h"
#include "Game.h"
#include "Effects/Effects.h"

using namespace Scene;
using namespace ShaderCompiler;
using namespace Effects;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hWnd;                                      // Window hWnd
RECT hWndRect;																	// Window Rect (this can change so if you change the size of the window please rewrite this value)
HWND desktopHwnd;																// Desktop hWnd (how do you enumerate desktops with this?)
RECT desktopRect;																// Desktop Rect								
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
bool appDone = false;
bool inSizeMove = false;
bool resizeWindow = false;
unsigned int resizeWidth = 0;
unsigned int resizeHeight = 0;
bool inFullScreen = false;
#if defined(_EDITOR)
bool editorPlayMode = false;
#endif
extern std::string gameAppTitle;

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
int currentCamera = 0;
float cameraSpeed = 0.05f;
XMFLOAT2 mouseCameraRotationSensitivity = { 0.001f, 0.001f };
XMFLOAT2 gamePadCameraRotationSensitivity = { 0.02f, -0.02f };

//app destruction
bool destroyed = false;

RECT GetMaximizedAreaSize()
{
	HMONITOR monitor = MonitorFromWindow(desktopHwnd, MONITOR_DEFAULTTONULL);
	MONITORINFO monitor_info{};
	monitor_info.cbSize = sizeof(monitor_info);
	GetMonitorInfoW(monitor, &monitor_info);
	return monitor_info.rcWork;
}

void ResetWindowStyle(bool fullscreen)
{
	SetWindowLong(hWnd, GWL_STYLE, 0);

#if !defined(_EDITOR)
	if (fullscreen)
	{
		SetWindowLongPtr(hWnd, GWL_STYLE, WS_MAXIMIZE);
		SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

		ShowWindow(hWnd, SW_SHOWMAXIMIZED);

		SetWindowPos(hWnd, HWND_TOP, desktopRect.left, desktopRect.top, desktopRect.right - desktopRect.left, desktopRect.bottom - desktopRect.top, SWP_NOZORDER | SWP_FRAMECHANGED);
	}
	else
	{
		//SetWindowLongPtr(hWnd, GWL_STYLE, /*WS_POPUP |*/ WS_OVERLAPPEDWINDOW);
		SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
		SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

		SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowWindow(hWnd, SW_NORMAL);
	}
#else
	SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
	SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

	ShowWindow(hWnd, SW_NORMAL);
	RECT winR = GetMaximizedAreaSize();
	SetWindowPos(hWnd, HWND_TOP, winR.left, winR.top, winR.right, winR.bottom, SWP_NOZORDER | SWP_FRAMECHANGED);
#endif
}

//CREATE
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	desktopHwnd = GetDesktopWindow();
	GetClientRect(desktopHwnd, &desktopRect);

	// Initialize global strings
	//LoadStringW(hInstance, IDS_APP_TITLE, gameAppTitle.c_str(), MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WINDOWSPROJECT2, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
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

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT2));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	//wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT2);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	/*this is mode full desktop space*/
#if defined(_EDITOR)
	RECT winR = GetMaximizedAreaSize();// desktopRect;
	hWnd = CreateWindowW(szWindowClass, nostd::StringToWString(gameAppTitle).c_str(), WS_OVERLAPPEDWINDOW, winR.left, winR.top, winR.right, winR.bottom, nullptr, nullptr, hInstance, nullptr);
#endif

	/*this is windowed mode*/
#if !defined(_EDITOR)
	hWnd = CreateWindowW(szWindowClass, nostd::StringToWString(gameAppTitle).c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
#endif

	if (!hWnd) return FALSE;

	ResetWindowStyle(inFullScreen);
	//ShowWindow(hWnd, nCmdShow);

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
	MonitorShaderChanges(defaultShadersFolder);

	//Initialize the audio system
	InitAudio();

	//initialize the render and reset the commands
	renderer = std::make_shared<Renderer>();
	renderer->Initialize(hWnd);
	renderer->ResetCommands();

	//create the templates
	CreateSystemTemplates();
	CreateTemplates();
	CreateLightingResourcesMapping();

	SetupRenderPipeline();

	//create the editor and a default scene
#if defined(_EDITOR)
	InitEditor();
#endif

	//kick the audio listener update
	AudioStep();

	//execute the commands on the GPU and wait for it's completion
	renderer->CloseCommandsAndFlush();

	return TRUE;
}

void CreateSystemTemplates() {
	using namespace Templates;

	CreateShader("BoundingBox", R"( {
		"systemCreated" : true,
		"mappedValues": [
			{ "value": [ 1.0, 0.0, 0.0 ], "variable": "baseColor", "variableType": "FLOAT3" }
		]
	})"_json);
	CreateShader("BaseLighting", R"( {
		"systemCreated" : true,
		"mappedValues": [
			{ "value": [ 0.11764706671237946, 0.5647059082984924, 1.0 ], "variable": "baseColor", "variableType": "FLOAT3" },
			{ "value": 400.0, "variable": "specularExponent", "variableType": "FLOAT" }
		]
	})"_json);
	CreateShader("Grid", R"( {
		"systemCreated" : true,
		"mappedValues": [
			{ "value": [ 1.0, 0.0, 1.0 ], "variable": "baseColor", "variableType": "RGB" },
			{ "value": 1024.0, "variable": "specularExponent", "variableType": "FLOAT" }
		]
	})"_json);
	CreateShader("ShadowMap", R"( { "systemCreated" : true })"_json);
	CreateShader("DepthMinMax", R"( { "systemCreated" : true })"_json);
	CreateShader("DepthMinMaxToRGBA", R"( { "systemCreated" : true })"_json);
	CreateShader("FullScreenQuad", R"( { "systemCreated" : true })"_json);
	CreateShader("LoadingBar", R"( { "systemCreated" : true })"_json);
	CreateMaterial("BoundingBox", R"({ "shader":"BoundingBox", "systemCreated":true })"_json);
	CreateMaterial("BaseLighting", R"({ "shader":"BaseLighting", "systemCreated":true, "samplers":[{ "Filter":"MIN_MAG_MIP_LINEAR", "AddressU":"ADDRESS_MODE_BORDER", "AddressV":"ADDRESS_MODE_BORDER", "AddressW":"ADDRESS_MODE_BORDER", "MipLODBias":0, "MaxAnisotropy":0, "ComparisonFunc":"NEVER", "BorderColor":"OPAQUE_WHITE", "MinLOD":0.0, "MaxLOD":3.4028234663852886e+38, "ShaderRegister":0, "RegisterSpace":0, "ShaderVisibility":"PIXEL" }] })"_json);
	CreateMaterial("Floor", R"({ "shader":"Grid", "systemCreated":true, "samplers":[{ "Filter":"MIN_MAG_MIP_LINEAR", "AddressU":"ADDRESS_MODE_BORDER", "AddressV":"ADDRESS_MODE_BORDER", "AddressW":"ADDRESS_MODE_BORDER", "MipLODBias":0, "MaxAnisotropy":0, "ComparisonFunc":"NEVER", "BorderColor":"OPAQUE_WHITE", "MinLOD":0.0, "MaxLOD":3.4028234663852886e+38, "ShaderRegister":0, "RegisterSpace":0, "ShaderVisibility":"PIXEL" }] })"_json);
	CreateMaterial("ShadowMap", R"({ "shader":"ShadowMap", "systemCreated":true, "samplers":[{ "Filter":"MIN_MAG_MIP_LINEAR", "AddressU":"ADDRESS_MODE_BORDER", "AddressV":"ADDRESS_MODE_BORDER", "AddressW":"ADDRESS_MODE_BORDER", "MipLODBias":0, "MaxAnisotropy":0, "ComparisonFunc":"NEVER", "BorderColor":"OPAQUE_WHITE", "MinLOD":0.0, "MaxLOD":3.4028234663852886e+38, "ShaderRegister":0, "RegisterSpace":0, "ShaderVisibility":"PIXEL" }] })"_json);
	CreateMaterial("DepthMinMax", R"({ "shader":"DepthMinMax", "systemCreated":true, "twoSided": true, "samplers":[{ "Filter":"MIN_MAG_MIP_POINT", "AddressU":"ADDRESS_MODE_BORDER", "AddressV":"ADDRESS_MODE_BORDER", "AddressW":"ADDRESS_MODE_BORDER", "MipLODBias":0, "MaxAnisotropy":0, "ComparisonFunc":"NEVER", "BorderColor":"OPAQUE_WHITE", "MinLOD":0.0, "MaxLOD":3.4028234663852886e+38, "ShaderRegister":0, "RegisterSpace":0, "ShaderVisibility":"PIXEL" }] })"_json);
	CreateMaterial("DepthMinMaxToRGBA", R"({ "shader":"DepthMinMaxToRGBA", "systemCreated":true, "twoSided": true, "samplers":[{ "Filter":"MIN_MAG_MIP_POINT", "AddressU":"ADDRESS_MODE_BORDER", "AddressV":"ADDRESS_MODE_BORDER", "AddressW":"ADDRESS_MODE_BORDER", "MipLODBias":0, "MaxAnisotropy":0, "ComparisonFunc":"NEVER", "BorderColor":"OPAQUE_WHITE", "MinLOD":0.0, "MaxLOD":3.4028234663852886e+38, "ShaderRegister":0, "RegisterSpace":0, "ShaderVisibility":"PIXEL" }] })"_json);
	CreateMaterial("FullScreenQuad", R"({ "shader":"FullScreenQuad", "systemCreated":true})"_json);
	CreateMaterial("LoadingBar", R"({ "shader":"LoadingBar", "systemCreated":true})"_json);
	CreatePrimitiveMeshTemplate("floor");
	CreatePrimitiveMeshTemplate("utahteapot");
	CreatePrimitiveMeshTemplate("cube");
	CreatePrimitiveMeshTemplate("pyramid");
	CreatePrimitiveMeshTemplate("decal");
	CreatePrimitiveMeshTemplate("boxlines");
}

void CreateTemplates() {

	using namespace Templates;

	Templates::LoadTemplates(defaultTemplatesFolder, Shader::templateName, Templates::CreateShader);
	Templates::LoadTemplates(defaultTemplatesFolder, Material::templateName, Templates::CreateMaterial);
	Templates::LoadTemplates(defaultTemplatesFolder, Model3D::templateName, Templates::CreateModel3D);
	Templates::LoadTemplates(defaultTemplatesFolder, Sound::templateName, Templates::CreateSound);
}

void CreateLightingResourcesMapping() {
	CreateLightsResources();
	CreateShadowMapResources();
}

//READ&GET

//UPDATE
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#if defined(_EDITOR)
	std::set<UINT> nonEditorMessages = { WM_QUIT };
	if (nonEditorMessages.contains(message)) DefWindowProc(hWnd, message, wParam, lParam);
	if (WndProcHandlerEditor(hWnd, message, wParam, lParam)) return true;
#endif

	switch (message)
	{
	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
	{
		Keyboard::ProcessMessage(message, wParam, lParam);
		Mouse::ProcessMessage(message, wParam, lParam);
	}
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
	{
		Mouse::ProcessMessage(message, wParam, lParam);
	}
	break;
	case WM_SYSKEYDOWN:
	{
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
			//implement fullscreen with ALT+ENTER
			/*
			if (inFullScreen) {
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

				ShowWindow(hWnd, SW_SHOWNORMAL);

				SetWindowPos(hWnd, HWND_TOP, hWndRect.left, hWndRect.top, hWndRect.right - hWndRect.left, hWndRect.bottom - hWndRect.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			else {
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

				ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			}
			*/
			/*
			//implement fullscreen with ALT+ENTER
			if (inFullScreen) {
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
				//SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);
				SetWindowLongPtr(hWnd, GWL_STYLE, 0);

				ShowWindow(hWnd, SW_SHOWNORMAL);
				//SetWindowLong(hWnd, GWL_STYLE, 0);
				SetWindowPos(hWnd, HWND_TOP, hWndRect.left, hWndRect.top, hWndRect.right - hWndRect.left, hWndRect.bottom - hWndRect.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			else {
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

				ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			}
			*/
#if !defined(_EDITOR)
			inFullScreen = !inFullScreen;
			ResetWindowStyle(inFullScreen);
#endif
		}
		Keyboard::ProcessMessage(message, wParam, lParam);
	}
	break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		Keyboard::ProcessMessage(message, wParam, lParam);
	}
	break;
	case WM_PAINT:
	{
		if (inSizeMove && renderer)
		{
			AppStep();
		}
		else
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(hWnd, &ps);
		}
	}
	break;
	case WM_DESTROY:
	{
		DestroyInstance();
		PostQuitMessage(0);
	}
	break;
	case WM_ENTERSIZEMOVE:
	{
		inSizeMove = true;
	}
	break;
	case WM_EXITSIZEMOVE:
	{
		inSizeMove = false;
		if (renderer) {
			GetWindowRect(hWnd, &hWndRect);
			resizeWindow = true;
			resizeWidth = static_cast<unsigned int>(hWndRect.right - hWndRect.left);
			resizeHeight = static_cast<unsigned int>(hWndRect.bottom - hWndRect.top);
		}
	}
	break;
	case WM_SIZE:
	{
		if (renderer)
		{
			resizeWindow = true;
			resizeWidth = LOWORD(lParam);
			resizeHeight = HIWORD(lParam);
		}
	}
	break;
	case WM_NCCALCSIZE:
	{
		if (wParam == TRUE)
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_SYSCOMMAND:
	{
		if (wParam == SC_MINIMIZE) {}
		if (wParam == SC_CLOSE) { appDone = true; return 0; }
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	break;
#if defined(_EDITOR)
	case WM_NCHITTEST:
	{
		POINTS pt = MAKEPOINTS(lParam);

		RECT& r = hWndRect;

		bool leftBorder = nostd::in_between(pt.x, r.left, r.left + 5);
		bool rightBorder = nostd::in_between(pt.x, r.right - 5, r.right);
		bool topBorder = nostd::in_between(pt.y, r.top, r.top + 5);
		bool bottomBorder = nostd::in_between(pt.y, r.bottom - 5, r.bottom);

		std::map<std::tuple<bool, bool, bool, bool>, LRESULT> htMap = {
			{ std::tuple(true,false,false,false), HTLEFT },
			{ std::tuple(false,true,false,false), HTRIGHT },
			{ std::tuple(false,false,true,false), HTTOP },
			{ std::tuple(false,false,false,true), HTBOTTOM },
			{ std::tuple(true,false,false,true), HTBOTTOMLEFT },
			{ std::tuple(false,true,false,true), HTBOTTOMRIGHT },
			{ std::tuple(true,false,false,true), HTTOPLEFT },
			{ std::tuple(false,true,false,true), HTTOPRIGHT },
		};

		std::tuple htTuple = std::tuple(leftBorder, rightBorder, topBorder, bottomBorder);
		return (htMap.contains(htTuple) ? htMap.at(htTuple) : HTCLIENT);
	}
	break;
#endif
	default:
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	break;
	}
	return 0;
}

void AppStep() {
	if (!inFullScreen)
	{
		GetWindowRect(hWnd, &hWndRect);
	}

	if (resizeWindow) {
		return ResizeWindow();
	}
	timer.Tick([&]() {});
	GameInputStep();
	EffectsStep(static_cast<FLOAT>(timer.GetElapsedSeconds()));
	AnimableStep(timer.GetElapsedSeconds());
	RenderablesStep();
	LightsStep();
	AudioStep();
	CamerasStep();
	GameStep();
	Render();

}

void GameInputStep()
{
	/*
#if defined(_EDITOR)
	return;
#endif
	if (GetNumCameras() == 0) return;

	//get the gamepad and keyboard states and update their trackers
	auto keyboardState = keyboard->GetState();
	keys.Update(keyboardState);
	auto mouseState = mouse->GetState();
	auto gamePadState = gamePad->GetState(0);
	buttons.Update(gamePadState);

	CameraPtr camera = GetCamera(currentCamera);

	if (!gamePadState.IsConnected()) {

		if (keys.pressed.PageDown) { currentCamera = (currentCamera + 1) % static_cast<UINT>(Scene::Camera::GetNumCameras()); }
		if (keys.pressed.PageUp) { currentCamera = (currentCamera != 0) ? (currentCamera - 1) : (static_cast<UINT>(Scene::Camera::GetNumCameras()) - 1); }

		//control the camera using the keyboard
		camera->ProcessKeyboardInput(keys, keyboardState);
		camera->ProcessMouseInput(mouseState, mouseCameraRotationSensitivity);

	}
	else
	{
		if (buttons.rightShoulder == GamePad::ButtonStateTracker::RELEASED) { currentCamera = (currentCamera + 1) % static_cast<UINT>(Scene::Camera::GetNumCameras()); }
		if (buttons.leftShoulder == GamePad::ButtonStateTracker::RELEASED) { currentCamera = (currentCamera != 0) ? (currentCamera - 1) : (static_cast<UINT>(Scene::Camera::GetNumCameras()) - 1); }

		camera->ProcessGamepadInput(gamePadState, gamePadCameraRotationSensitivity);

	}
	*/
}

void AnimableStep(double elapsedSeconds)
{
	for (auto& [name, r] : GetAnimables())
	{
		r->StepAnimation(elapsedSeconds);
	}
}

void AudioStep()
{
	GetAudioListenerVectors([](XMFLOAT3 pos, XMFLOAT3 fw, XMFLOAT3 up)
		{
			UpdateListener(pos, fw, up);
		}
	);
	UpdateAudio();
	SoundEffectsStep();
}

//RENDER

void Render()
{

	renderer->ResetCommands();
	renderer->SetCSUDescriptorHeap();

	RunRender();

	renderer->ExecuteCommands();
	renderer->Present();
}

void ResizeWindow()
{
	resizeWindow = false;
	renderer->Flush();

	WindowResizeReleaseResources();
	if (renderer)
	{
		renderer->Resize(resizeWidth, resizeHeight);
	}
	WindowResize(resizeWidth, resizeHeight);
}

//DESTROY
void DestroyInstance()
{
	if (destroyed) return;
	renderer->Flush();

#if defined(_EDITOR)
	DestroyEditor();
#endif

	GameDestroy();

#if defined(_EDITOR)
	DestroyTemplatesReferences();
#endif

	DestroySceneObjects();
	DestroyLightsResources();
	DestroyShadowMapResources();

	DestroyTemplates();

	DestroyConstantsBuffer();

	ShutdownAudio();
	gamePad.reset();
	keyboard.reset();

	DestroyRenderPipeline();

	DestroyShaderCompiler();

	renderer->Destroy();
	renderer = nullptr;

	destroyed = true;
}
