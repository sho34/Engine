#pragma once
#include <Scene.h>
#include <wrl.h>
//#include <Camera/Camera.h>
//#include <Renderable/Renderable.h>
//#include <Sound/SoundFX.h>
//#include <Light/Light.h>
#include <JObject.h>
#include <Templates.h>

enum SceneObjectType;
enum TemplateType;

namespace Scene
{
	struct SceneObject;
	struct SoundFX;
	struct Camera;
	struct Renderable;
	struct Light;
};

namespace DirectX
{
	class Mouse;
	struct XMFLOAT4X4;
};

using namespace Scene;

template<typename T>
struct CreatorModal {
	bool creating = false;
	T type;
	nlohmann::json json;
	nlohmann::json modalProperties;
	std::vector<std::string> atts;
	std::map<std::string, JEdvCreatorDrawerFunction> drawers;
	std::map<std::string, JEdvCreatorValidatorFunction> validators;
	std::function<void(T type, nlohmann::json)> onCreate;

	void DrawCreationPopup(const char* title)
	{
		if (!creating) return;

		ImGui::OpenPopup(title);
		if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			for (auto& att : atts)
			{
				if (drawers.at(att)) drawers.at(att)(att, json, modalProperties);
			}
			bool valid = true;
			for (auto& att : atts)
			{
				if (validators.at(att)) valid &= validators.at(att)(att, json);
			}
			ImGui::DrawItemWithEnabledState([this]
				{
					if (ImGui::Button("Create"))
					{
						onCreate(type, json);
						creating = false;
					}
				}, valid);
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				creating = false;
			}
			ImGui::EndPopup();
		}
	}
};

struct DeletePrompt
{
	bool showing = false;
	std::vector<nlohmann::json> references;
	std::function<void(std::vector<nlohmann::json> references)> OnDelete;
	std::function<void()> OnCancel;

	void DrawPrompt(const char* prompTitle)
	{
		const ImGuiTableFlags column_flags = ImGuiTableColumnFlags_None;
		bool deleteEnabled = false;
		bool allToDelete = true;
		std::string title = "delete-template";
		ImGui::OpenPopup(prompTitle);

		auto setAllToDelete = [this](bool value)
			{
				for (unsigned int i = 0; i < references.size(); i++)
				{
					auto& nav = references.at(i);
					std::string type = nav.at("type");
					if (type != "currentlevel")
					{
						nav.at("delete") = value;
					}
				}
			};

		if (ImGui::BeginPopupModal(prompTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			std::string tableName = "delete-template-table";
			if (ImGui::BeginTable(tableName.c_str(), 5, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersV))
			{
				ImGui::TableSetupColumn("delete", column_flags);
				ImGui::TableSetupColumn("file", column_flags);
				ImGui::TableSetupColumn("type", column_flags);
				ImGui::TableSetupColumn("name", column_flags);
				ImGui::TableSetupColumn("path", column_flags);

				ImGui::TableHeadersRow();

				for (unsigned int i = 0; i < references.size(); i++)
				{
					ImGui::TableNextRow();
					auto& nav = references.at(i);
					std::string type = nav.at("type");

					ImGui::TableSetColumnIndex(0);
					bool value = nav.at("delete");
					deleteEnabled |= value;
					allToDelete &= value;

					ImGui::PushID(std::string(std::string("delete-") + std::to_string(i)).c_str());
					if (ImGui::Checkbox("##", &value))
					{
						if (type != "currentlevel")
						{
							nav.at("delete") = value;
						}
					}
					ImGui::PopID();

					if (type == "template")
					{
						ImGui::TableSetColumnIndex(2);
						std::string templ = nav.at("template");
						ImGui::Text(templ.c_str());
					}
					else if (type == "sceneobject")
					{
						ImGui::TableSetColumnIndex(1);
						std::string file = nav.at("filename");
						ImGui::Text(file.c_str());

						ImGui::TableSetColumnIndex(2);
						std::string sceneObject = nav.at("sceneObject");
						ImGui::Text(JsonContainerToString.at(sceneObject).c_str());
					}
					else
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("scene");

						ImGui::TableSetColumnIndex(2);
						std::string sceneObject = nav.at("sceneObject");
						ImGui::Text(JsonContainerToString.at(sceneObject).c_str());
					}

					std::string name = nav.at("name");
					std::string path = nav.at("path");

					ImGui::TableSetColumnIndex(3);
					ImGui::Text(name.c_str());

					ImGui::TableSetColumnIndex(4);
					ImGui::Text(path.c_str());
				}

				ImGui::EndTable();
			}

			if (ImGui::Checkbox(allToDelete ? "Deselect all" : "Select all", &allToDelete))
			{
				setAllToDelete(allToDelete);
			}

			ImGui::DrawItemWithEnabledState([this]
				{
					if (ImGui::Button("Delete"))
					{
						OnDelete(references);
					}
				}, deleteEnabled);

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				OnCancel();
			}
			ImGui::EndPopup();
		}
	}
};

namespace Editor {

	static const LONG ApplicationBarBottom = 19L;
	static const LONG RightPanelWidth = 400L;

