#include "pch.h"

#include <atlbase.h>
#include <JExposeEditor.h>
#include <Editor.h>
#include <Mouse.h>
#include <Application.h>
#include <JObject.h>
#include <Renderer.h>
#include <RenderPass/RenderPass.h>
#include <DeviceUtils/Resources/Resources.h>
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>
#include <Light/Light.h>
#include <Sound/SoundFX.h>
#include <Templates.h>
#include <Sound/Sound.h>
#include <Textures/Texture.h>
#include <Shader/Shader.h>
#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <IconsFontAwesome5.h>
#include <functional>
#include <RightPanelComponent.h>
#include <Level.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <MousePicking.h>

extern HWND hWnd;
extern RECT hWndRect;
extern std::unique_ptr<DirectX::Mouse> mouse;
extern std::unique_ptr<DirectX::Keyboard> keyboard;
extern std::shared_ptr<Renderer> renderer;
extern std::string gameAppTitle;
extern bool inSizeMove;
extern RECT GetMaximizedAreaSize();

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Editor {

	std::string currentLevelName = defaultLevelName;
	bool levelModified = false;
	bool defaultLevel = true;

	bool initialized = false;
	bool maximized = true;
	bool mouseClicked = false;
	bool clickedInDragArea = false;
	int lastMouseX;
	int lastMouseY;

	std::shared_ptr<Renderable> boundingBox = nullptr;

	float titleBH = static_cast<float>(ApplicationBarBottom);
	float panW = static_cast<float>(RightPanelWidth);
	ImGuiWindowFlags panFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	bool NonGameMode = false;
	bool lockedGameAreaInput = false;

	RightPanelComponent sceneObjectEdition("sceneObjects", { "hidden", "uuid" }, { "Scene Objects", "Details" }, { "Scene Objects" });
	RightPanelComponent templateEdition("templates", { "hidden", "uuid" }, { "Templates", "Details" }, { "Templates" });

	ImGui_ImplDX12_InitInfo init_info = {};

	enum MouseGameAreaMode
	{
		MOUSE_GAMEAREA_MODE_NONE,
		MOUSE_GAMEAREA_MODE_PICKING,
		MOUSE_GAMEAREA_MODE_GIZMO,
		MOUSE_GAMEAREA_MODE_CAMERA
	};
	MouseGameAreaMode currentMouseMode = MOUSE_GAMEAREA_MODE_NONE;
	MousePicking mousePicking;

	struct
	{
		bool leftButton = false;
		bool rightButton = false;
		bool wheelCaptured = false;
		bool wheelMode = false;
		int wheel = 0;
		int mouseX = 0;
		int mouseY = 0;
		void Reset()
		{
			leftButton = false;
			rightButton = false;
			wheelCaptured = false;
			wheelMode = false;
			wheel = 0;
			mouseX = 0;
			mouseY = 0;
		}
	} mouseCamera;

	ImGuizmo::OPERATION gizmoOperation(ImGuizmo::TRANSLATE);
	ImGuizmo::MODE gizmoMode(ImGuizmo::WORLD);

	CreatorModal<SceneObjectType> sceneObjectModal;
	CreatorModal<TemplateType> templateModal;

	//Editor LifeCycle
	void InitEditor()
	{
		initialized = true;
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->AddFontDefault();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		float baseFontSize = 13.0f;
		float iconFontSize = baseFontSize * 2.0f / 3.0f;

		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphMinAdvanceX = iconFontSize;
		std::filesystem::path fontPath("Fonts");
		fontPath /= FONT_ICON_FILE_NAME_FAS;
		io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), iconFontSize, &icons_config, icons_ranges);

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(hWnd);

		init_info.Device = renderer->d3dDevice;
		init_info.CommandQueue = renderer->commandQueue;
		init_info.NumFramesInFlight = renderer->numFrames;
		init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

		// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
		// (current version of the backend will only allocate one descriptor, future versions will need to allocate more)
		init_info.SrvDescriptorHeap = DeviceUtils::GetCSUDescriptorHeap();
		init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
			{
				::CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_xhandle;
				::CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_xhandle;
				DeviceUtils::AllocCSUDescriptor(cpu_xhandle, gpu_xhandle);
				out_cpu_handle->ptr = cpu_xhandle.ptr;
				out_gpu_handle->ptr = gpu_xhandle.ptr;
			};
		init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_xhandle(cpu_handle);
				CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_xhandle(gpu_handle);
				DeviceUtils::FreeCSUDescriptor(cpu_xhandle, gpu_xhandle);
				cpu_handle.ptr = 0;
				gpu_handle.ptr = 0;
			};
		ImGuiImplRenderInit();
		SetupImGuiStyle();
	}

	void ImGuiImplRenderInit()
	{
		if (initialized) ImGui_ImplDX12_Init(&init_info);
	}

	void SetupImGuiStyle()
	{
		// Green Font style by aiekick from ImThemes
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.6000000238418579f;
		style.WindowPadding = ImVec2(8.0f, 8.0f);
		style.WindowRounding = 0.0f;
		style.WindowBorderSize = 1.0f;
		style.WindowMinSize = ImVec2(32.0f, 32.0f);
		style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Left;
		style.ChildRounding = 0.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupRounding = 0.0f;
		style.PopupBorderSize = 1.0f;
		style.FramePadding = ImVec2(4.0f, 3.0f);
		style.FrameRounding = 0.0f;
		style.FrameBorderSize = 0.0f;
		style.ItemSpacing = ImVec2(8.0f, 4.0f);
		style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
		style.CellPadding = ImVec2(4.0f, 2.0f);
		style.IndentSpacing = 21.0f;
		style.ColumnsMinSpacing = 6.0f;
		style.ScrollbarSize = 14.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabMinSize = 10.0f;
		style.GrabRounding = 0.0f;
		style.TabRounding = 4.0f;
		style.TabBorderSize = 0.0f;
		style.TabMinWidthForCloseButton = 0.0f;
		style.ColorButtonPosition = ImGuiDir_Right;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05882352963089943f, 0.05882352963089943f, 0.05882352963089943f, 0.9399999976158142f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f, 0.9399999976158142f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.4392156898975372f, 0.4392156898975372f, 0.4392156898975372f, 0.6000000238418579f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.5686274766921997f, 0.5686274766921997f, 0.5686274766921997f, 0.699999988079071f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.7568627595901489f, 0.7568627595901489f, 0.7568627595901489f, 0.800000011920929f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.03921568766236305f, 0.03921568766236305f, 0.03921568766236305f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.6000000238418579f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.5490196347236633f, 0.800000011920929f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.7490196228027344f, 0.800000011920929f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 1.0f, 0.800000011920929f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.5490196347236633f, 0.4000000059604645f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.7490196228027344f, 0.6000000238418579f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 1.0f, 0.800000011920929f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.5490196347236633f, 0.4000000059604645f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.7490196228027344f, 0.6000000238418579f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 1.0f, 0.800000011920929f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.5490196347236633f, 0.4000000059604645f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.7490196228027344f, 0.6000000238418579f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 1.0f, 0.800000011920929f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.5490196347236633f, 0.4000000059604645f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.7490196228027344f, 0.6000000238418579f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 1.0f, 0.800000011920929f);
		style.Colors[ImGuiCol_Tab] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.5490196347236633f, 0.800000011920929f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 0.7490196228027344f, 0.800000011920929f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.1294117718935013f, 0.7490196228027344f, 1.0f, 0.800000011920929f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1764705926179886f, 0.1764705926179886f, 0.1764705926179886f, 1.0f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.3568627536296844f, 0.3568627536296844f, 0.3568627536296844f, 0.5400000214576721f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392217636108f, 0.6980392336845398f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
		style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
		style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
		style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.07000000029802322f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.3499999940395355f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
	}

	void DestroyEditor() {
		initialized = false;

		for (auto& uuid : sceneObjectEdition.editables)
		{
			SendEditorDestroyPreview(uuid, GetSceneObject);
		}
		for (auto& uuid : templateEdition.editables)
		{
			SendEditorDestroyPreview(uuid, GetTemplate);
		}

		ClearSceneObjectsSelection();

		sceneObjectEdition.Destroy();
		templateEdition.Destroy();

		mousePicking.pickedObjects.clear();
		DestroyPickingPass();
		DestroyRenderableBoundingBox();

		// Cleanup
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	bool WndProcHandlerEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	}

	//Editor Drawing
	void DrawEditor(std::shared_ptr<Camera> camera) {

		if (inSizeMove) return;

		unsigned int backBufferIndex = renderer->backBufferIndex;
		auto pass = renderer->swapChainPass->swapChainPass;
		auto backBuffer = pass->renderTargets[backBufferIndex];
		auto commandList = renderer->commandList;
		TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		Editor::NonGameMode = false;

		if (sceneObjectEdition.selectedNextFrame != "")
		{
			OpenTemplate(sceneObjectEdition.selectedNextFrame);
			sceneObjectEdition.selectedNextFrame = "";
		}

		if (templateEdition.selectedNextFrame != "")
		{
			OpenTemplate(templateEdition.selectedNextFrame);
			templateEdition.selectedNextFrame = "";
		}

		DrawApplicationBar();
		DrawRightPanel();
		if (camera)
			DrawPickedObjectsGizmo(camera);

		sceneObjectModal.DrawCreationPopup();
		templateModal.DrawCreationPopup();

		// Rendering
		ImGui::Render();

		// Render Dear ImGui graphics
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), renderer->commandList);

		TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}

	void DrawApplicationBar()
	{
		RECT dragRect;
		ZeroMemory(&dragRect, sizeof(dragRect));
		dragRect.bottom = ApplicationBarBottom;

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::MenuItem(ICON_FA_FILE "New");
				ImGui::Separator();
				if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN "Open")) {
					OpenLevelFile();
				}

				ImGui::DrawItemWithEnabledState([]
					{
						if (ImGui::MenuItem(ICON_FA_SAVE "Save")) {
							SaveLevelToFile(currentLevelName);
						}
					}, currentLevelName != ""
				);

				if (ImGui::MenuItem(ICON_FA_SAVE "Save As..")) {
					SaveLevelAs();
				}
				ImGui::Separator();
				if (ImGui::MenuItem(ICON_FA_SAVE "Save Templates")) {
					SaveTemplates();
				}
				ImGui::Separator();
				if (ImGui::MenuItem(ICON_FA_TIMES "Exit")) // It would be nice if this was a "X" like in the windows title bar set off to the far right
				{
					PostMessageA(hWnd, WM_QUIT, 0, 0);
				}
				ImGui::EndMenu();

			}

			auto cursorPos = ImGui::GetCursorScreenPos();
			dragRect.left = static_cast<LONG>(cursorPos.x);

			const ImGuiViewport* viewport = ImGui::GetMainViewport();

			std::string titleBar = gameAppTitle;
			if (currentLevelName != "")
			{
				titleBar += " - " + currentLevelName;
			}

			auto windowWidth = ImGui::GetWindowSize().x;
			auto textWidth = ImGui::CalcTextSize(titleBar.c_str()).x;

			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
			ImGui::Text(titleBar.c_str());

			struct WinButtonDef {
				std::string label;
				FLOAT x;
				FLOAT y = 0.0f;
				std::function<void()> onClick;
			};

			WinButtonDef windowsButtons[] = {
				{
					.label = ICON_FA_TIMES,
					.x = viewport->WorkSize.x - 1.0f * 19.0f,
					.onClick = []()
				{
					if (!Editor::levelModified || Editor::defaultLevel)
					{
						PostMessageA(hWnd,WM_QUIT,0,0);
					}
					else
					{
						int response = MessageBoxA(hWnd,"The level has been modified, do you wish to Save your work before leaving?","Save before leaving?",MB_ICONWARNING | MB_YESNOCANCEL);
						switch (response) {
						case IDYES:
						{
							Editor::SaveLevelToFile(Editor::currentLevelName);
						}
						case IDNO:
						{
							PostMessageA(hWnd, WM_QUIT, 0, 0);
						}
						break;
						}
					}
				}},
				{
					.label = ICON_FA_WINDOW_MAXIMIZE,
					.x = viewport->WorkSize.x - 2.0f * 19.0f,
					.onClick = []()
				{
					if (maximized)
					{
						SetWindowPos(hWnd, HWND_TOP, 0, 0, 640, 480, 0);
					}
					else
					{
						RECT desktopRect = GetMaximizedAreaSize();
						SetWindowPos(hWnd, HWND_TOP, 0, 0, desktopRect.right - desktopRect.left, desktopRect.bottom - desktopRect.top, 0);
					}
					maximized = !maximized;
				}},
				{
					.label = ICON_FA_WINDOW_MINIMIZE,
					.x = viewport->WorkSize.x - 3.0f * 19.0f,
					.onClick = []()
				{
					//ShowWindow(hWnd, SW_MINIMIZE);
					SendMessageA(hWnd,WM_SYSCOMMAND, SC_MINIMIZE,0);
				}},
			};

			dragRect.right = static_cast<LONG>(windowsButtons[_countof(windowsButtons) - 1].x);

			for (auto button : windowsButtons) {
				ImGui::SetCursorPos(ImVec2(button.x, button.y));
				if (ImGui::Button(button.label.c_str(), ImVec2(19.0f, 19.0f))) { button.onClick(); }
			}

			ImGui::EndMainMenuBar();
		}
		HandleApplicationDragTitleBar(dragRect);
	}

	void HandleApplicationDragTitleBar(RECT& dragRect)
	{
		auto mouseState = mouse->GetState();

		auto inDragBounds = [&dragRect, &mouseState]() {
			return (
				mouseState.y < dragRect.bottom &&
				mouseState.y > dragRect.top &&
				mouseState.x < dragRect.right &&
				mouseState.x > dragRect.left
				);
			};

		if (mouseState.leftButton && !mouseClicked)
		{
			mouseClicked = true;
			if (inDragBounds())
			{
				clickedInDragArea = true;
				lastMouseX = mouseState.x;
				lastMouseY = mouseState.y;
			}
		}
		else if (mouseState.leftButton && clickedInDragArea)
		{
			INT diffMouseX = mouseState.x - lastMouseX;
			INT diffMouseY = mouseState.y - lastMouseY;

			RECT currentRect;
			GetWindowRect(hWnd, &currentRect);

			INT newX = currentRect.left + diffMouseX;
			INT newY = currentRect.top + diffMouseY;

			SetWindowPos(hWnd, nullptr, newX, newY, 0, 0, SWP_NOSIZE);

			lastMouseX = mouseState.x - diffMouseX;
			lastMouseY = mouseState.y - diffMouseY;
		}
		else if (!mouseState.leftButton)
		{
			mouseClicked = false;
			clickedInDragArea = false;
		}
	}

	void OpenLevelFile()
	{
		using namespace Scene::Level;

		ImGui::OpenFile([](std::filesystem::path path)
			{
				std::filesystem::path jsonFilePath = path;
				jsonFilePath.replace_extension(".json");
				SetLevelToLoad(jsonFilePath.generic_string());
			},
			defaultLevelsFolder);
	}

	void SaveLevelAs()
	{
		std::thread saveAs([]()
			{
				//first create the directory if needed
				std::filesystem::path directory(nostd::StringToWString(defaultLevelsFolder));
				std::filesystem::create_directory(directory);

				std::wstring path = L"";
				COMDLG_FILTERSPEC filters[] = { {.pszName = L"JSON files. (*.json)", .pszSpec = L"*.json" } };
				std::pair<COMDLG_FILTERSPEC*, int> filter_info = std::make_pair<COMDLG_FILTERSPEC*, int>(filters, _countof(filters));
				if (!SaveFileDialog(path, std::filesystem::absolute(directory), L"", &filter_info)) return;
				if (path.empty()) return;

				std::filesystem::path jsonFilePath = path;
				jsonFilePath.replace_extension(".json");

				SaveLevelToFile(nostd::WStringToString(jsonFilePath.filename()));
			}
		);
		saveAs.detach();
	}

	bool SaveFileDialog(std::wstring& path, std::wstring defaultDirectory, std::wstring defaultFileName, std::pair<COMDLG_FILTERSPEC*, int>* pFilterInfo)
	{
		IFileSaveDialog* p_file_save = nullptr;
		bool are_all_operation_success = false;
		while (!are_all_operation_success)
		{
			HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
				IID_IFileSaveDialog, reinterpret_cast<void**>(&p_file_save));
			if (FAILED(hr))
				break;

			if (!pFilterInfo)
			{
				COMDLG_FILTERSPEC save_filter[1];
				save_filter[0].pszName = L"All files";
				save_filter[0].pszSpec = L"*.*";
				hr = p_file_save->SetFileTypes(1, save_filter);
				if (FAILED(hr))
					break;
				hr = p_file_save->SetFileTypeIndex(1);
				if (FAILED(hr))
					break;
			}
			else
			{
				hr = p_file_save->SetFileTypes(pFilterInfo->second, pFilterInfo->first);
				if (FAILED(hr))
					break;
				hr = p_file_save->SetFileTypeIndex(1);
				if (FAILED(hr))
					break;
			}

			if (!defaultDirectory.empty()) {
				IShellItem* pCurFolder = NULL;
				hr = SHCreateItemFromParsingName(defaultDirectory.c_str(), NULL, IID_PPV_ARGS(&pCurFolder));
				if (FAILED(hr))
					break;
				p_file_save->SetFolder(pCurFolder);
				pCurFolder->Release();
			}

			if (!defaultFileName.empty())
			{
				hr = p_file_save->SetFileName(defaultFileName.c_str());
				if (FAILED(hr))
					break;
			}

			hr = p_file_save->Show(NULL);
			if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) // No item was selected.
			{
				are_all_operation_success = true;
				break;
			}
			else if (FAILED(hr))
				break;

			IShellItem* p_item;
			hr = p_file_save->GetResult(&p_item);
			if (FAILED(hr))
				break;

			PWSTR item_path;
			hr = p_item->GetDisplayName(SIGDN_FILESYSPATH, &item_path);
			if (FAILED(hr))
				break;
			path = item_path;
			CoTaskMemFree(item_path);
			p_item->Release();

			are_all_operation_success = true;
		}

		if (p_file_save)
			p_file_save->Release();
		return are_all_operation_success;
	}

	void SaveLevelToFile(std::string levelFileName)
	{
		using namespace nlohmann;

		nlohmann::json level;

		level["renderables"] = json::array();
		level["lights"] = json::array();
		level["cameras"] = json::array();
		level["sounds"] = json::array();

		WriteRenderablesJson(level["renderables"]);
		WriteLightsJson(level["lights"]);
		WriteCamerasJson(level["cameras"]);
		WriteSoundEffectsJson(level["sounds"]);

		std::string levelString = level.dump(4);

		const std::string levelsRootFolder = "Levels/";
		const std::string filename = levelsRootFolder + levelFileName;

		//first create the directory if needed
		std::filesystem::path directory(levelsRootFolder);
		std::filesystem::create_directory(directory);

		//then create the json level file
		std::filesystem::path path(filename);
		path.replace_extension(".json");
		std::string pathStr = path.generic_string();
		std::ofstream file;
		file.open(pathStr);
		file.write(levelString.c_str(), levelString.size());
		file.close();

		currentLevelName = levelFileName;
		defaultLevel = false;
		levelModified = false;
	}

	void SaveTemplates()
	{
		using namespace Templates;
		Templates::SaveTemplates(defaultTemplatesFolder, Shader::templateName, WriteShadersJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Material::templateName, WriteMaterialsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Model3D::templateName, WriteModel3DsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Sound::templateName, WriteSoundsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Texture::templateName, WriteTexturesJson);
	}

	float separatorFactor = 0.0f;
	const float panelMinHeight = 47.0f;
	void DrawRightPanel() {

		auto getSceneObjects = []()
			{
				auto sceneObjects = GetSceneObjects();
				std::vector<UUIDName> sceneObjectsList;
				for (auto& [type, uuidnames] : sceneObjects)
				{
					for (auto& uuidname : uuidnames)
					{
						std::string& uuid = std::get<0>(uuidname);
						std::string& name = std::get<1>(uuidname);

						std::string nname = SceneObjectTypeToString.at(type) + "/" + name;
						sceneObjectsList.push_back(std::tie(uuid, nname));
					}
				}
				return sceneObjectsList;
			};
		auto getTemplates = []()
			{
				auto templates = GetTemplates();
				std::vector<UUIDName> templatesList;
				for (auto& [type, uuidnames] : templates)
				{
					for (auto& uuidname : uuidnames)
					{
						std::string& uuid = std::get<0>(uuidname);
						std::string& name = std::get<1>(uuidname);

						std::string nname = TemplateTypeToString.at(type) + "/" + name;
						templatesList.push_back(std::tie(uuid, nname));
					}
				}
				return templatesList;
			};
		auto matchSceneObjectsAttributes = []()
			{
				sceneObjectEdition.CreateEditableAttributesToMatch<SceneObjectType>(
					GetSceneObjectType,
					GetSceneObject,
					GetSceneObjectAttributes,
					GetSceneObjectDrawers,
					GetSceneObjectPreviewers
				);
			};
		auto matchTemplatesAttributes = []()
			{
				templateEdition.CreateEditableAttributesToMatch<TemplateType>(
					GetTemplateType,
					GetTemplate,
					GetTemplateAttributes,
					GetTemplateDrawers,
					GetTemplatePreviewers
				);
			};

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 panPos = ImVec2(viewport->WorkSize.x - panW, viewport->WorkPos.y);
		ImVec2 panSize = ImVec2(panW, viewport->WorkSize.y);

		ImGui::SetNextWindowPos(panPos);
		ImGui::SetNextWindowSize(panSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Right panel", (bool*)1, panFlags);
		{
			float halfY = panSize.y * 0.5f;

			float soPanelH = max(halfY * (1.0f - separatorFactor) - 2.5f, panelMinHeight);
			ImVec2 soPos = ImVec2(panPos.x, panPos.y);
			ImVec2 soSize = ImVec2(panSize.x, soPanelH);

			sceneObjectEdition.DrawPanel(soPos, soSize, SceneObjectsTypePanelMenuItems,
				getSceneObjects, GetSceneObject,
				OnChangeSceneObjectTab,
				matchSceneObjectsAttributes,
				[](std::string uuid, bool selected) { SetSceneObjectSelection(uuid, selected); },
				[](std::string uuid) { SendEditorPreview(uuid, GetSceneObject, sceneObjectEdition.drawers); },
				[](SceneObjectType type) { StartSceneObjectCreation(type); },
				[](std::string uuid) { DeleteSceneObject(uuid); }
			);

			ImGui::Button("DragableSeparator", ImVec2(-1, 5));
			if (ImGui::IsItemActive())
			{
				float deltaY = ImGui::GetMouseDragDelta().y;
				separatorFactor -= deltaY / halfY;
				float hi = 1.0f - panelMinHeight / halfY;
				float low = -1.0f + panelMinHeight / halfY;
				if (separatorFactor < hi && separatorFactor > low)
					ImGui::ResetMouseDragDelta(); // Reset delta for continuous dragging
				separatorFactor = std::clamp(separatorFactor, low, hi);
			}

			float tePanelH = max(panSize.y - soPanelH - 5.0f, panelMinHeight);
			ImVec2 tePos = ImVec2(panPos.x, soSize.y + 5.0f);
			ImVec2 teSize = ImVec2(panSize.x, tePanelH);

			templateEdition.DrawPanel(tePos, teSize, TemplateTypePanelMenuItems,
				getTemplates, GetTemplate,
				OnChangeTemplateTab,
				matchTemplatesAttributes,
				[](std::string uuid, bool selected) {},
				[](std::string uuid) { SendEditorPreview(uuid, GetTemplate, templateEdition.drawers); },
				[](TemplateType type) { StartTemplateCreation(type); },
				[](std::string uuid) { DeleteTemplate(uuid); }
			);
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	//SceneObjects Panel
	void OnChangeSceneObjectTab(std::string newTab)
	{
		sceneObjectEdition.selectedTab = newTab;
		sceneObjectEdition.editables = sceneObjectEdition.selected;
		if (newTab == sceneObjectEdition.detailAbleTabs.at(1))
		{
			sceneObjectEdition.CreateEditableAttributesToMatch<SceneObjectType>(
				GetSceneObjectType,
				GetSceneObject,
				GetSceneObjectAttributes,
				GetSceneObjectDrawers,
				GetSceneObjectPreviewers
			);
			for (auto& uuid : sceneObjectEdition.editables)
			{
				SendEditorPreview(uuid, GetSceneObject, sceneObjectEdition.drawers);
			}
		}
		else
		{
			for (auto& uuid : sceneObjectEdition.editables)
			{
				SendEditorDestroyPreview(uuid, GetSceneObject);
			}
		}
	}

	void OpenSceneObject(std::string uuid)
	{
		sceneObjectEdition.selected = { uuid };
		OnChangeSceneObjectTab(templateEdition.detailAbleTabs.at(1));
	}

	void OpenSceneObjectOnNextFrame(std::string uuid)
	{
		sceneObjectEdition.selectedNextFrame = uuid;
	}

	void MarkScenePanelAssetsAsDirty()
	{
		sceneObjectEdition.dirtyAssetsTree = true;
	}

	void DestroyEditorSceneObjectsReferences()
	{
		sceneObjectEdition.Destroy();
	}

	//Templates Panel
	void OnChangeTemplateTab(std::string newTab)
	{
		templateEdition.selectedTab = newTab;
		templateEdition.editables = templateEdition.selected;
		if (newTab == templateEdition.detailAbleTabs.at(1))
		{
			templateEdition.CreateEditableAttributesToMatch<TemplateType>(
				GetTemplateType,
				GetTemplate,
				GetTemplateAttributes,
				GetTemplateDrawers,
				GetTemplatePreviewers
			);
			for (auto& uuid : templateEdition.editables)
			{
				SendEditorPreview(uuid, GetTemplate, templateEdition.drawers);
			}
		}
		else
		{
			for (auto& uuid : templateEdition.editables)
			{
				SendEditorDestroyPreview(uuid, GetTemplate);
			}
		}
	}

	void OpenTemplate(std::string uuid)
	{
		templateEdition.selected = { uuid };
		OnChangeTemplateTab(templateEdition.detailAbleTabs.at(1));
	}

	void OpenTemplateOnNextFrame(std::string uuid)
	{
		templateEdition.selectedNextFrame = uuid;
	}

	void MarkTemplatesPanelAssetsAsDirty()
	{
		templateEdition.dirtyAssetsTree = true;
	}

	//JObject's Preview Panel
	void SendEditorPreview(std::string uuid, auto GetJObject, auto drawers)
	{
		size_t flags = 0;
		std::shared_ptr<JObject> j = GetJObject(uuid);
		for (auto& [attribute, _] : drawers)
		{
			if (!j->UpdateFlagsMap.contains(attribute)) continue;
			auto& tp = j->UpdateFlagsMap.at(attribute);
			if (!std::get<1>(tp)) continue;
			flags |= std::get<0>(tp);
		}
		j->EditorPreview(flags);
	}

	void SendEditorDestroyPreview(std::string uuid, auto GetJObject)
	{
		std::shared_ptr<JObject> j = GetJObject(uuid);
		j->DestroyEditorPreview();
	}

	//Gizmos
	void DrawPickedObjectsGizmo(std::shared_ptr<Camera> camera)
	{
		if (mousePicking.pickedObjects.size() == 0ULL) return;

		switch (mousePicking.pickedType)
		{
		case SO_Renderables:
		{
			DrawRenderableGizmo(camera);
		}
		break;
		case SO_Lights:
		{
			DrawPickedLightGizmo(camera);
		}
		break;
		case SO_Cameras:
		{
			DrawCameraGizmo(camera);
		}
		break;
		case SO_SoundEffects:
		{
			DrawSoundEffectGizmo(camera);
		}
		break;
		}
	}

	void BeginGizmoInteraction(std::shared_ptr<Camera> camera, std::function<void(XMFLOAT4X4 view, XMFLOAT4X4 proj)> interaction)
	{
		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::AllowAxisFlip(false);

		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		ImGuizmo::SetID(0);

		XMFLOAT4X4 view;
		XMFLOAT4X4 proj;
		XMStoreFloat4x4(&view, camera->view());
		XMStoreFloat4x4(&proj, camera->perspectiveProjection.projectionMatrix);

		interaction(view, proj);
	}

	void DrawRenderableGizmo(std::shared_ptr<Camera> camera)
	{
		/*
		auto translateObjects = [](XMVECTOR translation)
			{
				XMVECTOR len = XMVector3Length(translation);
				if (len.m128_f32[0] < g_XMEpsilon.f[0]) return;
				for (auto& o : mousePicking.pickedObjects)
				{
					if (!o->contains("position")) continue;

					XMFLOAT3 pos = ToXMFLOAT3(o->at("position"));
					pos.x += translation.m128_f32[0];
					pos.y += translation.m128_f32[1];
					pos.z += translation.m128_f32[2];
					nlohmann::json patch = { {"position",FromXMFLOAT3(pos)} };
					o->JUpdate(patch);
				}
			};
		*/
		/*
		auto rotateObjects = [](XMFLOAT4X4 transformation)
			{
				XMFLOAT3 p0 = ToXMFLOAT3((*mousePicking.pickedObjects.begin())->at("position"));
				XMFLOAT4 vp0 = { p0.x,p0.y,p0.z,0.0f };
				XMVECTOR pf032 = XMLoadFloat4(&vp0);

				XMFLOAT3 rotDelta = DX::GetPitchYawRoll(transformation);

				for (auto& o : mousePicking.pickedObjects)
				{
					if (!o->contains("rotation")) continue;

					XMFLOAT3 rot = ToXMFLOAT3(o->at("rotation"));
					rot.x += rotDelta.x;
					rot.y += rotDelta.y;
					rot.z += rotDelta.z;

					if (o != *mousePicking.pickedObjects.begin())
					{
						XMFLOAT3 p = ToXMFLOAT3(o->at("position"));
						XMFLOAT4 vp = { p.x,p.y,p.z,0.0f };
						XMVECTOR pf32 = XMLoadFloat4(&vp);
						XMVECTOR diff = XMVectorSubtract(pf32, pf032);
						XMVECTOR newPos = XMVector3Transform(diff, XMLoadFloat4x4(&transformation));
						newPos = XMVectorAdd(newPos, pf032);

						nlohmann::json patch = {
							{ "position", FromXMFLOAT3(*(XMFLOAT3*)newPos.m128_f32) },
							{ "rotation", FromXMFLOAT3(rot) }
						};
						o->JUpdate(patch);
					}
					else
					{
						nlohmann::json patch = { { "rotation",FromXMFLOAT3(rot) } };
						o->JUpdate(patch);
					}
				}
			};
		*/
		/*
		auto scaleObjects = [](XMVECTOR XMscale, XMFLOAT4X4 transformation)
			{
				XMFLOAT3 p0 = ToXMFLOAT3((*mousePicking.pickedObjects.begin())->at("position"));
				XMFLOAT4 vp0 = { p0.x,p0.y,p0.z,0.0f };
				XMVECTOR pf032 = XMLoadFloat4(&vp0);

				for (auto& o : mousePicking.pickedObjects)
				{
					if (!o->contains("scale")) continue;

					XMFLOAT3 scl = ToXMFLOAT3(o->at("scale"));
					scl.x *= XMscale.m128_f32[0];
					scl.y *= XMscale.m128_f32[1];
					scl.z *= XMscale.m128_f32[2];

					if (o != *mousePicking.pickedObjects.begin())
					{
						XMFLOAT3 p = ToXMFLOAT3(o->at("position"));
						XMFLOAT4 vp = { p.x,p.y,p.z,0.0f };
						XMVECTOR pf32 = XMLoadFloat4(&vp);
						XMVECTOR diff = XMVectorSubtract(pf32, pf032);
						XMVECTOR newPos = XMVector3Transform(diff, XMLoadFloat4x4(&transformation));
						newPos = XMVectorAdd(newPos, pf032);

						nlohmann::json patch = {
							{ "position", FromXMFLOAT3(*(XMFLOAT3*)newPos.m128_f32) },
							{ "scale", FromXMFLOAT3(scl) }
						};
						o->JUpdate(patch);
					}
					else
					{
						nlohmann::json patch = { { "scale",FromXMFLOAT3(scl) } };
						o->JUpdate(patch);
					}
				}
			};
		*/

		/*
		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::AllowAxisFlip(false);

		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		ImGuizmo::SetID(0);

		XMFLOAT4X4 w;
		XMFLOAT4X4 view;
		XMFLOAT4X4 proj;
		XMStoreFloat4x4(&w, (*mousePicking.pickedObjects.begin())->world());
		XMStoreFloat4x4(&view, camera->view());
		XMStoreFloat4x4(&proj, camera->perspectiveProjection.projectionMatrix);

		if (ImGui::IsKeyPressed(ImGuiKey_T)) // t ky
		{
			gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			gizmoMode = ImGuizmo::MODE::WORLD;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_R)) // r key
		{
			gizmoOperation = ImGuizmo::OPERATION::ROTATE;
			gizmoMode = ImGuizmo::MODE::WORLD;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_S)) // s Key
		{
			gizmoOperation = ImGuizmo::OPERATION::SCALE;
			gizmoMode = ImGuizmo::MODE::LOCAL;
		}

		XMFLOAT4X4 delta;
		ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *w.m, *delta.m, NULL, NULL, NULL);

		XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
		XMVECTOR XMtranslation;
		XMVECTOR XMrotation;
		XMVECTOR XMscale;
		XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

		if (gizmoOperation == ImGuizmo::OPERATION::TRANSLATE)
		{
			translateObjects(XMtranslation);
		}
		else if (gizmoOperation == ImGuizmo::OPERATION::ROTATE)
		{
			rotateObjects(delta);
		}
		else if (gizmoOperation == ImGuizmo::OPERATION::SCALE)
		{
			scaleObjects(XMscale, delta);
			//XMFLOAT3 newScale = boundingBox->scale();
			//newScale.x *= XMscale.m128_f32[0];
			//newScale.y *= XMscale.m128_f32[1];
			//newScale.z *= XMscale.m128_f32[2];
			//scale(newScale);
		}
		*/
	}

	void DrawPickedLightGizmo(std::shared_ptr<Camera> camera)
	{
		std::shared_ptr<Light> light = std::dynamic_pointer_cast<Light>((*mousePicking.pickedObjects.begin()));
		switch (light->lightType())
		{
		case LT_Directional:
		{
			BeginGizmoInteraction(camera, [&camera, light](XMFLOAT4X4 view, XMFLOAT4X4 proj)
				{
					XMMATRIX world = camera->world();
					XMVECTOR forward = XMVectorScale(camera->forward(), 10.0f);
					XMMATRIX forwardM = XMMatrixTranslationFromVector(forward);
					world = XMMatrixMultiply(world, forwardM);
					XMFLOAT4X4 w;
					XMStoreFloat4x4(&w, world);

					XMFLOAT4X4 delta;
					ImGuizmo::Manipulate(*view.m, *proj.m, ImGuizmo::OPERATION::ROTATE, ImGuizmo::MODE::WORLD, *w.m, *delta.m, NULL, NULL, NULL);

					XMFLOAT3 delRotation = DX::GetPitchYawRoll(delta);
					XMFLOAT3 curRotation = light->rotation();
					curRotation.x += delRotation.x;
					curRotation.y += delRotation.y;
					curRotation.z += delRotation.z;

					nlohmann::json patch = { { "rotation", FromXMFLOAT3(curRotation) } };
					light->JUpdate(patch);
				}
			);
		}
		break;
		}
	}

	void DrawCameraGizmo(std::shared_ptr<Camera> camera)
	{
	}

	void DrawSoundEffectGizmo(std::shared_ptr<Camera> camera)
	{
	}

	//SceneObject Selection
	std::map<std::string, std::shared_ptr<SceneObject>> selectedSceneObjectsMap;
	std::set<std::shared_ptr<SceneObject>> selectedSceneObjects;
	void SelectSceneObject(std::string uuid)
	{
		if (uuid == "" || (boundingBox && boundingBox->uuid() == uuid))
		{
			return;
		}

		std::shared_ptr<Renderable> r = FindInRenderables(uuid);
		r->OnPick();

		/*
		std::map<std::tuple<bool, bool>, std::function<void()>> actions =
		{
			{ std::make_tuple(false,false), [] //no shift, selecting nothing
				{
					mousePicking.pickedObjects.clear();
					sceneObjectEdition.selected.clear();
				}
			},
			{ std::make_tuple(false,true), [uuid] //no shift, selecting something
				{
					mousePicking.pickedObjects.clear();
					sceneObjectEdition.selected.clear();
					mousePicking.pickedObjects.insert(GetSceneObject(uuid));
					sceneObjectEdition.selected.insert(uuid);
					gizmoOperation = ImGuizmo::TRANSLATE;
					gizmoMode = ImGuizmo::WORLD;
				}
			},
			{ std::make_tuple(true,false), [] //shift, selecthing nothing. this wil undo last selection
				{
					if (mousePicking.pickedObjects.size() > 1ULL)
					{
						std::shared_ptr<SceneObject> last = mousePicking.pickedObjects.back();
						std::string uuid = last->at("uuid");
						mousePicking.pickedObjects.erase_back();
						sceneObjectEdition.selected.erase(uuid);
					}
					else
					{
						mousePicking.pickedObjects.clear();
						sceneObjectEdition.selected.clear();
					}
				}
			},
			{ std::make_tuple(true,true), [uuid]
				{
					gizmoOperation = ImGuizmo::TRANSLATE;
					gizmoMode = ImGuizmo::WORLD;
					mousePicking.pickedObjects.insert(GetSceneObject(uuid));
					sceneObjectEdition.selected.insert(uuid);
				}
			},
		};

		actions.at(std::make_tuple(keyboard->GetState().LeftShift, uuid != ""))();
		*/
	}

	void SelectRenderable(std::shared_ptr<Renderable> renderable)
	{
		ToggleSceneObjectFromSelection(renderable);
	}

	void SelectLight(std::shared_ptr<Light> light)
	{
		ToggleSceneObjectFromSelection(light);
	}

	void SelectCamera(std::shared_ptr<Camera> camera)
	{
		ToggleSceneObjectFromSelection(camera);
	}

	void SelectSoundEffect(std::shared_ptr<SoundFX> soundEffect)
	{
		ToggleSceneObjectFromSelection(soundEffect);
	}

	void ToggleSceneObjectFromSelection(std::shared_ptr<SceneObject> sceneObject)
	{
		std::string uuid = sceneObject->at("uuid");
		if (!sceneObjectEdition.selected.contains(uuid))
		{
			InsertSceneObjectToSelection(sceneObject);
		}
		else
		{
			EraseSceneObjectFromSelection(sceneObject);
		}
	}

	void SetSceneObjectSelection(std::string uuid, bool selected)
	{
		std::shared_ptr<SceneObject> so = GetSceneObject(uuid);
		if (selected)
		{
			selectedSceneObjectsMap.insert_or_assign(uuid, so);
			selectedSceneObjects.insert(so);
		}
		else
		{
			selectedSceneObjectsMap.erase(uuid);
			selectedSceneObjects.erase(so);
		}
	}

	void InsertSceneObjectToSelection(std::shared_ptr<SceneObject> sceneObject)
	{
		std::string uuid = sceneObject->at("uuid");
		sceneObjectEdition.selected.insert(uuid);
		SetSceneObjectSelection(uuid, true);
	}

	void EraseSceneObjectFromSelection(std::shared_ptr<SceneObject> sceneObject)
	{
		std::string uuid = sceneObject->at("uuid");
		sceneObjectEdition.selected.erase(uuid);
		SetSceneObjectSelection(uuid, false);
	}

	void ClearSceneObjectsSelection()
	{
		selectedSceneObjectsMap.clear();
		selectedSceneObjects.clear();
	}

	//BoundingBox
	bool RenderableBoundingBoxExists()
	{
		return boundingBox != nullptr;
	}

	void CreateRenderableBoundingBox(std::shared_ptr<Camera> camera)
	{
		nlohmann::json jbox = nlohmann::json(
			{
				{ "meshMaterials",
					{
						{
							{ "material", "2e4d8bf0-0761-45d9-8313-17cdf9b5f8fc"},
							{ "mesh", "30f15e68-db42-46fa-b846-b2647a0ac9b9" }
						}
					}
				},
				{ "castShadows", false },
				{ "shadowed", false },
				{ "name" , "EditorBoundingBox" },
				{ "uuid" , "797b3a2b-6854-47ab-8e07-1974055e490d" },
				{ "position" , { 0.0f, 0.0f, 0.0f} },
				{ "topology", "LINELIST"},
				{ "rotation" , { 0.0, 0.0, 0.0 } },
				{ "scale" , { 1.0f, 1.0f, 1.0f } },
				{ "skipMeshes" , {}},
				{ "visible" , false},
				{ "hidden" , true},
				{ "cameras", { camera->uuid() }}
			}
		);
		boundingBox = CreateSceneObjectFromJson<Renderable>(jbox);
		boundingBox->BindToScene();
	}

	void DestroyRenderableBoundingBox()
	{
		SafeDeleteSceneObject(boundingBox);
	}

	void UpdateBoundingBox()
	{
		std::set<std::shared_ptr<SceneObject>>& objects = selectedSceneObjects;
		if (objects.size() == 0ULL)
		{
			boundingBox->visible(false);
			return;
		}

		//OutputDebugStringA("UpdateBoundingBox\n");
		BoundingBox bb;
		bool bbfirst = true;
		for (auto& so : objects)
		{
			//OutputDebugStringA(std::string(std::string(so->at("name")) + "\n").c_str());
			if (bbfirst)
			{
				bb = so->GetBoundingBox();
				bbfirst = false;
			}
			else
			{
				BoundingBox sobb = so->GetBoundingBox();
				BoundingBox mbb;
				bb.CreateMerged(mbb, sobb, bb);
				bb = mbb;
			}
		}
		boundingBox->visible(true);
		boundingBox->scale(bb.Extents);
		boundingBox->position(bb.Center);
		boundingBox->WriteConstantsBuffer(renderer->backBufferIndex);
	}

	//Mouse Processing
	bool MouseIsInGameArea(std::unique_ptr<DirectX::Mouse>& mouse)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		RECT gameArea;
		ZeroMemory(&gameArea, sizeof(gameArea));
		gameArea.top = ApplicationBarBottom + 1L;
		gameArea.bottom = static_cast<LONG>(viewport->Size.y);
		gameArea.right = static_cast<LONG>(viewport->Size.x - panW);
		int x = mouse->GetState().x;
		int y = mouse->GetState().y;
		return (x > gameArea.left && x<gameArea.right && y > gameArea.top && y < gameArea.bottom);
	}

	void GameAreaMouseProcessing(std::unique_ptr<DirectX::Mouse>& mouse, std::shared_ptr<Camera> camera)
	{
		if (sceneObjectModal.creating || templateModal.creating) return;

		DirectX::Mouse::State state = mouse->GetState();

		if (!MouseIsInGameArea(mouse) && state.leftButton)
		{
			lockedGameAreaInput = true;
		}
		else if (lockedGameAreaInput && !state.leftButton)
		{
			lockedGameAreaInput = false;
		}

		if (NonGameMode || lockedGameAreaInput)
			return;

		auto resetMouseProcessing = []()
			{
				currentMouseMode = MOUSE_GAMEAREA_MODE_NONE;
				mousePicking.Reset();
				mouseCamera.Reset();
			};

		if (!MouseIsInGameArea(mouse))
		{
			resetMouseProcessing();
			return;
		}

		if (ImGuizmo::IsOver())
		{
			currentMouseMode = MOUSE_GAMEAREA_MODE_GIZMO;
		}

		switch (currentMouseMode)
		{
		case MOUSE_GAMEAREA_MODE_NONE:
		{
			if (state.leftButton)
			{
				currentMouseMode = MOUSE_GAMEAREA_MODE_PICKING;
				mousePicking.StartPicking(state);
			}
			if (state.rightButton)
			{
				currentMouseMode = MOUSE_GAMEAREA_MODE_CAMERA;
				mouseCamera.rightButton = true;
				mouseCamera.mouseX = state.x;
				mouseCamera.mouseY = state.y;
			}
			if (!mouseCamera.wheelCaptured)
			{
				mouseCamera.wheel = state.scrollWheelValue;
				mouseCamera.wheelCaptured = true;
			}
			else
			{
				if (mouseCamera.wheel != state.scrollWheelValue)
				{
					mouseCamera.mouseX = state.x;
					mouseCamera.mouseY = state.y;
					mouseCamera.wheel = state.scrollWheelValue;
					mouseCamera.wheelMode = true;
					currentMouseMode = MOUSE_GAMEAREA_MODE_CAMERA;
				}
			}

		}
		break;
		case MOUSE_GAMEAREA_MODE_PICKING:
		{
			if (mousePicking.CanPick(state))
			{
				mousePicking.Pick();
			}
			else if (mousePicking.MouseMoved(state))
			{
				currentMouseMode = MOUSE_GAMEAREA_MODE_CAMERA;
				mouseCamera.leftButton = true;
				mouseCamera.mouseX = state.x;
				mouseCamera.mouseY = state.y;
			}
			else if (!state.leftButton)
			{
				resetMouseProcessing();
			}
		}
		break;
		case MOUSE_GAMEAREA_MODE_GIZMO:
		{
			if (!ImGuizmo::IsOver())
			{
				resetMouseProcessing();
			}
		}
		break;
		case MOUSE_GAMEAREA_MODE_CAMERA:
		{
			if (mouseCamera.wheelMode)
			{
				int wheelDelta = state.scrollWheelValue - mouseCamera.wheel;
				mouseCamera.wheel = state.scrollWheelValue;
				if (wheelDelta != 0)
				{
					//do something like settings.at("camera").at("speed").at("fw");
					float fwMovement = wheelDelta > 0 ? 1.0f : -1.0f;
					camera->MoveAlongFwAxis(fwMovement);
				}
				if (state.leftButton || state.rightButton)
				{
					currentMouseMode = MOUSE_GAMEAREA_MODE_NONE;
					resetMouseProcessing();
				}
			}
			else
			{
				int mousedx = state.x - mouseCamera.mouseX;
				int mousedy = state.y - mouseCamera.mouseY;
				mouseCamera.mouseX = state.x;
				mouseCamera.mouseY = state.y;

				if (mouseCamera.leftButton)
				{
					if (state.leftButton)
					{
						float dx = static_cast<float>(mousedx) * 0.3f;
						float dy = static_cast<float>(mousedy) * 0.3f;
						camera->Rotate(dx, dy);
					}
					else
					{
						resetMouseProcessing();
					}
				}
				if (mouseCamera.rightButton)
				{
					if (state.rightButton)
					{
						float dx = -static_cast<float>(mousedx) * 0.01f;
						float dy = -static_cast<float>(mousedy) * 0.01f;
						camera->MovePerpendicularFwAxis(dx, dy);
					}
					else
					{
						resetMouseProcessing();
					}
				}
			}
		}
		break;
		}
	}

	//SceneObject Picking
	bool PickingPassExists()
	{
		return mousePicking.pickingPass != nullptr;
	}

	void CreatePickingPass()
	{
		mousePicking.pickingPass = GetRenderPassInstance(nullptr, 0, FindRenderPassUUIDByName("PickingPass"), HWNDWIDTH, HWNDHEIGHT);
	}

	void DestroyPickingPass()
	{
		UnbindPickingRenderables();

		if (mousePicking.pickingPass) {
			DestroyRenderPassInstance(mousePicking.pickingPass);
		}
		mousePicking.pickingPass = nullptr;
		if (mousePicking.pickingCpuBuffer)
		{
			mousePicking.pickingCpuBuffer = nullptr;
		}
	}

	void BindPickingRenderables()
	{
		for (auto& r : GetRenderables())
		{
			BindRenderableToPickingPass(r);
		}
	}

	void BindRenderableToPickingPass(std::shared_ptr<Renderable> r)
	{
		std::shared_ptr<RenderPassInstance>& pass = mousePicking.pickingPass;
		r->CreateRenderPassMaterialsInstances(pass);
		r->CreateRenderPassConstantsBuffersInstances(pass);
		r->CreateRenderPassRootSignatures(pass);
		r->CreateRenderPassPipelineStates(pass);
	}

	void UnbindPickingRenderables()
	{
		for (auto& r : GetRenderables())
		{
			UnbindRenderableFromPickingPass(r);
		}
	}

	void UnbindRenderableFromPickingPass(std::shared_ptr<Renderable> r)
	{
		std::shared_ptr<RenderPassInstance>& pass = mousePicking.pickingPass;
		r->DestroyRenderPassMaterialsInstances(pass);
		r->DestroyRenderPassConstantsBuffersInstances(pass);
		r->DestroyRenderPassRootSignatures(pass);
		r->DestroyRenderPassPipelineStates(pass);
	}

	void RenderPickingPass(std::shared_ptr<Camera> camera)
	{
		if (!camera || !mousePicking.doPicking || !mousePicking.pickingPass) return;

#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, L"Scene Picker");
#endif

		mousePicking.pickingPass->rendererToTexturePass->Pass([camera]()
			{
				unsigned int backBufferIndex = renderer->backBufferIndex;
				unsigned int objectId = 1U;
				for (auto& r : GetRenderables())
				{
					//OutputDebugStringA(("RenderPickingPass:" + r->name() + ":" + std::to_string(objectId) + "\n").c_str());
					if (!r->visible() || boundingBox == r) continue;

					r->WriteConstantsBuffer("objectId", objectId, backBufferIndex);
					r->Render(mousePicking.pickingPass, camera);
					objectId++;
				}
			}
		);

#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif
	}

	void PickFromScene()
	{
		if (!mousePicking.doPicking) return;
		mousePicking.doPicking = false;
		currentMouseMode = MOUSE_GAMEAREA_MODE_NONE;

		if (!mousePicking.pickingPass) return;

		DeviceUtils::CaptureTexture(
			renderer->d3dDevice,
			renderer->commandQueue,
			mousePicking.pickingPass->rendererToTexturePass->renderToTexture[0]->renderToTexture,
			mousePicking.pickingPass->rendererToTexturePass->renderToTexture[0]->width * sizeof(unsigned int),
			mousePicking.pickingPass->rendererToTexturePass->renderToTexture[0]->resourceDesc,
			mousePicking.pickingCpuBuffer,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);

		size_t width = static_cast<size_t>(hWndRect.right - hWndRect.left);
		size_t height = static_cast<size_t>(hWndRect.bottom - hWndRect.top);
		size_t Begin = 0;
		size_t End = width * height * sizeof(unsigned int);
		size_t offset = (width * mousePicking.pickingY + mousePicking.pickingX);

		D3D12_RANGE readbackBufferRange{ Begin, End };
		unsigned int* pReadbackBufferData{};
		mousePicking.pickingCpuBuffer->Map(0, &readbackBufferRange, reinterpret_cast<void**>(&pReadbackBufferData));

		PickSceneObject(pReadbackBufferData[offset]);

		D3D12_RANGE emptyRange{ 0, 0 };
		mousePicking.pickingCpuBuffer->Unmap(0, &emptyRange);
	}

	void PickSceneObject(unsigned int pickedObjectId)
	{
		if (ImGuizmo::IsUsing())
		{
			return;
		}

		if (pickedObjectId == 0U)
		{
			SelectSceneObject("");
			return;
		}

		unsigned int objectId = 1U;
		for (auto& r : GetRenderables())
		{
			if (!r->visible() || r == boundingBox) continue;

			if (pickedObjectId == objectId)
			{
				//OutputDebugStringA(("PickSceneObject:" + r->name() + ":" + std::to_string(objectId) + "\n").c_str());
				SelectSceneObject(r->uuid());
				break;
			}
			objectId++;
		}
	}

	void ReleasePickingPassResources()
	{
		if (mousePicking.pickingPass) mousePicking.pickingPass->rendererToTexturePass->ReleaseResources();
	}

	void ResizePickingPass(unsigned int width, unsigned int height)
	{
		if (mousePicking.pickingPass) mousePicking.pickingPass->rendererToTexturePass->Resize(width, height);
	}

	//JObjects Creation
	void StartSceneObjectCreation(SceneObjectType type)
	{
		sceneObjectModal.json = GetSceneObjectJson(type);
		sceneObjectModal.atts = GetSceneObjectRequiredAttributes(type);
		sceneObjectModal.drawers = GetSceneObjectCreatorDrawers(type);
		sceneObjectModal.validators = GetSceneObjectValidators(type);
		sceneObjectModal.type = type;
		sceneObjectModal.creating = true;
		sceneObjectModal.onCreate = CreateSceneObject;
	}

	void StartTemplateCreation(TemplateType type)
	{
		templateModal.json = GetTemplateJson(type);
		templateModal.atts = GetTemplateRequiredAttributes(type);
		templateModal.drawers = GetTemplateCreatorDrawers(type);
		templateModal.validators = GetTemplateValidators(type);
		templateModal.type = type;
		templateModal.creating = true;
		templateModal.onCreate = CreateTemplate;
	}
};

