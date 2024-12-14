#pragma once

#if defined(_EDITOR)

namespace Editor {

	static const std::wstring defaultLevelsFolder = L"Levels/";
	static const std::wstring defaultTemplatesFolder = L"Templates/";
	static const std::wstring defaultLevelName = L"baseLevel.json";

	void InitEditor();
	void DestroyEditor();
	bool WndProcHandlerEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DrawEditor();
	void HandleApplicationDragTitleBar(RECT& dragRect);
	void SaveLevelToFile(std::wstring levelFileName);
	void SaveLevelAs();
}
#endif