#pragma once

#if defined(_EDITOR)

namespace Editor {

	enum RightPanelSceneTabOptions {
		Renderables,
		Lights,
		Cameras
	};

	static std::wstring RightPanelSceneTabOptionsStr[] = {
		L"Renderables",
		L"Lights",
		L"Cameras"
	};

	enum RightPanelTemplatesTabOptions {
		Material,
		Model3D,
		Shader,
		Sound
	};

	static std::wstring RightPanelTemplatesTabOptionsStr[] = {
		L"Material",
		L"Model3D",
		L"Shader",
		L"Sound"
	};

	void InitEditor();
	void DestroyEditor();
	bool WndProcHandlerEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void HandleApplicationDragTitleBar(RECT& dragRect);
	void DrawEditor();
	void DrawApplicationBar();

	void DrawRightPanelSceneObjectsTab(ImVec2 pos, ImVec2 size);
	void DrawRightPanelTemplatesTab(ImVec2 pos, ImVec2 size);
	void DrawRightPanel();

	void OpenFile();
	void SaveLevelToFile(std::wstring levelFileName);
	void SaveLevelAs();
	void SaveTemplates();
}
#endif