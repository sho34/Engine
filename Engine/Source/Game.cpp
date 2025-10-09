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

using namespace Scene;
using namespace DeviceUtils;
using namespace ComputeShader;

extern std::unique_ptr<DirectX::Mouse> mouse;

GameStates gameState = GameStates::GS_None;
std::string gameAppTitle = "Culpeo Test Game";

extern std::shared_ptr<Renderer> renderer;

template<typename T>
struct GameStatesMachine {
	T currentState;
	std::map<T, std::function<void()>> onEnter;
	std::map<T, std::function<void()>> onLeave;
	std::map<T, std::function<void()>> onStep;
	std::map<T, std::function<void()>> onRender;
	std::map<T, std::function<void()>> onPostRender;

	void ChangeState(T newState)
	{
		if (onLeave.contains(currentState)) { onLeave.at(currentState)(); }
		currentState = newState;
		if (onEnter.contains(currentState)) { onEnter.at(currentState)(); }
	}

	void Step()
	{
		if (onStep.contains(currentState)) { onStep.at(currentState)(); }
	}

	void Render()
	{
		if (onRender.contains(currentState)) { onRender.at(currentState)(); }
	}

	void PostRender()
	{
		if (onPostRender.contains(currentState)) { onPostRender.at(currentState)(); }
	}
};

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
	loadingProgressTween = std::make_shared<tween>(tween(0.0f, 1.0f, 4000, tween::easing::linear));
}

void BootScreenStep()
{
	bootScreenAlpha = bootAlphaTween->step();
	loadingProgress = loadingProgressTween->step();

	/*
	if (bootScreenAlpha == 1.0f) {
		bootScreenAlphaTween = nullptr;
		gsm.ChangeState(GS_Loading);
	}
	*/
}

void BootScreenRender()
{
	using namespace Scene;
	if (GetNumSwapChainCameras() > 0ULL)
	{
		XMFLOAT2 pos(0.0f, -0.8f);
		XMFLOAT2 scale(0.8f, 0.02f);
		auto red = DirectX::Colors::Red;
		auto blue = DirectX::Colors::Blue;

		bootScreen->WriteConstantsBuffer("alpha", bootScreenAlpha, renderer->backBufferIndex);
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

void BootScreenLeave()
{
	bootScreen = nullptr;
}

//Loading
void LoadingScreenCreate()
{
	/*
	renderer->RenderCriticalFrame([]
		{
			loadingScreenBar = CreateRenderable(nlohmann::json(
				{
					{ "uuid", getUUID() },
					{ "name", "loadingScreenBar" },
					{ "depthStencilFormat", "UNKNOWN" },
					{ "meshMaterials",
						{
							{
								{ "material", FindMaterialUUIDByName("LoadingBar") },
								{ "mesh", FindMeshUUIDByName("decal") }
							}
						}
					}
				}
			));
		}
	);

	loadingScreenProgressTween = std::make_shared<tween>(tween(0.0f, 1.0f, 4000, tween::easing::linear));
	*/
}

void LoadingScreenLeave()
{
	/*
	renderer->RenderCriticalFrame([]
		{
			DestroyRenderable(loadingScreenBar);
			DestroyCamera(UICamera);
		}
	);
	loadingScreenProgressTween = nullptr;
	*/
}

void LoadingScreenStep()
{
	/*
	loadingScreenProgress = loadingScreenProgressTween->step();
	if (loadingScreenProgress == 1.0f)
	{
		gsm.ChangeState(GS_Playing);
	}
	*/
}

void LoadingScreenRender()
{
	/*
	resolvePass->Pass([](size_t passHash)
		{
			XMFLOAT2 pos(0.0f, -0.8f);
			XMFLOAT2 scale(0.8f, 0.02f);
			auto red = DirectX::Colors::Red;
			auto blue = DirectX::Colors::Blue;
			loadingScreenBar->WriteConstantsBuffer<XMFLOAT2>("pos", pos, renderer->backBufferIndex);
			loadingScreenBar->WriteConstantsBuffer<XMFLOAT2>("scale", scale, renderer->backBufferIndex);
			loadingScreenBar->WriteConstantsBuffer<XMVECTORF32>("color1", red, renderer->backBufferIndex);
			loadingScreenBar->WriteConstantsBuffer<XMVECTORF32>("color2", blue, renderer->backBufferIndex);
			loadingScreenBar->WriteConstantsBuffer<float>("progress", loadingScreenProgress, renderer->backBufferIndex);
			loadingScreenBar->Render(passHash, UICamera);
		}
	);
	*/
}

//Playing
void PlayModeCreate()
{
	/*
	renderer->RenderCriticalFrame([]
		{
			LoadLevel("family");
			mainPass = CreateMainPass();
		}
	);
	mainPassCamera = GetCameraByName("cam.0");
	*/
}

void PlayModeStep()
{

}

void PlayModeRender()
{
	/*
	WriteConstantsBuffers();

	RenderSceneShadowMaps();

	mainPass->Pass([](size_t passHash)
		{
			RenderSceneObjects(passHash, mainPassCamera);
		}
	);

	resolvePass->Pass([](size_t passHash)
		{
			resolvePass->CopyFromRenderToTexture(mainPass->renderToTexture[0]);
		}
	);
	*/
}

void PlayModeLeave()
{
	/*
	renderer->RenderCriticalFrame([]
		{
			mainPass = nullptr;
			mainPassCamera = nullptr;
		}
	);
	*/
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
			LoadLevel("bootscreen");
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
}

extern DX::StepTimer timer;
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