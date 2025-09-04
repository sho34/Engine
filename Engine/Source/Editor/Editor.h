#pragma once
#include <Scene.h>
#include <wrl.h>
#include <Camera/Camera.h>
#include <Renderable/Renderable.h>
#include <JObject.h>
#include <Templates.h>

#if defined(_EDITOR)

using namespace Scene;

template<typename T>
struct CreatorModal {
	bool creating = false;
	T type;
	nlohmann::json json;
	std::vector<std::string> atts;
	std::map<std::string, JEdvCreatorDrawerFunction> drawers;
	std::function<void(T type, nlohmann::json)> onCreate;
	void DrawCreationPopup()
	{
		if (!creating) return;

		ImGui::OpenPopup("createPopup");
		if (ImGui::BeginPopupModal("createPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			for (auto& att : atts)
			{
				if (drawers.at(att)) drawers.at(att)(att, json);
			}
			if (ImGui::Button("Crear"))
			{
				onCreate(type, json);
				creating = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancelar"))
			{
				creating = false;
			}
			ImGui::EndPopup();
		}
	}
};

namespace Editor {

	static const LONG ApplicationBarBottom = 19L;
	static const LONG RightPanelWidth = 400L;

	void InitEditor();

	void ImGuiImplRenderInit();
	void SetupImGuiStyle();
	void DestroyEditor();
	bool WndProcHandlerEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void HandleApplicationDragTitleBar(RECT& dragRect);

	void DrawEditor(std::shared_ptr<Camera> camera);
	void DrawApplicationBar();
	void OpenSceneObject(std::string uuid);
	void OpenSceneObjectOnNextFrame(std::string uuid);
	void OnChangeSceneObjectTab(std::string newTab);
	void OpenTemplate(std::string uuid);
	void OpenTemplateOnNextFrame(std::string uuid);
	void SendEditorPreview(std::string uuid, auto GetJObject, auto drawers);
	void SendEditorDestroyPreview(std::string uuid, auto GetJObject);
	void OnChangeTemplateTab(std::string newTab);
	void DrawRightPanel();
	void MarkScenePanelAssetsAsDirty();
	void MarkTemplatesPanelAssetsAsDirty();
	void DestroyEditorSceneObjectsReferences();

	void DrawPickedObjectsGuizmo(std::shared_ptr<Camera> camera, ImGuizmo::OPERATION& gizmoOperation, ImGuizmo::MODE& gizmoMode);

	void OpenLevelFile();

	void SaveLevelToFile(std::string levelFileName);
	void SaveLevelAs();
	void SaveTemplates();

	void SelectSceneObject(std::string uuid);

	bool RenderableBoundingBoxExists();
	void CreateRenderableBoundingBox(std::shared_ptr<Camera> camera);
	void DestroyRenderableBoundingBox();
	void WriteRenderableBoundingBoxConstantsBuffer();
	//void DrawOkPopup(unsigned int& flag, unsigned int cmpFlag, std::string popupId, std::function<void()> drawContent);
	//void DrawCreateWindow(unsigned int& flag, unsigned int cmpFlag, std::string popupId, std::function<void(std::function<void()>)> drawContent);

	//MOUSE PROCESSING
	bool MouseIsInGameArea(std::unique_ptr<DirectX::Mouse>& mouse);
	void GameAreaMouseProcessing(std::unique_ptr<DirectX::Mouse>& mouse, std::shared_ptr<Camera> camera);

	//OBJECT PICKING
	bool PickingPassExists();
	void CreatePickingPass();
	void DestroyPickingPass();
	void BindPickingRenderables();
	void BindRenderableToPickingPass(std::shared_ptr<Renderable> r);
	void RenderPickingPass(std::shared_ptr<Camera> cam);
	void PickFromScene();
	void PickSceneObject(unsigned int pickedObjectId);
	void ReleasePickingPassResources();
	void ResizePickingPass(unsigned int width, unsigned int height);

	//Creation
	void StartSceneObjectCreation(SceneObjectType type);
	void StartTemplateCreation(TemplateType type);

}
#endif