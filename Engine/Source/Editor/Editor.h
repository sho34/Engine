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
	void DrawRenderableGuizmo(std::shared_ptr<Camera> camera);
	void DrawLightGuizmo(std::shared_ptr<Camera> camera);
	void DrawCameraGuizmo(std::shared_ptr<Camera> camera);
	void DrawSoundGuizmo(std::shared_ptr<Camera> camera);
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

	//MOUSE PROCESSING
	bool MouseIsInGameArea(std::unique_ptr<DirectX::Mouse>& mouse);
	void GameAreaMouseProcessing(std::unique_ptr<DirectX::Mouse>& mouse, std::shared_ptr<Camera> camera);

	//OBJECT PICKING
	void CreatePickingPass();
	void DestroyPickingPass();
	void MapPickingRenderables();
	void RenderPickingPass(std::shared_ptr<Camera> camera);
	void PickFromScene();
	void PickSceneObject(unsigned int pickedObjectId);
	void ReleasePickingPassResources();
	void ResizePickingPass(unsigned int width, unsigned int height);

	//GIZMO INTERACTION

	//CAMERA INTERACTION

}
#endif