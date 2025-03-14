#pragma once

#include "../Templates/Templates.h"
#include "../Scene/Scene.h"

#if defined(_EDITOR)

namespace Editor {

	using namespace Scene;

	void InitEditor();

	void ImGuiImplRenderInit();
	void DestroyEditor();
	bool WndProcHandlerEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void HandleApplicationDragTitleBar(RECT& dragRect);

	void DrawEditor();
	void DrawApplicationBar();
	void DrawRightPanel();

	bool OpenFileDialog(std::wstring& path, std::wstring defaultDirectory = L"", std::wstring defaultFileName = L"", std::pair<COMDLG_FILTERSPEC*, int>* pFilterInfo = nullptr);
	void OpenFile(std::function<void(std::filesystem::path)> onFileSelected, std::string defaultDirectory, std::string filterName = "JSON files. (*.json)", std::string filterPattern = "*.json");
	void OpenLevelFile();

	void SaveLevelToFile(std::string levelFileName);
	void SaveLevelAs();
	void SaveTemplates();

	void SelectSceneObject(_SceneObjects objectType, void* obj);

	void RenderSelectedLightShadowMapChain();

	void CreateRenderableBoundingBox();
	void WriteRenderableBoundingBoxConstantsBuffer();

}
#endif