#pragma once
#include <Scene.h>
#include <wrl.h>
#include <JObject.h>
#include <Templates.h>
#include <DirectXMath.h>

#define _EDITOR_BOUNDINGBOX
#define _EDITOR_PICKINGPASS
#define _EDITOR_BILLBOARD

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
	void DrawEditor(CameraUUID camera = JUUID(""));
	void DrawApplicationBar();
	void HandleApplicationDragTitleBar(RECT& dragRect);
	RECT GetGameControllerRect();
	void DrawGameController();
	void OpenLevelFile();
	void SaveLevelAs();
	bool SaveFileDialog(std::wstring& path, std::wstring defaultDirectory = L"", std::wstring defaultFileName = L"", std::pair<COMDLG_FILTERSPEC*, int>* pFilterInfo = nullptr);
	std::string GetLevelString();
	void SaveLevelToFile(std::string levelFileName);
	void SaveTemplates();
	void DrawRightPanel();
	void PromptTemplateDeletion(std::vector<nlohmann::json> references, std::function<void(std::vector<nlohmann::json>)> OnDelete, std::function<void()> OnCancel);
	void CloseDeletionPrompt();
	void BuildAssetsTree();

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
	void SendEditorPreview(JUUID uuid, auto GetJObject, auto drawers);
	void SendEditorDestroyPreview(JUUID uuid, auto GetJObject);

	//Model3D Animation Sequencer
	void OpenAnimationSequencer(std::string uuid);
	bool PendingAnimationSequencer();
	bool PendingAnimationSequencerDestruction();
	void LoadAnimationSequencer();
	void StepAnimationSequencer();
	void DestroyAnimationSequencer();

	//Gizmos
	void ResetGizmoVariableWorkers();
	bool InteractWithGizmos(std::set<SceneObject*>& objects2Gizmo);
	void DrawPickedObjectsGizmo(CameraUUID camera);
	void BeginGizmoInteraction(CameraUUID camera, std::function<void(DirectX::XMFLOAT4X4, DirectX::XMFLOAT4X4)> interaction = [](DirectX::XMFLOAT4X4, DirectX::XMFLOAT4X4) {});

	//SceneObject Selection
	void SelectSceneObject(JUUID uuid);
	void SelectRenderable(JUUID ruuid);
	void SelectLight(JUUID luuid);
	void SelectCamera(JUUID cuuid);
	void SelectSoundEffect(JUUID suuid);
	void ToggleSceneObjectFromSelection(JUUID uuid);
	void SetSceneObjectSelection(JUUID uuid, bool selected);
	void InsertSceneObjectToSelection(JUUID uuid);
	void EraseSceneObjectFromSelection(JUUID uuid);
	void ClearSceneObjectsSelection();

	//BoundingBox
	bool RenderableBoundingBoxExists();
	void CreateRenderableBoundingBox(CameraUUID camera);
	void DestroyRenderableBoundingBox();
	void UpdateBoundingBox();

	//Mouse Processing
	bool MouseIsInGameArea(std::unique_ptr<DirectX::Mouse>& mouse);
	void GameAreaMouseProcessing(std::unique_ptr<DirectX::Mouse>& mouse, CameraUUID camera);

	//SceneObject Picking
	bool PickingPassExists();
	void CreatePickingPass();
	void DestroyPickingPass();
	void BindPickingRenderables();
	void BindRenderableToPickingPass(RenderableUUID r);
	void UnbindPickingRenderables();
	void UnbindRenderableFromPickingPass(RenderableUUID r);
	void RenderPickingPass(CameraUUID camera);
	void PickFromScene();
	void PickSceneObject(unsigned int pickedObjectId);
	void ReleasePickingPassResources();
	void ResizePickingPass(unsigned int width, unsigned int height);

	//JObjects Creation
	void StartSceneObjectCreation(SceneObjectType type);
	void StartTemplateCreation(TemplateType type);

	//Billboards
	JUUID CreateBillboardFromMaterials(CameraUUID camera, std::string name, std::string material, std::string pickingMaterial);
	void RegisterBillboard(JUUID sceneObject);
	JUUID GetBillboard(JUUID sceneObject);
	void DestroyBillboard(JUUID sceneObject);
	void CreateRegisteredBillboards(CameraUUID camera);
	bool PendingBillboards();
	bool PendingBillboardsDestruction();
	void UpdateBillboards();
	void DestroyPendingBillboards();
	void DestroyBillboards();

	//Game Mode Activation
	bool IsPlaying();
	bool IsPaused();
	void SwitchToPlayMode();
	void SwitchToPauseMode();
	void SwitchToUnPausedMode();
	void SwitchToNonPlayMode();
}
