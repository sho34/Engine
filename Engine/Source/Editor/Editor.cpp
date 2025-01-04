#include "pch.h"

#if defined(_EDITOR)
#include "Editor.h"
#include "../Renderer/Renderer.h"
#include "../Templates/TemplatesImpl.h"
#include "../Scene/SceneImpl.h"

extern bool appDone;
extern HWND hWnd;
extern std::unique_ptr<DirectX::Mouse> mouse;
extern std::shared_ptr<Renderer> renderer;

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Editor {

  std::string currentLevelName = defaultLevelName;

  bool dragDrop = false;
  INT lastMouseX;
  INT lastMouseY;

  void InitEditor()
  {
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
      
      CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_xhandle;
      CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_xhandle;

      AllocCSUDescriptor(cpu_xhandle, gpu_xhandle);

      out_cpu_handle->ptr = cpu_xhandle.ptr;
      out_gpu_handle->ptr = gpu_xhandle.ptr;
    };
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
      CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_xhandle(cpu_handle);
      CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_xhandle(gpu_handle);
      FreeCSUDescriptor(cpu_xhandle, gpu_xhandle);
      cpu_handle.ptr = 0;
      gpu_handle.ptr = 0;
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

    DrawApplicationBar();
    DrawRightPanel();
  

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

  void DrawApplicationBar()
  {
    RECT dragRect;
    ZeroMemory(&dragRect, sizeof(dragRect));
    dragRect.bottom = 19;
    
    if (ImGui::BeginMainMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        ImGui::MenuItem(ICON_FA_FILE "New");
        ImGui::Separator();
        if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN "Open")) {
          OpenFile();
        }
        if (ImGui::MenuItem(ICON_FA_SAVE "Save")) {
          SaveLevelToFile(currentLevelName);
        }
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
          appDone = true;
        }
        ImGui::EndMenu();

      }

      auto cursorPos = ImGui::GetCursorScreenPos();
      dragRect.left = static_cast<LONG>(cursorPos.x);

      const ImGuiViewport* viewport = ImGui::GetMainViewport();

      ImGui::SetCursorPos(ImVec2(viewport->WorkSize.x * 0.5f - 3.0f, 2.0f));
      ImGui::TextColored(ImVec4{ 1.0f,1.0f,1.0f,1.0f }, "Title");

      struct WinButtonDef {
        std::string label;
        FLOAT x;
        FLOAT y = 0.0f;
        std::function<void()> onClick;
      };

      WinButtonDef windowsButtons[] = {
        { .label = ICON_FA_TIMES, .x = viewport->WorkSize.x - 1.0f * 19.0f, .onClick = []() {appDone = true; } },
        {.label = ICON_FA_WINDOW_MAXIMIZE, .x = viewport->WorkSize.x - 2.0f * 19.0f, .onClick = []() {
          HWND desktopHwnd = GetDesktopWindow();
          RECT desktopRect;
          GetClientRect(desktopHwnd, &desktopRect);
          int winWidth = desktopRect.right;
          int winHeight = desktopRect.bottom - 40;
          SetWindowPos(hWnd, HWND_TOP, 0, 0, winWidth, winHeight, 0);
        } },
        { .label = ICON_FA_WINDOW_MINIMIZE, .x = viewport->WorkSize.x - 3.0f * 19.0f, .onClick = []() { ShowWindow(hWnd, SW_MINIMIZE); } },
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

  void DebugWindowPosSize(std::string name) {
    ImVec2 spos = ImGui::GetCursorScreenPos();
    ImVec2 ravail = ImGui::GetContentRegionAvail();

    std::string str = name + "\npos:(" + std::to_string(spos.x) + "," + std::to_string(spos.y) + ")\nsize:(" + std::to_string(ravail.x) + "," + std::to_string(ravail.y) + ")\n\n";

    OutputDebugStringA(str.c_str());
  }

  void DebugPosSize(std::string name, ImVec2 pos, ImVec2 size) {
    std::string str = name + "\npos:(" + std::to_string(pos.x) + "," + std::to_string(pos.y) + ")\nsize:(" + std::to_string(size.x) + "," + std::to_string(size.y) + ")\n\n";

    OutputDebugStringA(str.c_str());
  }

  auto DrawTableRows(auto GetObjects, auto OnSelect) {
    int row = 0;
    for (auto name : GetObjects()) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      if (ImGui::TextLink(name.c_str())) { OnSelect(name); }
    }
  }

  auto DrawListPanel = [](const char* tableName, auto GetObjects, auto OnSelect) {
    if (ImGui::BeginTable(tableName, 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
    {
      DrawTableRows(GetObjects, OnSelect);
      ImGui::EndTable();
    }
  };

  template<typename T>
  void DrawTabPanel(ImVec2 pos, ImVec2 size, const char* name, T& tab,
    std::unordered_map<T, std::string> TabToStr,
    std::map<T, std::function<std::vector<std::string>()>> GetTabList,
    void* &selected,
    std::map<T, std::function<void(std::string, void*&)>> OnSelect
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
          if (ImGui::TabItemButton(name.c_str(), flag)) { tab = type; }
        }
        DrawListPanel((TabToStr.at(tab) + "-table").c_str(), GetTabList.at(tab),
          [tab, &selected, OnSelect](std::string name) {
            OnSelect.at(tab)(name, selected);
          });
      }
      ImGui::EndTabBar();
    }
    ImGui::EndChild();
  }

  template<typename T>
  void DrawDetailPanel(ImVec2 pos, ImVec2 size, T& tab,
    void*& selected,
    std::unordered_map<T, std::string> SelectedPrefix,
    std::map<T, std::function<std::string(void*)>> GetSelectedName,
    std::map<T, std::function<void(void*&, ImVec2, ImVec2)>> DrawSelectedPanel
    )
  {
    bool pop = false;
    std::string panelName = "panel-" + SelectedPrefix.at(tab);// + GetSelectedName.at(tab)(selected);
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::BeginChild(panelName.c_str());
    {
      if (ImGui::SmallButton("<<")) { pop = true; }
      ImGui::SameLine();
      ImGui::Text(GetSelectedName.at(tab)(selected).c_str());

      ImGui::NewLine();
      DrawSelectedPanel.at(tab)(selected, pos, size);
    }
    ImGui::EndChild();
    if (pop) { selected = nullptr; }
  }

  _SceneObjects soTab = _SceneObjects::Renderables;
  _Templates tempTab = _Templates::Materials;
  void* selSO = nullptr;
  void* selTemp = nullptr;
  float titleBH = 19.0f;
  float panW = 400.0f;
  ImGuiWindowFlags panFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

  void DrawRightPanel() {
    
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 panPos = ImVec2(viewport->Size.x - panW, titleBH);
    ImVec2 panSize = ImVec2(panW, viewport->Size.y - titleBH+10.0f);

    ImGui::SetNextWindowPos(panPos);
    ImGui::SetNextWindowSize(panSize);
    ImGui::Begin("Right panel",(bool*)1, panFlags);
    {
      ImVec2 soPos = ImVec2(panPos.x, panPos.y);
      ImVec2 tempPos = ImVec2(panPos.x, panSize.y * 0.5f + 20.0f);
      ImVec2 soSize = ImVec2(panSize.x, panSize.y * 0.5f);
      ImVec2 tempSize = ImVec2(panSize.x, panSize.y * 0.5f - 20.0f);

      if (selSO == nullptr) { 
        DrawTabPanel(soPos, soSize, "Scene Objects", soTab,
          SceneObjectsToStr, GetSceneObjects, selSO, SetSelectedSceneObject
        );
      } else {
        DrawDetailPanel(soPos, soSize, soTab, selSO,
          SceneObjectsToStr, GetSceneObjectName, DrawSceneObjectPanel
        );
      }
      
      //ImGui::SetCursorPos(ImVec2(0.0f, nextCursorPosY));
      if (selTemp == nullptr) {
        DrawTabPanel(tempPos, tempSize, "Templates", tempTab,
          TemplatesToStr, GetTemplates, selTemp, SetSelectedTemplate
        );
      } else {
        DrawDetailPanel(tempPos, tempSize, tempTab, selTemp,
          TemplatesToStr, GetTemplateName, DrawTemplatePanel
        );
      }
    }
    ImGui::End();
  }

  bool OpenFileDialog(std::wstring& path, std::wstring defaultDirectory = L"", std::wstring defaultFileName = L"", std::pair<COMDLG_FILTERSPEC*, int>* pFilterInfo = nullptr)
  {
    IFileOpenDialog* p_file_open = nullptr;
    bool are_all_operation_success = false;
    while (!are_all_operation_success)
    {
      HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&p_file_open));
      if (FAILED(hr))
        break;

      if (!pFilterInfo)
      {
        COMDLG_FILTERSPEC open_filter[1];
        open_filter[0].pszName = L"All files";
        open_filter[0].pszSpec = L"*.*";
        hr = p_file_open->SetFileTypes(1, open_filter);
        if (FAILED(hr))
          break;
        hr = p_file_open->SetFileTypeIndex(1);
        if (FAILED(hr))
          break;
      }
      else
      {
        hr = p_file_open->SetFileTypes(pFilterInfo->second, pFilterInfo->first);
        if (FAILED(hr))
          break;
        hr = p_file_open->SetFileTypeIndex(1);
        if (FAILED(hr))
          break;
      }

      if (!defaultDirectory.empty()) {
        IShellItem* pCurFolder = NULL;
        hr = SHCreateItemFromParsingName(defaultDirectory.c_str(), NULL, IID_PPV_ARGS(&pCurFolder));
        if (FAILED(hr))
          break;
        p_file_open->SetFolder(pCurFolder);
        pCurFolder->Release();
      }

      if (!defaultFileName.empty())
      {
        hr = p_file_open->SetFileName(defaultFileName.c_str());
        if (FAILED(hr))
          break;
      }

      hr = p_file_open->Show(NULL);
      if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) // No item was selected.
      {
        are_all_operation_success = true;
        break;
      }
      else if (FAILED(hr))
        break;

      IShellItem* p_item;
      hr = p_file_open->GetResult(&p_item);
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

    if (p_file_open)
      p_file_open->Release();
    return are_all_operation_success;
  }

  void OpenFile() {
    std::thread load([]() {
      //first create the directory if needed
      std::filesystem::path directory(defaultLevelsFolder);
      std::filesystem::create_directory(directory);

      std::wstring path = L"";
      COMDLG_FILTERSPEC filters[] = { {.pszName = L"JSON files. (*.json)", .pszSpec = L"*.json" } };
      std::pair<COMDLG_FILTERSPEC*, int> filter_info = std::make_pair<COMDLG_FILTERSPEC*, int>(filters, _countof(filters));
      if (!OpenFileDialog(path, std::filesystem::absolute(directory), L"", &filter_info)) return;
      if (path.empty()) return;

      std::filesystem::path jsonFilePath = path;
      jsonFilePath.replace_extension(".json");

      soTab = _SceneObjects::Renderables;
      tempTab = _Templates::Materials;
      selSO = nullptr;
      selTemp = nullptr;

      LoadLevel(jsonFilePath);
    });
    load.detach();
  }

  void SaveLevelToFile(std::string levelFileName) {
    using namespace nlohmann;

    nlohmann::json level;

    level["renderables"] = json::array();
    level["lights"] = json::array();
    level["cameras"] = json::array();
    level["sounds"] = json::array();

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

    using namespace Scene::SoundEffect;
    for (auto& [name,sfx] : GetSoundsEffects()) {
      level["sounds"].push_back(sfx->json());
    }

    std::string levelString =  level.dump(4);

    const std::string levelsRootFolder = "Levels/";
    const std::string filename = levelsRootFolder + levelFileName;

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
      //first create the directory if needed
      std::filesystem::path directory(StringToWString(defaultLevelsFolder));
      std::filesystem::create_directory(directory);

      std::wstring path = L"";
      COMDLG_FILTERSPEC filters[] = { {.pszName = L"JSON files. (*.json)", .pszSpec = L"*.json" } };
      std::pair<COMDLG_FILTERSPEC*, int> filter_info = std::make_pair<COMDLG_FILTERSPEC*, int>(filters, _countof(filters));
      if (!SaveFileDialog(path, std::filesystem::absolute(directory), L"", &filter_info)) return;
      if (path.empty()) return;

      std::filesystem::path jsonFilePath = path;
      jsonFilePath.replace_extension(".json");

      SaveLevelToFile(WStringToString(jsonFilePath.filename()));
    });
    saveAs.detach();
  }

  void SaveTemplates()
  {
    
    Templates::SaveTemplates(defaultTemplatesFolder, Templates::Shader::templateName, Templates::Shader::json());
    Templates::SaveTemplates(defaultTemplatesFolder, Templates::Material::templateName, Templates::Material::json());
    Templates::SaveTemplates(defaultTemplatesFolder, Templates::Model3D::templateName, Templates::Model3D::json());
    Templates::SaveTemplates(defaultTemplatesFolder, Templates::Sound::templateName, Templates::Sound::json());

  }

};

#endif