	//Editor LifeCycle
	void InitEditor();
	void ImGuiImplRenderInit();
	void SetupImGuiStyle();
	void DestroyEditor();
	bool WndProcHandlerEditor(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Editor Drawing
	void DrawEditor(std::shared_ptr<Camera> camera);
	void DrawApplicationBar();
	void HandleApplicationDragTitleBar(RECT& dragRect);
	void OpenLevelFile();
	void SaveLevelAs();
	bool SaveFileDialog(std::wstring& path, std::wstring defaultDirectory = L"", std::wstring defaultFileName = L"", std::pair<COMDLG_FILTERSPEC*, int>* pFilterInfo = nullptr);
	void SaveLevelToFile(std::string levelFileName);
	void SaveTemplates();
	void DrawRightPanel();
	void PromptTemplateDeletion(std::vector<nlohmann::json> references, std::function<void(std::vector<nlohmann::json>)> OnDelete, std::function<void()> OnCancel);
	void DrawDeletePrompt();
	void CloseDeletionPrompt();

	//SceneObjects Panel
	void OnChangeSceneObjectTab(std::string newTab);
	void OpenSceneObject(std::string uuid);
	void OpenSceneObjectOnNextFrame(std::string uuid);
	void MarkScenePanelAssetsAsDirty();
	void DestroyEditorSceneObjectsReferences();

	//Templates Panel
	void OnChangeTemplateTab(std::string newTab);
	void OpenTemplate(std::string uuid);
	void OpenTemplateOnNextFrame(std::string uuid);
	void MarkTemplatesPanelAssetsAsDirty();

	//JObject's Preview Panel
	void SendEditorPreview(std::string uuid, auto GetJObject, auto drawers);
	void SendEditorDestroyPreview(std::string uuid, auto GetJObject);

	//Gizmos
	void DrawPickedObjectsGizmo(std::shared_ptr<Camera> camera);
	void BeginGizmoInteraction(std::shared_ptr<Camera> camera, std::function<void(DirectX::XMFLOAT4X4, DirectX::XMFLOAT4X4)> interaction = [](DirectX::XMFLOAT4X4, DirectX::XMFLOAT4X4) {});
	void DrawRenderableGizmo(std::shared_ptr<Camera> camera);
	void DrawPickedLightGizmo(std::shared_ptr<Camera> camera);
	void DrawCameraGizmo(std::shared_ptr<Camera> camera);
	void DrawSoundEffectGizmo(std::shared_ptr<Camera> camera);

	//SceneObject Selection
	void SelectSceneObject(std::string uuid);
	void SelectRenderable(std::shared_ptr<Renderable> renderable);
	void SelectLight(std::shared_ptr<Light> light);
	void SelectCamera(std::shared_ptr<Camera> camera);
	void SelectSoundEffect(std::shared_ptr<SoundFX> soundEffect);
	void ToggleSceneObjectFromSelection(std::shared_ptr<SceneObject> sceneObject);
	void SetSceneObjectSelection(std::string uuid, bool selected);
	void InsertSceneObjectToSelection(std::shared_ptr<SceneObject> sceneObject);
	void EraseSceneObjectFromSelection(std::shared_ptr<SceneObject> sceneObject);
	void ClearSceneObjectsSelection();

	//BoundingBox
	bool RenderableBoundingBoxExists();
	void CreateRenderableBoundingBox(std::shared_ptr<Camera> camera);
	void DestroyRenderableBoundingBox();
	void UpdateBoundingBox();

	//Mouse Processing
	bool MouseIsInGameArea(std::unique_ptr<DirectX::Mouse>& mouse);
	void GameAreaMouseProcessing(std::unique_ptr<DirectX::Mouse>& mouse, std::shared_ptr<Camera> camera);

	//SceneObject Picking
	bool PickingPassExists();
	void CreatePickingPass();
	void DestroyPickingPass();
	void BindPickingRenderables();
	void BindRenderableToPickingPass(std::shared_ptr<Renderable> r);
	void UnbindPickingRenderables();
	void UnbindRenderableFromPickingPass(std::shared_ptr<Renderable> r);
	void RenderPickingPass(std::shared_ptr<Camera> camera);
	void PickFromScene();
	void PickSceneObject(unsigned int pickedObjectId);
	void ReleasePickingPassResources();
	void ResizePickingPass(unsigned int width, unsigned int height);

	//JObjects Creation
	void StartSceneObjectCreation(SceneObjectType type);
	void StartTemplateCreation(TemplateType type);

	//Billboards
	std::shared_ptr<Renderable> CreateBillboardFromMaterials(std::string name, std::string material, std::string pickingMaterial);
	void RegisterBillboard(std::shared_ptr<SceneObject> sceneObject);
	std::shared_ptr<Renderable> GetBillboard(std::shared_ptr<SceneObject> sceneObject);
	void DestroyBillboard(std::shared_ptr<SceneObject> sceneObject);
	void CreateRegisteredBillboards();
	bool PendingBillboards();
	void DestroyBillboards();
}
