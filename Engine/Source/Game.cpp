#include "pch.h"
#include "Game.h"
#include "Renderer/Renderer.h"
//#include <Scene.h>
//#include <Camera/Camera.h>
//#include <Sound/SoundFX.h>
//#include <Light/Light.h>
//#include <DeviceUtils/DescriptorHeap/DescriptorHeap.h>
//#include <RenderPass/SwapChainPass.h>
//#include <RenderPass/RenderToTexturePass.h>

//#if defined(_EDITOR)
//#include <Editor.h>
//using namespace Editor;
//#endif
//#include <DeviceUtils/Resources/Resources.h>
//#include <DirectXHelper.h>
#include <StepTimer.h>
#include "Engine.h"
//
//#include <RenderPass/RenderPass.h>
//#include <Renderable/Renderable.h>
//#include <Level.h>
//#include <VenomController.h>
//#include <GameStateMachine.h>
//#include <SpinYawController.h>

using namespace Scene;
using namespace DeviceUtils;
using namespace ComputeShader;

extern std::unique_ptr<DirectX::Mouse> mouse;
extern DX::StepTimer timer;

GameStates gameState = GameStates::GS_None;
std::string gameAppTitle = "Culpeo Test Game";

extern std::unique_ptr<Renderer> renderer;
JUUID levelCameraUUID;
JUUID editorCameraUUID;
std::string editorPrePlayDump;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	return EngineWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

GameStatesMachine<GameStates> gsm =
{
	.currentState = GS_None,
	.onEnter = {
		{ GS_Booting, BootScreenCreate },
		{ GS_Loading, LoadingScreenCreate },
		{ GS_Playing, PlayModeCreate },
#if defined(_EDITOR)
		{ GS_Editor, EditorModeCreate },
		{ GS_EditorPlaying, EditorPlayingModeCreate }
#endif
	},
	.onLeave = {
		{ GS_Booting, BootScreenLeave },
		{ GS_Loading, LoadingScreenLeave },
		{ GS_Playing, PlayModeLeave },
#if defined(_EDITOR)
		{ GS_Editor, EditorModeLeave },
		{ GS_EditorPlaying, EditorPlayingModeLeave }
#endif
	},
	.onStep = {
#if !defined(_EDITOR)
		{ GS_None, []() {gsm.ChangeState(GS_Booting); }},
#else
		{ GS_None, []() {gsm.ChangeState(GS_Editor); }},
#endif
		{ GS_Booting, BootScreenStep },
		{ GS_Loading, LoadingScreenStep },
		{ GS_Playing, PlayModeStep },
#if defined(_EDITOR)
		{ GS_Editor, EditorModeStep },
		{ GS_EditorPlaying, EditorPlayingModeStep }
#endif
	},
	.onRender = {
		{ GS_Booting, BootScreenRender },
		{ GS_Loading, LoadingScreenRender },
		{ GS_Playing, PlayModeRender },
#if defined(_EDITOR)
		{ GS_Editor, EditorModeRender },
		{ GS_EditorPlaying, EditorPlayingModeRender },
#endif
	},
	.onPostRender = {
#if defined(_EDITOR)
		{ GS_Editor, EditorModePostRender },
		{ GS_EditorPlaying, EditorPlayingModePostRender },
#endif
	}
};

void RunRender()
{
	gsm.Render();
}

void PostRender()
{
	gsm.PostRender();
}

void GameStep()
{
	gsm.Step();
}

void GameDestroy()
{
	gsm.ChangeState(GS_Destroy);
}

void GetAudioListenerVectors(std::function<void(XMFLOAT3, XMVECTOR)> audioListenerCallback)
{
}

void WindowResizeReleaseResources()
{
	/*
#if defined(_EDITOR)
	Editor::ReleasePickingPassResources();
#endif
	if (mainPass) mainPass->ReleaseResources();
	if (resolvePass) resolvePass->ReleaseResources();
	*/
}

void WindowResize(unsigned int width, unsigned int height)
{
	/*
#if defined(_EDITOR)
	Editor::ResizePickingPass(width, height);
#endif
	if (mainPass) mainPass->Resize(width, height);
	if (resolvePass) resolvePass->Resize(width, height);

	std::shared_ptr<MaterialInstance>& toneMapMaterial = toneMapQuad->meshMaterials.begin()->second;
	toneMapMaterial->textures.insert_or_assign(TextureShaderUsage_Base, GetTextureFromGPUHandle("toneMap", mainPass->renderToTexture[0]->gpuTextureHandle));
	*/
}

void RunPreRenderComputeShaders()
{
	//RunBoundingBoxComputeShaders();
}

void RunPostRenderComputeShaders()
{
	//RunBoundingBoxComputeShadersSolution();
}

