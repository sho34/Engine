#include "pch.h"
#include "Game.h"
#include "Renderer/Renderer.h"
#include "Scene/Scene.h"
#include "Renderer/DeviceUtils/DescriptorHeap/DescriptorHeap.h"
#include "Renderer/RenderPass/RenderPass.h"

#if defined(_EDITOR)
#include "Editor/Editor.h"
using namespace Editor;
#endif

//#include "tweeny-3.2.0.h"

using namespace Scene;
using namespace RenderPass;
using namespace DeviceUtils;

extern RECT hWndRect;
std::shared_ptr<DeviceUtils::DescriptorHeap> mainPassHeap;
std::shared_ptr<SwapChainPass> resolvePass;
std::shared_ptr<RenderToTexturePass> mainPass;
GameStates gameState = GameStates::GS_None;
std::string gameAppTitle = "Culpeo Test Game";
std::shared_ptr<Renderable> bootScreen;
std::shared_ptr<Renderable> loadingScreenBar;
std::shared_ptr<Camera> UICamera;
std::shared_ptr<Camera> mainPassCamera;

extern std::shared_ptr<Renderer> renderer;

template<typename T>
struct GameStatesMachine {
	T currentState;
	std::map<T, std::function<void()>> onEnter;
	std::map<T, std::function<void()>> onLeave;
	std::map<T, std::function<void()>> onStep;
	std::map<T, std::function<void()>> onRender;

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
	}
};

void SetupRenderPipeline()
{
	mainPassHeap = std::make_shared<DeviceUtils::DescriptorHeap>();
	mainPassHeap->CreateDescriptorHeap(renderer->d3dDevice, renderer->numFrames);
}

void DestroyRenderPipeline()
{
	resolvePass = nullptr;
	mainPassHeap->DestroyDescriptorHeap();
	mainPassHeap = nullptr;
}

void RunRender()
{
	using namespace Scene;

	if (!resolvePass) return;

	if (!GetNumCameras())
	{
		resolvePass->BeginRenderPass();
		resolvePass->EndRenderPass();
		return;
	}

	gsm.Render();
}

void GameStep()
{
	gsm.Step();
}

void GameDestroy()
{
	gsm.ChangeState(GS_Destroy);
	DestroyRenderable(bootScreen);
	DestroyRenderable(loadingScreenBar);
	DestroyCamera(UICamera);
	resolvePass = nullptr;
}

void GetAudioListenerVectors(std::function<void(XMFLOAT3 pos, XMFLOAT3 fw, XMFLOAT3 up)> audioListenerCallback)
{
	if (!mainPassCamera) return;

	XMFLOAT3 pos = mainPassCamera->position;
	XMVECTOR fw = mainPassCamera->CameraFw();
	XMVECTOR up = mainPassCamera->CameraUp();

	audioListenerCallback(pos, *(XMFLOAT3*)&fw.m128_f32, *(XMFLOAT3*)&up.m128_f32);
}

void WindowResizeReleaseResources()
{
	if (mainPass) mainPass->ReleaseResources();
	if (resolvePass) resolvePass->ReleaseResources();
}

void WindowResize(unsigned int width, unsigned int height)
{
	if (mainPass) mainPass->Resize(width, height);
	if (resolvePass) resolvePass->Resize(width, height);
}

//Booting
float bootScreenAlpha = 0.0f;
std::shared_ptr<tween> bootScreenAlphaTween;
void BootScreenCreate()
{
	renderer->RenderCriticalFrame([]
		{
			resolvePass = CreateRenderPass("resolvePass", mainPassHeap);

			bootScreen = CreateRenderable(
				R"(
				{
					"meshMaterials": [
						{
							"material": "FullScreenQuad",
							"mesh" : "decal",
							"textures": [
								{ "path": "Assets/ui/logo.dds", "format":"R8G8B8A8_UNORM", "numFrames":0 }
							]
						}
					],
					"name": "bootScreen",
					"pipelineState": {
						"depthStencilFormat":"UNKNOWN"
					}
				})"_json
			);

			UICamera = CreateCamera(
				R"(
					{
						"fitWindow":true,
						"name" : "bootCamera",
						"perspective" : {
							"farZ": 100.0,
							"fovAngleY" : 1.2217305898666382,
							"height" : 920,
							"nearZ" : 0.009999999776482582,
							"width" : 1707
						},
						"position": [-5.699999809265137, 2.200000047683716, 3.799999952316284] ,
						"projectionType" : "Perspective",
						"rotation" : [1.5707963705062866, -0.19634954631328583, 0.0] ,
						"speed" : 0.05000000074505806
				})"_json
			);
		}
	);

	bootScreenAlphaTween = std::make_shared<tween>(tween(0.0f, 1.0f, 1000, tween::easing::linear));
}

void BootScreenStep()
{
	bootScreenAlpha = bootScreenAlphaTween->step();
	if (bootScreenAlpha == 1.0f) {
		bootScreenAlphaTween = nullptr;
		gsm.ChangeState(GS_Loading);
	}
}

void BootScreenRender()
{
	resolvePass->BeginRenderPass();

	bootScreen->WriteConstantsBuffer<float>("alpha", bootScreenAlpha, renderer->backBufferIndex);
	bootScreen->Render(UICamera);

	resolvePass->EndRenderPass();
}

