#pragma once

#include "../Templates/Templates.h"
#include "../Scene/Scene.h"

#if defined(_EDITOR)

namespace Editor {

	static const LONG ApplicationBarBottom = 19L;
	static const LONG RightPanelWidth = 400L;

	using namespace Scene;

	void InitEditor();

	void ImGuiImplRenderInit();
	void SetupImGuiStyle();
	void DestroyEditor();
	bool WndProcHandlerEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void HandleApplicationDragTitleBar(RECT& dragRect);

	void DrawEditor(std::shared_ptr<Camera> camera = nullptr);
	void DrawApplicationBar();
	void DrawRightPanel();
	void DrawSelectedObjectGuizmo(std::shared_ptr<Camera> camera);

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

	void CreatePickingPass();
	void DestroyPickingPass();
	void MapPickingRenderables();
	void PickingStep(std::unique_ptr<DirectX::Mouse>& mouse);
	void RenderPickingPass(std::shared_ptr<Camera> camera);
	void PickFromScene();
	void PickSceneObject(unsigned int pickedObjectId);
	bool MouseIsInGameArea(std::unique_ptr<DirectX::Mouse>& mouse);
	void ReleasePickingPassResources();
	void ResizePickingPass(unsigned int width, unsigned int height);

}
#endif