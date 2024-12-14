#include "pch.h"

#if defined(_EDITOR)
#include "Editor.h"
#include "../Renderer/Renderer.h"
#include "../Scene/Renderable/Renderable.h"
#include "../Scene/Lights/Lights.h"
#include "../Scene/Camera/Camera.h"

extern bool appDone;
extern HWND hWnd;
extern std::unique_ptr<DirectX::Mouse> mouse;
extern std::shared_ptr<Renderer> renderer;

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Editor {

  std::wstring currentLevelName = defaultLevelName;

  bool dragDrop = false;
  INT lastMouseX;
  INT lastMouseY;

  void InitEditor()
  {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hWnd);

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = renderer->d3dDevice;
    init_info.CommandQueue = renderer->commandQueue;
    init_info.NumFramesInFlight = renderer->numFrames;
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

    // Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
    // (current version of the backend will only allocate one descriptor, future versions will need to allocate more)
    init_info.SrvDescriptorHeap = DeviceUtils::ConstantsBuffer::GetCSUDescriptorHeap();
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
      *out_cpu_handle = GetCpuDescriptorHandleCurrent();
      *out_gpu_handle = GetGpuDescriptorHandleCurrent();
      GetCpuDescriptorHandleCurrent().Offset(GetCSUDescriptorSize());
      GetGpuDescriptorHandleCurrent().Offset(GetCSUDescriptorSize());
      /*return g_pd3dSrvDescHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle);*/
      };
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
      /*return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle);*/
      };
    ImGui_ImplDX12_Init(&init_info);

  }

  void DestroyEditor() {
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

    auto inDragBounds=[&dragRect, &mouseState]() {
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

  void DrawEditor() {

    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    RECT dragRect;
    ZeroMemory(&dragRect,sizeof(dragRect));
    dragRect.bottom = 19;

    if (ImGui::BeginMainMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        ImGui::MenuItem("New");
        ImGui::Separator();
        ImGui::MenuItem("Open");
        if (ImGui::MenuItem("Save")) {
          SaveLevelToFile(currentLevelName);
        }
        if(ImGui::MenuItem("Save As..")) {
          SaveLevelAs();
        }
        ImGui::Separator();
        ImGui::MenuItem("Save Templates");
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) // It would be nice if this was a "X" like in the windows title bar set off to the far right
        {
          appDone = true;
        }
        ImGui::EndMenu();
      
      }

      auto cursorPos = ImGui::GetCursorScreenPos();

      dragRect.left = static_cast<LONG>(cursorPos.x);

      ImGui::SetCursorPos(ImVec2(viewport->WorkSize.x * 0.5f - 3.0f, 2.0f));
      ImGui::TextColored(ImVec4{ 1.0f,1.0f,1.0f,1.0f }, "Title");

      struct WinButtonDef {
        std::string label;
        FLOAT x;
        FLOAT y = 0.0f;
        std::function<void()> onClick;
      };

      WinButtonDef windowsButtons[] = {
        { .label = "X", .x = viewport->WorkSize.x - 1.0f * 19.0f, .onClick = []() {appDone = true; }},
        { .label = "O", .x = viewport->WorkSize.x - 2.0f * 19.0f, .onClick = []() {
          HWND desktopHwnd = GetDesktopWindow();
          RECT desktopRect;
          GetClientRect(desktopHwnd, &desktopRect);
          int winWidth = desktopRect.right;
          int winHeight = desktopRect.bottom - 40;
          SetWindowPos(hWnd, HWND_TOP, 0, 0, winWidth, winHeight, 0);
        }},
        { .label = "_", .x = viewport->WorkSize.x - 3.0f * 19.0f, .onClick = []() { ShowWindow(hWnd, SW_MINIMIZE); }},
      };

      dragRect.right = static_cast<LONG>(windowsButtons[_countof(windowsButtons) - 1].x);

      for (auto button : windowsButtons) {
        ImGui::SetCursorPos(ImVec2(button.x,button.y));
        if (ImGui::Button(button.label.c_str(), ImVec2(19.0f, 19.0f))) { button.onClick(); }
      }

      ImGui::EndMainMenuBar();
    }
    HandleApplicationDragTitleBar(dragRect);
  

    /*
    UINT panelWidth = 300;
    UINT panelHeight = hWndRect.bottom - hWndRect.top;
    UINT panelPosX = hWndRect.right - panelWidth;
    UINT panelPosY = 0;
    ImGui::SetNextWindowPos(ImVec2(panelPosX, panelPosY));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

    ImGui::Begin("Demo light",(bool*)1,
      ImGuiWindowFlags_NoTitleBar |
      ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove
    );
    {
      LightPtr light = GetLight(L"light1(dir)");
      ImGui::ColorEdit3("color", &light->directional.color.x);
    }
    ImGui::End();
    */

    // Rendering
    ImGui::Render();

    // Render Dear ImGui graphics
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), renderer->commandList);
  }

  void SaveLevelToFile(std::wstring levelFileName) {
    using namespace nlohmann;

    json level;

    level["renderables"] = json::array();
    level["lights"] = json::array();
    level["cameras"] = json::array();

    using namespace Scene::Renderable;
    for (auto& [name,renderable] : GetRenderables()) {
      level["renderables"].push_back(renderable->json());
    }

    using namespace Scene::Lights;
    for (auto& light : GetLights()) {
      level["lights"].push_back(light->json());
    }

    using namespace Scene::Camera;
    for (auto& camera : GetCameras()) {
      if (camera->light != nullptr) continue;
      level["cameras"].push_back(camera->json());
    }

    std::string levelString =  level.dump(4);

    const std::wstring levelsRootFolder = L"Levels/";
    const std::wstring filename = levelsRootFolder + levelFileName;

    //first create the directory if needed
    std::filesystem::path directory(levelsRootFolder);
    std::filesystem::create_directory(directory);

    //then create the json level file
    std::filesystem::path path(filename);
    std::string pathStr = path.generic_string();
    std::ofstream file;
    file.open(pathStr);
    file.write(levelString.c_str(), levelString.size());
    file.close();
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

  void SaveLevelAs() {

    std::thread saveAs([]() {
      const std::wstring levelsRootFolder = L"Levels/";
      //first create the directory if needed
      std::filesystem::path directory(levelsRootFolder);
      std::filesystem::create_directory(directory);

      std::wstring path = L"";
      COMDLG_FILTERSPEC filters[] = { {.pszName = L"JSON files. (*.json)", .pszSpec = L"*.json" } };
      std::pair<COMDLG_FILTERSPEC*, int> filter_info = std::make_pair<COMDLG_FILTERSPEC*, int>(filters, _countof(filters));
      if (!SaveFileDialog(path, std::filesystem::absolute(directory), L"", &filter_info)) return;
      if (path.empty()) return;

      std::filesystem::path jsonFilePath = path;
      jsonFilePath.replace_extension(".json");

      SaveLevelToFile(jsonFilePath.filename());
    });
    saveAs.detach();
  }
};

#endif