//Booting
//float bootScreenAlpha = 0.0f;
//float loadingProgress = 0.0f;
//std::shared_ptr<tween> bootAlphaTween;
//std::shared_ptr<tween> loadingProgressTween;
//std::shared_ptr<Renderable> bootScreen;
//std::shared_ptr<Renderable> loadingBar;

void BootScreenCreate(GameStates prevState)
{
	//renderer->RenderCriticalFrame([]
	//	{
	//		using namespace Scene::Level;
	//
	//		LoadLevel("bootscreen");
	//		BindSceneObjects();
	//	}
	//);
	//
	//bootScreen = GetFromRenderablesByName("logo");
	//loadingBar = GetFromRenderablesByName("loadingBar");
	//bootAlphaTween = std::make_shared<tween>(tween(0.0f, 1.0f, 1000, tween::easing::linear));
}

void BootScreenLeave(GameStates nextState)
{
	//bootScreen = nullptr;
	//bootAlphaTween = nullptr;
}

void BootScreenStep()
{
	//bootScreenAlpha = bootAlphaTween->step();
	//
	//if (bootScreenAlpha == 1.0f) {
	//	gsm.ChangeState(GS_Loading);
	//}
}

void BootScreenRender()
{
	//using namespace Scene;
	//if (GetNumSwapChainCameras() > 0ULL)
	//{
	//	bootScreen->WriteConstantsBuffer("alpha", bootScreenAlpha, renderer->backBufferIndex);
	//
	//	//hide the loading bar
	//	XMFLOAT2 scale(0.0f, 0.0f);
	//	loadingBar->WriteConstantsBuffer<XMFLOAT2>("scale", scale, renderer->backBufferIndex);
	//
	//	WriteConstantsBuffers();
	//	RenderSceneShadowMaps();
	//	RenderSceneCameras();
	//}
}

//Loading
void LoadingScreenCreate(GameStates prevState)
{
	//loadingProgressTween = std::make_shared<tween>(tween(0.0f, 1.0f, 4000, tween::easing::linear));
}

void LoadingScreenLeave(GameStates nextState)
{
	//loadingBar = nullptr;
	//loadingProgressTween = nullptr;
}

void LoadingScreenStep()
{
	//loadingProgress = loadingProgressTween->step();
	//if (loadingProgress == 1.0f)
	//{
	//	gsm.ChangeState(GS_Playing);
	//}
}

void LoadingScreenRender()
{
	//using namespace Scene;
	//if (GetNumSwapChainCameras() > 0ULL)
	//{
	//	XMFLOAT2 pos(0.0f, -0.8f);
	//	XMFLOAT2 scale(0.8f, 0.02f);
	//	auto red = DirectX::Colors::Red;
	//	auto blue = DirectX::Colors::Blue;
	//
	//	loadingBar->WriteConstantsBuffer<XMFLOAT2>("pos", pos, renderer->backBufferIndex);
	//	loadingBar->WriteConstantsBuffer<XMFLOAT2>("scale", scale, renderer->backBufferIndex);
	//	loadingBar->WriteConstantsBuffer<XMVECTORF32>("color1", red, renderer->backBufferIndex);
	//	loadingBar->WriteConstantsBuffer<XMVECTORF32>("color2", blue, renderer->backBufferIndex);
	//	loadingBar->WriteConstantsBuffer<float>("progress", loadingProgress, renderer->backBufferIndex);
	//	WriteConstantsBuffers();
	//	RenderSceneShadowMaps();
	//	RenderSceneCameras();
	//}
}

//Playing
void PlayModeCreate(GameStates prevState)
{
	//renderer->RenderCriticalFrame([]
	//	{
	//		using namespace Scene::Level;
	//
	//		LoadLevel("venom");
	//		BindSceneObjects();
	//	}
	//);
}

void PlayModeLeave(GameStates nextState)
{
}

void PlayModeStep()
{
	//Game::StepControllers(static_cast<float>(timer.GetElapsedSeconds() / 1000.0f));
}

void PlayModeRender()
{
	//using namespace Scene;
	//if (GetNumSwapChainCameras() > 0ULL)
	//{
	//	WriteConstantsBuffers();
	//	RenderSceneShadowMaps();
	//	RenderSceneCameras();
	//}
}

//Editor
#if defined(_EDITOR)

//EditorMode
void DestroyEditorModeBindings()
{
	DestroyBillboards();
	DestroyRenderableBoundingBox();
	DestroyPickingPass();
}

