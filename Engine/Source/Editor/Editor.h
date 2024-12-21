#pragma once

#if defined(_EDITOR)

namespace Editor {

	void InitEditor();
	void DestroyEditor();
	bool WndProcHandlerEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void HandleApplicationDragTitleBar(RECT& dragRect);
	void DrawEditor();
	void OpenFile();
	void SaveLevelToFile(std::wstring levelFileName);
	void SaveLevelAs();
	void SaveTemplates();
}
#endif