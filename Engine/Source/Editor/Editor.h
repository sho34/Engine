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

	void OpenLevelFile();

	void SaveLevelToFile(std::string levelFileName);
	void SaveLevelAs();
	void SaveTemplates();

	void SelectSceneObject(_SceneObjects objectType, std::string uuid);

	void RenderSelectedLightShadowMapChain();

	void CreateRenderableBoundingBox();
	void WriteRenderableBoundingBoxConstantsBuffer();
	void DrawOkPopup(unsigned int& flag, unsigned int cmpFlag, std::string popupId, std::function<void()> drawContent);
	void DrawCreateWindow(unsigned int& flag, unsigned int cmpFlag, std::string popupId, std::function<void(std::function<void()>)> drawContent);
	void ImDrawMaterialShaderSelection(nlohmann::json& mat, std::string key, ShaderType type, std::function<void()> cb = [] {});

}
#endif