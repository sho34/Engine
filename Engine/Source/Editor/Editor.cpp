#include "pch.h"

#if defined(_EDITOR)
#include "Editor.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/RenderPass/RenderPass.h"
#include <imgui.h>
#include <Mouse.h>
#include <Application.h>
#include <Editor.h>
#include "../Renderer/DeviceUtils/Resources/Resources.h"

extern HWND hWnd;
extern RECT hWndRect;
extern std::unique_ptr<DirectX::Mouse> mouse;
extern std::shared_ptr<Renderer> renderer;
extern std::string gameAppTitle;
extern std::filesystem::path levelToLoad;

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Editor {

	std::string currentLevelName = defaultLevelName;

	bool initialized = false;
	bool maximized = true;
	bool dragDrop = false;
	int lastMouseX;
	int lastMouseY;
	bool boundingBoxRendering = false;
	std::shared_ptr<Renderable> boundingBox = nullptr;
	_SceneObjects soTab = _SceneObjects::SO_Renderables;
	std::string selSO = "";
	_Templates tempTab = _Templates::T_Materials;
	std::string selTemp = "";
	float titleBH = static_cast<float>(ApplicationBarBottom);
	float panW = static_cast<float>(RightPanelWidth);
	ImGuiWindowFlags panFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

	ImGui_ImplDX12_InitInfo init_info = {};

	enum MouseGameAreaMode
	{
		MOUSE_GAMEAREA_MODE_NONE,
		MOUSE_GAMEAREA_MODE_PICKING,
		MOUSE_GAMEAREA_MODE_GIZMO,
		MOUSE_GAMEAREA_MODE_CAMERA
	};
	MouseGameAreaMode currentMouseMode = MOUSE_GAMEAREA_MODE_NONE;

	struct
	{
		std::shared_ptr<RenderToTexturePass> pickingPass;
		CComPtr<ID3D12Resource> pickingCpuBuffer;
		unsigned int pickingX = 0U;
		unsigned int pickingY = 0U;
		bool doPicking = false;
		void Reset()
		{
			pickingX = 0U;
			pickingY = 0U;
			doPicking = false;
		}
		void StartPicking(DirectX::Mouse::State state)
		{
			pickingX = state.x;
			pickingY = state.y;
			doPicking = false;
		}
		bool CanPick(DirectX::Mouse::State state) const
		{
			/*
			std::string str = "button:" + std::string((!state.leftButton) ? "true" : "false") + " " +
				"state.x:" + std::to_string(state.x) + " " + "pickingX:" + std::to_string(pickingX) + " " +
				"state.y:" + std::to_string(state.y) + " " + "pickingY:" + std::to_string(pickingY) + " " +
				"\n";
			OutputDebugStringA(str.c_str());
			*/
			return (!state.leftButton && state.x == pickingX && state.y == pickingY);
		}
		void Pick()
		{
			doPicking = true;
		}
		bool MouseMoved(DirectX::Mouse::State state) const
		{
			return (state.leftButton && (state.x != pickingX || state.y != pickingY));
		}

	} mousePicking;

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

	void CreateRenderableBoundingBox()
	{
		boundingBox = CreateRenderable(R"(
			{
				"meshMaterials": [ { "material": "2e4d8bf0-0761-45d9-8313-17cdf9b5f8fc", "mesh" : "30f15e68-db42-46fa-b846-b2647a0ac9b9" } ],
				"meshMaterialsShadowMap": [],
				"name": "EditorBoundingBox",
				"uuid": "797b3a2b-6854-47ab-8e07-1974055e490d",
				"position": [ 0.0, 0.0, 0.0],
				"primitiveTopologyType":"LINE",
				"rotation": [ 0.0, 0.0, 0.0 ],
				"scale": [ 1.0, 1.0, 1.0],
				"skipMeshes":[],
				"visible":false,
				"hidden":true
			}
		)"_json);
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

		DestroyRenderable(boundingBox);

		// Cleanup
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	bool WndProcHandlerEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
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

		if (mouseState.leftButton) {

			if (inDragBounds() && !dragDrop) {
				dragDrop = true;
			}

			if (dragDrop) {
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
			else {
				lastMouseX = mouseState.x;
				lastMouseY = mouseState.y;
			}

		}
		else {
			dragDrop = false;
			lastMouseX = mouseState.x;
			lastMouseY = mouseState.y;
		}
	}

	void DrawEditor(std::shared_ptr<Camera> camera) {

		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		DrawApplicationBar();
		DrawRightPanel();
		DrawSelectedObjectGuizmo(camera);

		// Rendering
		ImGui::Render();

		// Render Dear ImGui graphics
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), renderer->commandList);
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

				DrawItemWithEnabledState([]
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
					PostMessageA(hWnd,WM_QUIT,0,0);
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
						HWND desktopHwnd = GetDesktopWindow();
						RECT desktopRect;
						GetClientRect(desktopHwnd, &desktopRect);
						int winWidth = desktopRect.right;
						int winHeight = desktopRect.bottom - 40;
						SetWindowPos(hWnd, HWND_TOP, 0, 0, winWidth, winHeight, 0);
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

	void DrawTableRows(auto GetObjects, auto OnSelect, auto OnDelete)
	{
		ImGui::TableSetupColumn("Delete", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Object", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TableHeader("");
		ImGui::TableSetColumnIndex(1);
		ImGui::TableHeader("Object");

		int row = 0;
		for (UUIDName uuidName : GetObjects())
		{
			std::string name = std::get<1>(uuidName);
			std::string uuid = std::get<0>(uuidName);
			std::string rowName = "table-row-" + uuid;
			ImGui::PushID(rowName.c_str());
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (ImGui::SmallButton(ICON_FA_TIMES)) { OnDelete(uuid); }
			ImGui::TableSetColumnIndex(1);
			if (ImGui::TextLink(name.c_str())) { OnSelect(uuid); }
			ImGui::PopID();
		}
	}

	void BuildAssetsTree(std::map<std::string, std::any>& assets, std::vector<std::string>& parts)
	{
		std::string part = *parts.begin();
		if (!assets.contains(part))
		{
			assets.insert_or_assign(part, std::map<std::string, std::any>());
		}

		if (parts.begin() != parts.end() - 1)
		{
			std::map<std::string, std::any>& child = std::any_cast<std::map<std::string, std::any>&>(assets.at(part));
			std::vector<std::string> nextParts;
			std::copy(parts.begin() + 1, parts.end(), std::back_inserter(nextParts));
			BuildAssetsTree(child, nextParts);
		}
	}

	void DrawAssetTreeNodes(std::map<std::string, std::any>& dump, std::string path, auto OnPick, auto OnDelete)
	{
		for (auto it = dump.begin(); it != dump.end(); it++)
		{
			std::map<std::string, std::any>& child = std::any_cast<std::map<std::string, std::any>&>(it->second);
			std::string p = path + (path.empty() ? "" : "/") + it->first;

			if (child.empty())
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.15f));
				{
					ImGui::PushID((std::string("delete-") + p).c_str());
					{
						if (ImGui::Button(ICON_FA_TIMES, ImVec2(16.0f, 16.0f)))
						{
							OnDelete(p);
						}
					}
					ImGui::PopID();
				}
				ImGui::PopStyleVar();

				ImGui::SameLine();

				if (ImGui::TreeNodeEx(it->first.c_str(), ImGuiTreeNodeFlags_Leaf))
				{
					if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiPopupFlags_MouseButtonLeft))
					{
						OnPick(p);
					}
					ImGui::TreePop();
				}
			}
			else
			{
				if (ImGui::TreeNodeEx(it->first.c_str()))
				{
					DrawAssetTreeNodes(child, p, OnPick, OnDelete);
					ImGui::TreePop();
				}
			}
		}
	}

	void DrawAssetsTree(auto GetObjects, auto OnPick, auto OnDelete, std::string ignorePrefix)
	{
		std::map<std::string, std::any> assets;

		for (UUIDName uuidName : GetObjects())
		{
			std::string uuid = std::get<0>(uuidName);
			std::string path = std::get<1>(uuidName);

			if (!ignorePrefix.empty())
			{
				path = std::regex_replace(path, std::regex(ignorePrefix), "");
			}

			std::vector<std::string> parts = nostd::split(path, "/");
			BuildAssetsTree(assets, parts);
		}

		DrawAssetTreeNodes(assets, "", OnPick, OnDelete);
	}

	void DrawListPanel(auto OnCreate, auto DrawObjects)
	{
		if (ImGui::SmallButton(ICON_FA_PLUS))
		{
			OnCreate();
		}

		DrawObjects();
	}

	template<typename T, typename V>
	void DrawTabPanel(ImVec2 pos, ImVec2 size, const char* name, T& tab,
		std::unordered_map<T, std::string> TabToStr,
		V& selected,
		std::map<T, std::function<void()>> OnCreate,
		auto DrawObjects
	)
	{
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		std::string panelName = "panel-" + TabToStr.at(tab);
		ImGui::BeginChild(panelName.c_str());
		{
			std::string tabBarName = "tabBar-" + std::string(name);
			ImGui::BeginTabBar(tabBarName.c_str());
			{
				for (auto& [type, name] : TabToStr) {
					ImGuiTabItemFlags_ flag = (tab == type) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
					if (flag == ImGuiTabItemFlags_SetSelected)
					{
						ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.230f, 0.230f, 0.230f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.230f, 0.230f, 0.230f, 1.0f));
					}
					if (ImGui::TabItemButton(name.c_str(), flag))
					{
						tab = type;
					}
					if (flag == ImGuiTabItemFlags_SetSelected)
					{
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
					}

				}
				DrawListPanel(
					OnCreate.at(tab),
					DrawObjects
				);
			}
			ImGui::EndTabBar();
		}
		ImGui::EndChild();
	}

	template<typename T, typename V>
	void DrawDetailPanel(ImVec2 pos, ImVec2 size, T& tab,
		V& selected,
		std::unordered_map<T, std::string> SelectedPrefix,
		std::map<T, std::function<std::string(V)>> GetSelectedName,
		std::map<T, std::function<void(V, ImVec2, ImVec2, bool)>> DrawSelectedPanel,
		std::map<T, std::function<void(V&)>> OnDeSelect
	)
	{
		bool pop = false;
		std::string panelName = "panel-" + SelectedPrefix.at(tab);// + GetSelectedName.at(tab)(selected);
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		ImGui::BeginChild(panelName.c_str(), ImVec2(0, 0), ImGuiChildFlags_AlwaysUseWindowPadding);
		{
			if (ImGui::SmallButton("<<")) { pop = true; }
			ImGui::SameLine();
			ImGui::Text(GetSelectedName.at(tab)(selected).c_str());

			ImGui::NewLine();
			DrawSelectedPanel.at(tab)(selected, pos, size, pop);
		}
		ImGui::EndChild();
		if (pop) { OnDeSelect.at(tab)(selected); }
	}

	void DrawRightPanel() {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImVec2 panPos = ImVec2(viewport->Size.x - panW, titleBH);
		ImVec2 panSize = ImVec2(panW, viewport->Size.y - titleBH + 10.0f);

		ImGui::SetNextWindowPos(panPos);
		ImGui::SetNextWindowSize(panSize);
		ImGui::Begin("Right panel", (bool*)1, panFlags);
		{
			ImVec2 soPos = ImVec2(panPos.x, panPos.y);
			ImVec2 tempPos = ImVec2(panPos.x, panSize.y * 0.5f + 20.0f);
			ImVec2 soSize = ImVec2(panSize.x, panSize.y * 0.5f);
			ImVec2 tempSize = ImVec2(panSize.x, panSize.y * 0.5f - 20.0f);

			if (selSO.empty())
			{
				std::map<_SceneObjects, std::function<void(std::string, std::string&)>> SetSelectedSceneObjectProxy;

				std::transform(SetSelectedSceneObject.begin(), SetSelectedSceneObject.end(), std::inserter(SetSelectedSceneObjectProxy, SetSelectedSceneObjectProxy.end()), [](auto pair)
					{
						return std::pair<_SceneObjects, std::function<void(std::string, std::string&)>>(pair.first, [pair](std::string a, std::string& b)
							{
								boundingBox->visible(true);
								pair.second(a, b);
							}
						);
					}
				);

				DrawTabPanel(soPos, soSize, "Scene Objects", soTab, SceneObjectsToStr, selSO,
					CreateSceneObject,
					[SetSelectedSceneObjectProxy]()
					{
						std::string tableName = (SceneObjectsToStr.at(soTab) + "-table");
						if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_PadOuterX))
						{
							DrawTableRows(
								GetSceneObjects.at(soTab),
								[SetSelectedSceneObjectProxy](std::string uuid) { SetSelectedSceneObjectProxy.at(soTab)(uuid, selSO); },
								[](std::string uuid) { DeleteSceneObject.at(soTab)(uuid); }
							);
							ImGui::EndTable();
						}
					}
				);
			}
			else
			{
				std::map<_SceneObjects, std::function<void(std::string&)>> DeSelectSceneObjectProxy;

				std::transform(DeSelectSceneObject.begin(), DeSelectSceneObject.end(), std::inserter(DeSelectSceneObjectProxy, DeSelectSceneObjectProxy.end()), [](auto pair)
					{
						return std::pair<_SceneObjects, std::function<void(std::string&)>>(pair.first, [pair](std::string& b)
							{
								boundingBox->visible(false);
								pair.second(b);
							}
						);
					}
				);

				DrawDetailPanel(soPos, soSize, soTab, selSO, SceneObjectsToStr, GetSceneObjectName, DrawSceneObjectPanel, DeSelectSceneObjectProxy);
			}

			if (selTemp.empty())
			{
				DrawTabPanel(tempPos, tempSize, "Templates", tempTab, TemplatesToStr, selTemp,
					CreateTemplate,
					[]()
					{
						if (std::set({ T_Materials,T_Models3D,T_Shaders,T_Sounds }).contains(tempTab))
						{
							std::string tableName = (TemplatesToStr.at(tempTab) + "-table");
							if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_PadOuterX))
							{
								DrawTableRows(
									GetTemplates.at(tempTab),
									[](std::string uuid) { SetSelectedTemplate.at(tempTab)(uuid, selTemp); },
									[](std::string uuid) { DeleteTemplate.at(tempTab)(uuid); }
								);
								ImGui::EndTable();
							}
						}
						else if (std::set({ T_Textures }).contains(tempTab))
						{
							DrawAssetsTree(GetTemplates.at(tempTab), [](std::string texName)
								{
									std::string uuid = FindTextureUUIDByName(defaultAssetsFolder + texName);
									SetSelectedTemplate.at(tempTab)(uuid, selTemp);
								},
								[](std::string texName)
								{
									std::string uuid = FindTextureUUIDByName(defaultAssetsFolder + texName);
									DeleteTemplate.at(tempTab)(uuid);
								}
								, defaultAssetsFolder
							);
						}
					}
				);
			}
			else
			{
				DrawDetailPanel(tempPos, tempSize, tempTab, selTemp, TemplatesToStr, GetTemplateName, DrawTemplatePanel, DeSelectTemplate);
			}
		}
		ImGui::End();

		if (selSO == "")
		{
			DrawSceneObjectsPopups.at(soTab)();
		}
		if (selTemp == "")
		{
			DrawTemplatesPopups.at(tempTab)();
		}
	}

	XMFLOAT3 GetYawPitchRoll(XMFLOAT4X4 transform)
	{
		float pitch = XMScalarASin(-transform._32);

		XMVECTOR from(XMVectorSet(transform._12, transform._31, 0.0f, 0.0f));
		XMVECTOR to(XMVectorSet(transform._22, transform._33, 0.0f, 0.0f));
		XMVECTOR res(XMVectorATan2(from, to));

		float roll = XMVectorGetX(res);
		float yaw = XMVectorGetY(res);

		return XMFLOAT3(roll, pitch, yaw);
	}

	void DrawRenderableGuizmo(std::shared_ptr<Camera> camera)
	{
		std::shared_ptr<Renderable> renderable = GetRenderable(selSO);

		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 proj;
		XMStoreFloat4x4(&world, renderable->world());
		XMStoreFloat4x4(&view, camera->ViewMatrix());
		XMStoreFloat4x4(&proj, camera->perspective.projectionMatrix);

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
		ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *world.m, *delta.m, NULL, NULL, NULL);

		XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
		XMVECTOR XMtranslation;
		XMVECTOR XMrotation;
		XMVECTOR XMscale;
		XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

		if (gizmoOperation == ImGuizmo::OPERATION::TRANSLATE)
		{
			XMFLOAT3 newPos = renderable->position();
			newPos.x += XMtranslation.m128_f32[0];
			newPos.y += XMtranslation.m128_f32[1];
			newPos.z += XMtranslation.m128_f32[2];
			renderable->position(newPos);
		}
		else if (gizmoOperation == ImGuizmo::OPERATION::ROTATE)
		{
			XMFLOAT3 newRot = renderable->rotation();
			XMFLOAT3 rotDelta = GetYawPitchRoll(delta);
			newRot.x += rotDelta.x;
			newRot.y += rotDelta.y;
			newRot.z += rotDelta.z;
			renderable->rotation(newRot);
		}
		else if (gizmoOperation == ImGuizmo::OPERATION::SCALE)
		{
			XMFLOAT3 newScale = renderable->scale();
			newScale.x *= XMscale.m128_f32[0];
			newScale.y *= XMscale.m128_f32[1];
			newScale.z *= XMscale.m128_f32[2];
			renderable->scale(newScale);
		}
	}

	void DrawLightGuizmo(std::shared_ptr<Camera> camera)
	{
		std::shared_ptr<Light> light = GetLight(selSO);

		if (light->lightType() == LT_Ambient) return;

		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 proj;
		XMStoreFloat4x4(&world, light->world());
		XMStoreFloat4x4(&view, camera->ViewMatrix());
		XMStoreFloat4x4(&proj, camera->perspective.projectionMatrix);

		if ((ImGui::IsKeyPressed(ImGuiKey_T) && light->lightType() != LT_Directional) || light->lightType() == LT_Point) // t ky
		{
			gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			gizmoMode = ImGuizmo::MODE::WORLD;
		}
		if ((ImGui::IsKeyPressed(ImGuiKey_R) && light->lightType() != LT_Point) || light->lightType() == LT_Directional) // r key
		{
			gizmoOperation = ImGuizmo::OPERATION::ROTATE;
			gizmoMode = ImGuizmo::MODE::WORLD;
		}

		XMFLOAT4X4 delta;
		ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *world.m, *delta.m, NULL, NULL, NULL);

		XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
		XMVECTOR XMtranslation;
		XMVECTOR XMrotation;
		XMVECTOR XMscale;
		XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

		if (gizmoOperation == ImGuizmo::OPERATION::TRANSLATE)
		{
			XMFLOAT3 newPos = light->position();
			newPos.x += XMtranslation.m128_f32[0];
			newPos.y += XMtranslation.m128_f32[1];
			newPos.z += XMtranslation.m128_f32[2];
			light->position(newPos);
			if (light->hasShadowMaps())
			{
				light->shadowMapCameras[0]->position(newPos);
			}
		}
		else if (gizmoOperation == ImGuizmo::OPERATION::ROTATE)
		{
			XMFLOAT3 newRot = light->rotation();
			XMFLOAT3 rotDelta = GetYawPitchRoll(delta);
			newRot.x += rotDelta.x;
			newRot.y += rotDelta.y;
			newRot.z += rotDelta.z;
			light->rotation(newRot);
			if (light->hasShadowMaps())
			{
				light->shadowMapCameras[0]->rotation(XMFLOAT3({ newRot.x, newRot.y, 0.0f }));
			}
		}
	}

	void DrawCameraGuizmo(std::shared_ptr<Camera> camera)
	{
		std::shared_ptr<Camera> cam = GetCamera(selSO);

		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 proj;
		XMStoreFloat4x4(&world, cam->world());
		XMStoreFloat4x4(&view, camera->ViewMatrix());
		XMStoreFloat4x4(&proj, camera->perspective.projectionMatrix);

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

		XMFLOAT4X4 delta;
		ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *world.m, *delta.m, NULL, NULL, NULL);

		XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
		XMVECTOR XMtranslation;
		XMVECTOR XMrotation;
		XMVECTOR XMscale;
		XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

		if (gizmoOperation == ImGuizmo::OPERATION::TRANSLATE)
		{
			XMFLOAT3 newPos = cam->position();
			newPos.x += XMtranslation.m128_f32[0];
			newPos.y += XMtranslation.m128_f32[1];
			newPos.z += XMtranslation.m128_f32[2];
			cam->position(newPos);
		}
		else if (gizmoOperation == ImGuizmo::OPERATION::ROTATE)
		{
			XMFLOAT3 newRot = cam->rotation();
			XMFLOAT3 rotDelta = GetYawPitchRoll(delta);
			newRot.x += rotDelta.x;
			newRot.y += rotDelta.y;
			newRot.z += rotDelta.z;
			cam->rotation(newRot);
		}
	}

	void DrawSoundGuizmo(std::shared_ptr<Camera> camera)
	{
		std::shared_ptr<Scene::SoundEffect> sound = GetSoundEffect(selSO);

		XMFLOAT4X4 world;
		XMFLOAT4X4 view;
		XMFLOAT4X4 proj;
		XMStoreFloat4x4(&world, sound->world());
		XMStoreFloat4x4(&view, camera->ViewMatrix());
		XMStoreFloat4x4(&proj, camera->perspective.projectionMatrix);

		if (ImGui::IsKeyPressed(ImGuiKey_T)) // t ky
		{
			gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			gizmoMode = ImGuizmo::MODE::WORLD;
		}
		/*
		if (ImGui::IsKeyPressed(ImGuiKey_R)) // r key
		{
			gizmoOperation = ImGuizmo::OPERATION::ROTATE;
			gizmoMode = ImGuizmo::MODE::WORLD;
		}
		*/

		XMFLOAT4X4 delta;
		ImGuizmo::Manipulate(*view.m, *proj.m, gizmoOperation, gizmoMode, *world.m, *delta.m, NULL, NULL, NULL);

		XMMATRIX XMdelta = XMLoadFloat4x4(&delta);
		XMVECTOR XMtranslation;
		XMVECTOR XMrotation;
		XMVECTOR XMscale;
		XMMatrixDecompose(&XMscale, &XMrotation, &XMtranslation, XMdelta);

		if (gizmoOperation == ImGuizmo::OPERATION::TRANSLATE)
		{
			XMFLOAT3 newPos = sound->position();
			newPos.x += XMtranslation.m128_f32[0];
			newPos.y += XMtranslation.m128_f32[1];
			newPos.z += XMtranslation.m128_f32[2];
			sound->position(newPos);
		}
		/*
		else if (gizmoOperation == ImGuizmo::OPERATION::ROTATE)
		{
			XMFLOAT3 newRot = sound->rotation();
			XMFLOAT3 rotDelta = GetYawPitchRoll(delta);
			newRot.x += rotDelta.x;
			newRot.y += rotDelta.y;
			newRot.z += rotDelta.z;
			sound->rotation(newRot);
		}
		*/
	}

	void DrawSelectedObjectGuizmo(std::shared_ptr<Camera> camera)
	{
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::BeginFrame();

		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		if (selSO == "" || camera == nullptr) return;

		ImGuizmo::SetID(0);

		switch (soTab)
		{
		case SO_Renderables:
		{
			DrawRenderableGuizmo(camera);
		}
		break;
		case SO_Lights:
		{
			DrawLightGuizmo(camera);
		}
		break;
		case SO_Cameras:
		{
			DrawCameraGuizmo(camera);
		}
		break;
		case SO_SoundEffects:
		{
			DrawSoundGuizmo(camera);
		}
		break;
		}
	}

	void OpenLevelFile()
	{
		OpenFile([](std::filesystem::path path)
			{
				std::filesystem::path jsonFilePath = path;
				jsonFilePath.replace_extension(".json");
				levelToLoad = jsonFilePath;
				soTab = _SceneObjects::SO_Renderables;
				selSO = "";
				tempTab = _Templates::T_Materials;
				selTemp = "";
			},
			defaultLevelsFolder
		);
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
	}

	bool SaveFileDialog(std::wstring& path, std::wstring defaultDirectory = L"", std::wstring defaultFileName = L"", std::pair<COMDLG_FILTERSPEC*, int>* pFilterInfo = nullptr)
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

	void SaveTemplates()
	{
		using namespace Templates;
		Templates::SaveTemplates(defaultTemplatesFolder, Shader::templateName, WriteShadersJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Material::templateName, WriteMaterialsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Model3D::templateName, WriteModel3DsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Sound::templateName, WriteSoundsJson);
		Templates::SaveTemplates(defaultTemplatesFolder, Texture::templateName, WriteTexturesJson);
	}

	void SelectSceneObject(_SceneObjects objectType, std::string uuid)
	{
		soTab = objectType;
		selSO = uuid;
		gizmoOperation = ImGuizmo::TRANSLATE;
		gizmoMode = ImGuizmo::WORLD;
	}

	void RenderSelectedLightShadowMapChain()
	{
		if (soTab != _SceneObjects::SO_Lights || selSO == "") return;

		std::shared_ptr<Light> light = GetLight(selSO);
		if (!light->hasShadowMaps() || light->shadowMapUpdateFlags) return;

		light->RenderShadowMapMinMaxChain();
	}

	void WriteRenderableBoundingBoxConstantsBuffer()
	{
		if (selSO.empty()) return;

		switch (soTab)
		{
		case SO_Renderables:
		{
			GetRenderable(selSO)->FillRenderableBoundingBox(boundingBox);
		}
		break;
		case SO_Lights:
		{
			GetLight(selSO)->FillRenderableBoundingBox(boundingBox);
		}
		break;
		case SO_Cameras:
		{
			GetCamera(selSO)->FillRenderableBoundingBox(boundingBox);
		}
		break;
		case SO_SoundEffects:
		{
			GetSoundEffect(selSO)->FillRenderableBoundingBox(boundingBox);
		}
		break;
		default:
		{
		}
		break;
		}

		boundingBox->WriteConstantsBuffer(renderer->backBufferIndex);
	}

	void DrawOkPopup(unsigned int& flag, unsigned int cmpFlag, std::string popupId, std::function<void()> drawContent)
	{
		if (flag == cmpFlag)
		{
			ImGui::OpenPopup(popupId.c_str());
			if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				drawContent();
				if (ImGui::Button("Ok")) { flag = 0U; }
				ImGui::EndPopup();
			}
		}
	}

	void DrawCreateWindow(unsigned int& flag, unsigned int cmpFlag, std::string popupId, std::function<void(std::function<void()>)> drawContent)
	{
		if (flag == cmpFlag)
		{
			ImGui::OpenPopup(popupId.c_str());
			if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				drawContent([&flag] {flag = 0U; });
				ImGui::EndPopup();
			}
		}
	}

	void ImDrawMaterialShaderSelection(nlohmann::json& mat, std::string key, ShaderType type, std::function<void()> cb)
	{
		ImGui::Text(("Shader " + key).c_str());

		ImGui::PushID(("shader-name-combo-" + key).c_str());
		{
			std::string shaderUUID = mat.contains(key) ? mat.at(key) : "";
			if (shaderUUID != "")
			{
				if (ImGui::Button(ICON_FA_FILE_CODE))
				{
					Editor::tempTab = T_Shaders;
					Editor::selTemp = shaderUUID;
				}
				ImGui::SameLine();
			}

			std::vector<UUIDName> selectables = GetShadersUUIDsNamesByType(type);
			SortUUIDByName(selectables);
			std::string shaderName = shaderUUID != "" ? GetShaderName(shaderUUID) : " ";
			UUIDName selected = std::tie(shaderUUID, shaderName);
			DrawComboSelection(selected, selectables, [&mat, key, cb](UUIDName shader)
				{
					mat.at(key) = std::get<0>(shader);
					cb();
				}, ""
			);
		}
		ImGui::PopID();
	}

	//MOUSE PROCESSING
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
		if (AnyTemplatePopupOpen())
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

		DirectX::Mouse::State state = mouse->GetState();

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
						float dx = static_cast<float>(mousedx) * 0.01f;
						float dy = static_cast<float>(mousedy) * 0.01f;
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
						float dx = static_cast<float>(mousedx) * 0.01f;
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

	//OBJECT PICKING
	void CreatePickingPass()
	{
		unsigned int width = static_cast<unsigned int>(hWndRect.right - hWndRect.left);
		unsigned int height = static_cast<unsigned int>(hWndRect.bottom - hWndRect.top);

		mousePicking.pickingPass = CreateRenderPass("pickingPass", { DXGI_FORMAT_R32_UINT }, DXGI_FORMAT_D32_FLOAT, width, height);
	}

	void DestroyPickingPass()
	{
		mousePicking.pickingPass = nullptr;
	}

	void MapPickingRenderables()
	{
		std::string pickingMaterialUUID = FindMaterialUUIDByName("Picking");

		auto& renderables = GetRenderables();
		for (auto& [uuid, renderable] : renderables)
		{
			renderable->CreatePickingComponents(mousePicking.pickingPass->passHash, pickingMaterialUUID);
		}
	}

	void RenderPickingPass(std::shared_ptr<Camera> camera)
	{
		if (!mousePicking.doPicking) return;

#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, L"Scene Picker");
#endif

		auto WritePickingConstantsBuffer = [](std::shared_ptr<Renderable> r, unsigned int objectId, unsigned int backbufferIndex)
			{
				XMFLOAT3 posV = r->position();
				XMFLOAT3 rotV = r->rotation();
				XMFLOAT3 scaleV = r->scale();
				XMMATRIX rotationM = XMMatrixRotationRollPitchYawFromVector({ rotV.x, rotV.y, rotV.z, 0.0f });
				XMMATRIX scaleM = XMMatrixScalingFromVector({ scaleV.x, scaleV.y, scaleV.z });
				XMMATRIX positionM = XMMatrixTranslationFromVector({ posV.x, posV.y, posV.z });
				XMMATRIX world = XMMatrixMultiply(XMMatrixMultiply(scaleM, rotationM), positionM);

				r->WriteConstantsBuffer(r->pickingMeshMaterials, r->pickingMeshConstantsBuffer, "world", world, backbufferIndex);
				r->WriteConstantsBuffer(r->pickingMeshMaterials, r->pickingMeshConstantsBuffer, "objectId", objectId, backbufferIndex);
			};

		mousePicking.pickingPass->Pass([WritePickingConstantsBuffer, camera](size_t passHash)
			{
				unsigned int backBufferIndex = renderer->backBufferIndex;
				camera->WriteConstantsBuffer(backBufferIndex);

				unsigned int objectId = 1U;
				for (auto& [name, r] : GetRenderables())
				{
					if (!r->visible()) continue;

					WritePickingConstantsBuffer(r, objectId, backBufferIndex);

					r->RenderCustomizable(passHash,
						r->pickingMeshMaterials,
						r->pickingMeshConstantsBuffer,
						r->pickingMeshHashedRootSignatures,
						r->pickingMeshHashedPipelineStates,
						camera
					);
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

		DeviceUtils::CaptureTexture(
			renderer->d3dDevice,
			renderer->commandQueue,
			mousePicking.pickingPass->renderToTexture[0]->renderToTexture,
			mousePicking.pickingPass->renderToTexture[0]->width * sizeof(unsigned int),
			mousePicking.pickingPass->renderToTexture[0]->resourceDesc,
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

		Editor::PickSceneObject(pReadbackBufferData[offset]);

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
			SelectSceneObject(_SceneObjects::SO_Renderables, "");
			boundingBox->visible(false);
			return;
		}

		unsigned int objectId = 1U;
		for (auto& [uuid, r] : GetRenderables())
		{
			if (!r->visible()) continue;

			if (pickedObjectId == objectId)
			{
				if (uuid != selSO)
				{
					SelectSceneObject(_SceneObjects::SO_Renderables, uuid);
					boundingBox->visible(true);
				}
				break;
			}
			objectId++;
		}

	}

	void ReleasePickingPassResources()
	{
		if (mousePicking.pickingPass) mousePicking.pickingPass->ReleaseResources();
	}

	void ResizePickingPass(unsigned int width, unsigned int height)
	{
		if (mousePicking.pickingPass) mousePicking.pickingPass->Resize(width, height);
	}
};

#endif