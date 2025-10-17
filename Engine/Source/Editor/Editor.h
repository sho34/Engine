#pragma once
#include <Scene.h>
#include <wrl.h>
#include <JObject.h>
#include <Templates.h>
#include <DirectXMath.h>

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
	RECT GetGameControllerRect();
	void DrawGameController();
	void OpenLevelFile();
	void SaveLevelAs();
	bool SaveFileDialog(std::wstring& path, std::wstring defaultDirectory = L"", std::wstring defaultFileName = L"", std::pair<COMDLG_FILTERSPEC*, int>* pFilterInfo = nullptr);
	void SaveLevelToFile(std::string levelFileName);
	void SaveTemplates();
	void DrawRightPanel();
	void PromptTemplateDeletion(std::vector<nlohmann::json> references, std::function<void(std::vector<nlohmann::json>)> OnDelete, std::function<void()> OnCancel);
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

	//Model3D Animation Sequencer
	void OpenAnimationSequencer(std::string uuid);
	bool PendingAnimationSequencer();
	bool PendingAnimationSequencerDestruction();
	void LoadAnimationSequencer();
	void StepAnimationSequencer();
	void DestroyAnimationSequencer();

	//Gizmos
	void ResetGizmoVariableWorkers();
	bool InteractWithGizmos(std::set<std::shared_ptr<SceneObject>>& objects2Gizmo);
	void DrawPickedObjectsGizmo(std::shared_ptr<Camera> camera);
	void BeginGizmoInteraction(std::shared_ptr<Camera> camera, std::function<void(DirectX::XMFLOAT4X4, DirectX::XMFLOAT4X4)> interaction = [](DirectX::XMFLOAT4X4, DirectX::XMFLOAT4X4) {});

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
	bool PendingBillboardsDestruction();
	void DestroyPendingBillboards();
	void DestroyBillboards();
}
