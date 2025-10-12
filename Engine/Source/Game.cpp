#include "pch.h"
#include "Game.h"
#include "Renderer/Renderer.h"
#include <Scene.h>
#include <Camera/Camera.h>
#include <DeviceUtils/DescriptorHeap/DescriptorHeap.h>
#include <RenderPass/SwapChainPass.h>
#include <RenderPass/RenderToTexturePass.h>

#if defined(_EDITOR)
#include <Editor.h>
using namespace Editor;
#endif
#include <DeviceUtils/Resources/Resources.h>
#include <DirectXHelper.h>
#include <StepTimer.h>

#include <RenderPass/RenderPass.h>
#include <Renderable/Renderable.h>
#include <Level.h>
#include <VenomController.h>
#include <GameStateMachine.h>

using namespace Scene;
using namespace DeviceUtils;
using namespace ComputeShader;

extern std::unique_ptr<DirectX::Mouse> mouse;
extern DX::StepTimer timer;

GameStates gameState = GameStates::GS_None;
std::string gameAppTitle = "Culpeo Test Game";

extern std::shared_ptr<Renderer> renderer;

GameStatesMachine<GameStates> gsm =
{
	.currentState = GS_None,
	.onEnter = {
		{ GS_Booting, BootScreenCreate },
		{ GS_Loading, LoadingScreenCreate },
		{ GS_Playing, PlayModeCreate },
#if defined(_EDITOR)
		{ GS_Editor, EditorModeCreate },
#endif
	},
	.onLeave = {
		{ GS_Booting, BootScreenLeave },
		{ GS_Loading, LoadingScreenLeave },
		{ GS_Playing, PlayModeLeave },
#if defined(_EDITOR)
		{ GS_Editor, EditorModeLeave },
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
		{	GS_Editor, EditorModeStep },
#endif
	},
	.onRender = {
		{ GS_Booting, BootScreenRender },
		{ GS_Loading, LoadingScreenRender },
		{ GS_Playing, PlayModeRender },
#if defined(_EDITOR)
		{ GS_Editor, EditorModeRender },
#endif
	},
	.onPostRender = {
#if defined(_EDITOR)
		{ GS_Editor, EditorModePostRender },
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
	RunBoundingBoxComputeShaders();
}

void RunPostRenderComputeShaders()
{
	RunBoundingBoxComputeShadersSolution();
}

//Booting
float bootScreenAlpha = 0.0f;
float loadingProgress = 0.0f;
std::shared_ptr<tween> bootAlphaTween;
std::shared_ptr<tween> loadingProgressTween;
std::shared_ptr<Renderable> bootScreen;
std::shared_ptr<Renderable> loadingBar;

void BootScreenCreate()
{
	renderer->RenderCriticalFrame([]
		{
			using namespace Scene::Level;

			LoadLevel("bootscreen");
			BindSceneObjects();
		}
	);

	bootScreen = FindInRenderablesByName("logo");
	loadingBar = FindInRenderablesByName("loadingBar");
	bootAlphaTween = std::make_shared<tween>(tween(0.0f, 1.0f, 1000, tween::easing::linear));
}

void BootScreenStep()
{
	bootScreenAlpha = bootAlphaTween->step();

	if (bootScreenAlpha == 1.0f) {
		gsm.ChangeState(GS_Loading);
	}
}

void BootScreenRender()
{
	using namespace Scene;
	if (GetNumSwapChainCameras() > 0ULL)
	{
		bootScreen->WriteConstantsBuffer("alpha", bootScreenAlpha, renderer->backBufferIndex);

		//hide the loading bar
		XMFLOAT2 scale(0.0f, 0.0f);
		loadingBar->WriteConstantsBuffer<XMFLOAT2>("scale", scale, renderer->backBufferIndex);

		WriteConstantsBuffers();
		RenderSceneShadowMaps();
		RenderSceneCameras();
	}
}

void BootScreenLeave()
{
	bootScreen = nullptr;
	bootAlphaTween = nullptr;
}

//Loading
void LoadingScreenCreate()
{
	loadingProgressTween = std::make_shared<tween>(tween(0.0f, 1.0f, 4000, tween::easing::linear));
}

void LoadingScreenLeave()
{
	loadingBar = nullptr;
	loadingProgressTween = nullptr;
}

void LoadingScreenStep()
{
	loadingProgress = loadingProgressTween->step();
	if (loadingProgress == 1.0f)
	{
		gsm.ChangeState(GS_Playing);
	}
}

void LoadingScreenRender()
{
	using namespace Scene;
	if (GetNumSwapChainCameras() > 0ULL)
	{
		XMFLOAT2 pos(0.0f, -0.8f);
		XMFLOAT2 scale(0.8f, 0.02f);
		auto red = DirectX::Colors::Red;
		auto blue = DirectX::Colors::Blue;

		loadingBar->WriteConstantsBuffer<XMFLOAT2>("pos", pos, renderer->backBufferIndex);
		loadingBar->WriteConstantsBuffer<XMFLOAT2>("scale", scale, renderer->backBufferIndex);
		loadingBar->WriteConstantsBuffer<XMVECTORF32>("color1", red, renderer->backBufferIndex);
		loadingBar->WriteConstantsBuffer<XMVECTORF32>("color2", blue, renderer->backBufferIndex);
		loadingBar->WriteConstantsBuffer<float>("progress", loadingProgress, renderer->backBufferIndex);
		WriteConstantsBuffers();
		RenderSceneShadowMaps();
		RenderSceneCameras();
	}
}

//Playing
void PlayModeCreate()
{
	renderer->RenderCriticalFrame([]
		{
			using namespace Scene::Level;

			LoadLevel("venom");
			BindSceneObjects();
		}
	);
}

void PlayModeStep()
{
	Game::StepControllers(static_cast<float>(timer.GetElapsedSeconds() / 1000.0f));

}

void PlayModeRender()
{
	using namespace Scene;
	if (GetNumSwapChainCameras() > 0ULL)
	{
		WriteConstantsBuffers();
		RenderSceneShadowMaps();
		RenderSceneCameras();
	}
}

void PlayModeLeave()
{
}

//Editor
#if defined(_EDITOR)

void DestroyEditorModeBindings()
{
	DestroyBillboards();
	DestroyRenderableBoundingBox();
	DestroyPickingPass();
}

void EditorModeCreate()
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

void EditorModeStep()
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

	if (RenderableBoundingBoxExists())
	{
		UpdateBoundingBox();
	}

	if (GetNumMouseCameras() > 0ULL)
	{
		GameAreaMouseProcessing(mouse, GetMouseCameras().at(0));
	}

	Game::StepControllers(static_cast<float>(timer.GetElapsedSeconds() / 1000.0f));
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
	bool criticalFrame = (!PickingPassExists() || !RenderableBoundingBoxExists()) && GetNumSwapChainCameras() > 0ULL || Editor::PendingBillboards() || Editor::PendingBillboardsDestruction();

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

				if (Editor::PendingBillboards())
					Editor::CreateRegisteredBillboards();

				if (Editor::PendingBillboardsDestruction())
					Editor::DestroyPendingBillboards();
			}
		);
	}

	PickFromScene();
}

void EditorModeLeave()
{
	renderer->RenderCriticalFrame([]
		{
			DestroyRenderableBoundingBox();
			DestroyPickingPass();
		}
	);
}

#endif

namespace Game
{
	std::map<std::string, std::function<std::shared_ptr<Game::Controller>()>> controllers =
	{
		{ "venom", []() { return std::make_shared<Game::VenomController>(); }},
	};

	std::vector<std::string> GetGameControllers()
	{
		return nostd::GetKeysFromMap(controllers);
	}

	std::shared_ptr<Game::Controller> GetGameController(std::string name)
	{

		return (controllers.contains(name)) ? controllers.at(name)() : nullptr;
	}
};