void CreateEditorIndependentCamera()
{
	if (GetNumMouseCameras() > 0ULL)
	{
		//no more than a single swapchain camera or mouse controller is allowed
		//todo handle RTT cameras that does resolving
		std::shared_ptr<Camera> levelCamera = GetMouseCameras().at(0);
		levelCameraUUID = levelCamera->uuid();

		//this should be done and reversed later in the same function.
		//the purpose is to allow to create the editor camera
		//switching will be performed by switching functions later so we undo this in a few lines below
		EraseCameraFromMouseCameras(levelCamera);
		EraseCameraFromSwapChainCameras(levelCamera);

		//the new camera uuid must be known previously so it can be assigned to the renderables first
		//this is because the camera binding must know before hand how to bind both camera the renderables 
		editorCameraUUID = getUUID();

		//make a patch for the uuid and clone the camera
		nlohmann::json parameters = {
			{ "uuid", editorCameraUUID },
			{ "name", "editorCamera" },
			{ "hidden", true },
			{ "systemCreated", true }
		};
		CloneSceneObject(levelCameraUUID, parameters);
		auto editorCamera = GetFromCameras(editorCameraUUID);

		//step out a little bit of the scene, we can came up with a better number eventually
		editorCamera->MoveForward(-10.0f);
		editorCamera->WriteConstantsBuffer(renderer->backBufferIndex);

		//restore cameras mapping
		EraseCameraFromMouseCameras(editorCamera);
		EraseCameraFromSwapChainCameras(editorCamera);
		InsertCameraIntoMouseCameras(levelCamera);
		InsertCameraIntoSwapChainCameras(levelCamera);

		Editor::RegisterBillboard(levelCameraUUID);
	}
	else
	{

	}
}

void SwitchToEditorCamera()
{
	std::shared_ptr<Camera> levelCamera = GetFromCameras(levelCameraUUID);
	std::shared_ptr<Camera> editorCamera = GetFromCameras(editorCameraUUID);

	editorCamera->renderables = levelCamera->renderables;
	EraseCameraFromMouseCameras(levelCamera);
	EraseCameraFromSwapChainCameras(levelCamera);
	InsertCameraIntoMouseCameras(editorCamera);
	InsertCameraIntoSwapChainCameras(editorCamera);
}

void SwitchToEditorPlayCamera()
{
	std::shared_ptr<Camera> levelCamera = GetFromCameras(levelCameraUUID);
	std::shared_ptr<Camera> editorCamera = GetFromCameras(editorCameraUUID);
	EraseCameraFromMouseCameras(editorCamera);
	EraseCameraFromSwapChainCameras(editorCamera);
	InsertCameraIntoMouseCameras(levelCamera);
	InsertCameraIntoSwapChainCameras(levelCamera);
}

void DestroyEditorCameras()
{
	SafeDeleteSceneObject(editorCameraUUID);
	editorCameraUUID.clear();
}

void ReloadSceneFromPrePlay()
{
	using namespace Scene::Level;

	DestroySceneObjects();

	nlohmann::json data = nlohmann::json::parse(editorPrePlayDump);

	LoadSceneObjects<Renderable>(data, SceneObjectTypeJsonContainer.at(SO_Renderables));
	LoadSceneObjects<Camera>(data, SceneObjectTypeJsonContainer.at(SO_Cameras));
	LoadSceneObjects<Light>(data, SceneObjectTypeJsonContainer.at(SO_Lights));
	LoadSceneObjects<SoundFX>(data, SceneObjectTypeJsonContainer.at(SO_SoundEffects));
}

void EditorModeCreate(GameStates prevState)
{
	if (prevState == GS_None)
	{
		renderer->RenderCriticalFrame([]
			{
				using namespace Scene::Level;

				//LoadDefaultLevel();
				//LoadLevel("bootscreen");
				LoadLevel("venom");
				BindSceneObjects();
			}
		);
	}
	else if (prevState == GS_EditorPlaying)
	{
		renderer->Flush();
		renderer->RenderCriticalFrame([]
			{
				DestroyEditorCameras();
				ReloadSceneFromPrePlay();
				BindSceneObjects();
			}
		);
	}

	CreateEditorIndependentCamera();
	SwitchToEditorCamera();
	WriteConstantsBuffers();
}

void EditorModeLeave(GameStates nextState)
{
	auto editorCamera = GetFromCameras(editorCameraUUID);
	editorCamera->renderables.clear();

	renderer->Flush();
	renderer->RenderCriticalFrame([]
		{
			DestroyEditorModeBindings();
		}
	);
}

void EditorModeStep()
{
	using namespace Scene::Level;

	//if there is a level pending to load
	if (PendingLevelToLoad())
	{
		//we must flush the rendering to in order to unlock gpu bounded resources
		renderer->Flush();
		//then we can load the scene in a new critical frame
		renderer->RenderCriticalFrame([]
			{
				DestroyEditorModeBindings();
				LoadPendingLevel();
				BindSceneObjects();
			}
		);
		CreateEditorIndependentCamera();
		SwitchToEditorCamera();
	}

	if (RenderableBoundingBoxExists())
	{
		UpdateBoundingBox();
	}

	if (GetNumMouseCameras() > 0ULL)
	{
		GameAreaMouseProcessing(mouse, GetMouseCameras().at(0));
	}

	StepAnimationSequencer();

	if (Editor::IsPlaying())
	{
		Editor::DestroyBillboard(levelCameraUUID);
		gsm.ChangeState(GS_EditorPlaying);
	}
}