void BootScreenLeave()
{
	renderer->RenderCriticalFrame([]
		{
			DestroyRenderable(bootScreen);
		}
	);
}

//Loading
std::shared_ptr<tween> loadingScreenProgressTween;
float loadingScreenProgress;
void LoadingScreenCreate()
{
	renderer->RenderCriticalFrame([]
		{
			loadingScreenBar = CreateRenderable(
				R"(
				{
					"meshMaterials": [
						{
							"material": "LoadingBar",
							"mesh" : "decal"
						}
					],
					"name": "loadingScreenBar",
					"pipelineState": {
						"depthStencilFormat":"UNKNOWN"
					}
				})"_json
			);
		}
	);

	loadingScreenProgressTween = std::make_shared<tween>(tween(0.0f, 1.0f, 4000, tween::easing::linear));
}

void LoadingScreenLeave()
{
	renderer->RenderCriticalFrame([]
		{
			DestroyRenderable(loadingScreenBar);
			DestroyCamera(UICamera);
		}
	);
	loadingScreenProgressTween = nullptr;
}

void LoadingScreenStep()
{
	loadingScreenProgress = loadingScreenProgressTween->step();
	if (loadingScreenProgress == 1.0f)
	{
		gsm.ChangeState(GS_Playing);
	}
}

void LoadingScreenRender()
{
	resolvePass->BeginRenderPass();

	XMFLOAT2 pos(0.0f, -0.8f);
	XMFLOAT2 scale(0.8f, 0.02f);
	auto red = DirectX::Colors::Red;
	auto blue = DirectX::Colors::Blue;
	loadingScreenBar->WriteConstantsBuffer<XMFLOAT2>("pos", pos, renderer->backBufferIndex);
	loadingScreenBar->WriteConstantsBuffer<XMFLOAT2>("scale", scale, renderer->backBufferIndex);
	loadingScreenBar->WriteConstantsBuffer<XMVECTORF32>("color1", red, renderer->backBufferIndex);
	loadingScreenBar->WriteConstantsBuffer<XMVECTORF32>("color2", blue, renderer->backBufferIndex);
	loadingScreenBar->WriteConstantsBuffer<float>("progress", loadingScreenProgress, renderer->backBufferIndex);
	loadingScreenBar->Render(UICamera);

	resolvePass->EndRenderPass();
}

//Playing
void PlayModeCreate()
{
	renderer->RenderCriticalFrame([]
		{
			LoadLevel("knight");
			unsigned int width = static_cast<unsigned int>(hWndRect.right - hWndRect.left);
			unsigned int height = static_cast<unsigned int>(hWndRect.bottom - hWndRect.top);
			mainPass = CreateRenderPass("mainPass", { DXGI_FORMAT_R8G8B8A8_UNORM }, DXGI_FORMAT_D32_FLOAT, width, height);
		}
	);
	mainPassCamera = GetCamera("cam.0");
}

void PlayModeStep()
{

}

void PlayModeRender()
{
	WriteConstantsBuffers();

	RenderSceneShadowMaps();

	mainPass->BeginRenderPass();
	{
		RenderSceneObjects(mainPassCamera);
	}
	mainPass->EndRenderPass();

	resolvePass->BeginRenderPass();
	{
		resolvePass->CopyFromRenderToTexture(mainPass->renderToTexture[0]);
	}
	resolvePass->EndRenderPass();
}

void PlayModeLeave()
{
	renderer->RenderCriticalFrame([]
		{
			mainPass = nullptr;
			mainPassCamera = nullptr;
		}
	);
}

//Editor
#if defined(_EDITOR)
void EditorModeCreate()
{
	renderer->RenderCriticalFrame([]
		{
			resolvePass = CreateRenderPass("resolvePass", mainPassHeap);

			//LoadDefaultLevel();
			LoadLevel("knight");
			unsigned int width = static_cast<unsigned int>(hWndRect.right - hWndRect.left);
			unsigned int height = static_cast<unsigned int>(hWndRect.bottom - hWndRect.top);
			mainPass = CreateRenderPass("mainPass", { DXGI_FORMAT_R8G8B8A8_UNORM }, DXGI_FORMAT_D32_FLOAT, width, height);

			CreateRenderableBoundingBox();
		}
	);
	mainPassCamera = GetCamera("cam.0");
}

void EditorModeStep()
{
	WriteRenderableBoundingBoxConstantsBuffer();
}

void EditorModeRender()
{
	WriteConstantsBuffers();

	RenderSelectedLightShadowMapChain();

	RenderSceneShadowMaps();

	mainPass->BeginRenderPass();
	{
		RenderSceneObjects(mainPassCamera);
	}
	mainPass->EndRenderPass();

	resolvePass->BeginRenderPass();
	{
		resolvePass->CopyFromRenderToTexture(mainPass->renderToTexture[0]);
		DrawEditor();
	}
	resolvePass->EndRenderPass();
}

void EditorModeLeave()
{
	renderer->RenderCriticalFrame([]
		{
			mainPass = nullptr;
			mainPassCamera = nullptr;
		}
	);
}
#endif