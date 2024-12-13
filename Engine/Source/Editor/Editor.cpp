#include "pch.h"

#if defined(_EDITOR)
#include "Editor.h"
#include "../Renderer/Renderer.h"

extern bool appDone;
extern HWND hWnd;
extern std::unique_ptr<DirectX::Mouse> mouse;
extern std::shared_ptr<Renderer> renderer;

namespace Editor {

bool dragDrop = false;
INT lastMouseX;
INT lastMouseY;

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
      ImGui::MenuItem("Save");
      ImGui::MenuItem("Save As..");
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

};

#endif