void EditorModeRender()
{
	using namespace Scene;
	if (GetNumSwapChainCameras() > 0ULL)
	{
#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "EditorModeRender");
#endif

		WriteConstantsBuffers();

#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "RenderPickingPass");
#endif
		RenderPickingPass(GetSwapChainCameras().at(0));
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif

#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "RenderSceneShadowMaps");
#endif
		RenderSceneShadowMaps();
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif

#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "RenderShadowMapMinMaxChain");
#endif
		RenderShadowMapMinMaxChain();
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif

#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "RenderSceneCameras");
#endif
		RenderSceneCameras();
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif

#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "DrawEditor");
#endif
		DrawEditor(GetSwapChainCameras().at(0));
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif

#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif
	}
	else
	{
#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, L"Fallback Draw");
#endif
		renderer->swapChainPass->Pass();
		DrawEditor(nullptr);
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif
	}
}

void EditorModePostRender()
{
	bool criticalFrame = (
		!PickingPassExists() ||
		!RenderableBoundingBoxExists()) && GetNumSwapChainCameras() > 0ULL ||
		PendingBillboards() ||
		PendingBillboardsDestruction() ||
		PendingAnimationSequencer() ||
		PendingAnimationSequencerDestruction();

	if (criticalFrame)
	{
		renderer->Flush();
		renderer->RenderCriticalFrame([]
			{
				if (!PickingPassExists())
				{
					CreatePickingPass();
					if (PickingPassExists())
					{
						BindPickingRenderables();
					}
				}

				if (GetNumMouseCameras() > 0ULL && !RenderableBoundingBoxExists())
				{
					CreateRenderableBoundingBox(GetMouseCameras().at(0));
				}

				if (PendingBillboards())
					CreateRegisteredBillboards();

				if (PendingBillboardsDestruction())
					DestroyPendingBillboards();

				if (PendingAnimationSequencer())
					LoadAnimationSequencer();

				if (PendingAnimationSequencerDestruction())
					DestroyAnimationSequencer();
			}
		);
	}

	PickFromScene();
}

//EditorPlayingMode
void EditorPlayingModeCreate(GameStates prevState)
{
	using namespace Editor;
	SwitchToEditorPlayCamera();
	editorPrePlayDump = GetLevelString();
}

void EditorPlayingModeLeave(GameStates nextState)
{

}

void EditorPlayingModeStep()
{
	using namespace Scene::Level;

	if (PendingLevelToLoad())
	{
		renderer->Flush();
		renderer->RenderCriticalFrame([]
			{
				DestroyEditorModeBindings();
				LoadPendingLevel();
				BindSceneObjects();
			}
		);
	}

	if (!Editor::IsPlaying())
	{
		gsm.ChangeState(GS_Editor);
		return;
	}

	Game::StepControllers(static_cast<float>(timer.GetElapsedSeconds() / 1000.0f));
}

void EditorPlayingModeRender()
{
	using namespace Scene;
	if (GetNumSwapChainCameras() > 0ULL)
	{
#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "EditorPlayingModeRender");
#endif
		WriteConstantsBuffers();


#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "RenderSceneShadowMaps");
#endif
		RenderSceneShadowMaps();
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif

#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "RenderSceneCameras");
#endif
		RenderSceneCameras();
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif

#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, "DrawEditor");
#endif
		DrawEditor(GetSwapChainCameras().at(0));
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif

#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif
	}
	else
	{
#if defined(_DEVELOPMENT)
		PIXBeginEvent(renderer->commandList.p, 0, L"Fallback Draw");
#endif
		renderer->swapChainPass->Pass();
		DrawEditor(nullptr);
#if defined(_DEVELOPMENT)
		PIXEndEvent(renderer->commandList.p);
#endif
	}
}

void EditorPlayingModePostRender()
{

}

#endif

namespace Game
{
	std::unordered_map<std::string, std::function<std::unique_ptr<Game::Controller>()>> controllers =
	{
		//{ "venom", []() { return std::make_unique<Game::VenomController>(); }},
		//{ "spinyaw", []() { return std::make_unique<Game::SpinYawController>(); }},
	};

	std::vector<std::string> GetGameControllers()
	{
		return nostd::GetKeysFromMap(controllers);
	}

	std::unique_ptr<Game::Controller> GetGameController(std::string name)
	{
		return nullptr;
		//return (controllers.contains(name)) ? controllers.at(name)() : nullptr;
	}
};