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
std::filesystem::path levelToLoad;
namespace Editor {
	extern std::string selSO;
	extern std::string selTemp;
};
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
		resolvePass->Pass([](size_t passHash)
			{
#if defined(_EDITOR)
				DrawEditor();
#endif
			}
		);
		/*
		resolvePass->BeginRenderPass();
		{
#if defined(_EDITOR)
			DrawEditor();
#endif
		}
		resolvePass->EndRenderPass();
		*/
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

	XMFLOAT3 pos = mainPassCamera->position();
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

			bootScreen = CreateRenderable(nlohmann::json(
				{
					{ "uuid", getUUID() },
					{ "name" ,"bootScreen" },
					{ "meshMaterials",
						{
							{
								{ "material", FindMaterialUUIDByName("FullScreenQuad") },
								{ "mesh", FindMeshUUIDByName("decal") },
								{ "textures",
									{
										{
											"BaseTexture" ,
											{
												{ "path", "Assets/ui/logo.dds" },
												{ "format", "B8G8R8A8_UNORM_SRGB" },
												{ "numFrames", 0 }
											}
										}
									}
								}
							}
						}
					},
					{ "depthStencilFormat", "UNKNOWN" }
				}
			));

			UICamera = CreateCamera(nlohmann::json(
				{
					{ "uuid", getUUID() },
					{ "name", "bootCamera" },
					{ "fitWindow", true },
					{ "perspective", {
							{ "farZ", 100.0 },
							{ "fovAngleY", 1.222f },
							{ "height", 920 },
							{ "nearZ", 0.01f },
							{ "width", 1707 },
						}
					},
					{ "position", { -5.7f, 2.2f, 3.8f } },
					{ "projectionType", "Perspective" },
					{ "rotation", { 1.6f, -0.2f, 0.0 } },
					{ "speed", 0.05f }
				}
			));
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
	resolvePass->Pass([](size_t passHash)
		{
			bootScreen->WriteConstantsBuffer<float>("alpha", bootScreenAlpha, renderer->backBufferIndex);
			bootScreen->Render(passHash, UICamera);
		}
	);
	/*
	resolvePass->BeginRenderPass();

	bootScreen->WriteConstantsBuffer<float>("alpha", bootScreenAlpha, renderer->backBufferIndex);
	bootScreen->Render(UICamera);

	resolvePass->EndRenderPass();
	*/
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
	/*
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
	*/
}

//Playing
void PlayModeCreate()
{
	renderer->RenderCriticalFrame([]
		{
			LoadLevel("family");
			unsigned int width = static_cast<unsigned int>(hWndRect.right - hWndRect.left);
			unsigned int height = static_cast<unsigned int>(hWndRect.bottom - hWndRect.top);
			mainPass = CreateRenderPass("mainPass", { DXGI_FORMAT_R8G8B8A8_UNORM }, DXGI_FORMAT_D32_FLOAT, width, height);
		}
	);
	mainPassCamera = GetCameraByName("cam.0");
}

void PlayModeStep()
{

}

void PlayModeRender()
{
	WriteConstantsBuffers();

	RenderSceneShadowMaps();

	mainPass->Pass([](size_t passHash)
		{
			RenderSceneObjects(passHash, mainPassCamera);
		}
	);
	/*
	mainPass->BeginRenderPass();
	{
		RenderSceneObjects(mainPassCamera);
	}
	mainPass->EndRenderPass();
	*/

	resolvePass->Pass([](size_t passHash)
		{
			resolvePass->CopyFromRenderToTexture(mainPass->renderToTexture[0]);
		}
	);
	/*
	resolvePass->BeginRenderPass();
	{
		resolvePass->CopyFromRenderToTexture(mainPass->renderToTexture[0]);
	}
	resolvePass->EndRenderPass();
	*/
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
			//LoadLevel("knight-original");
			LoadLevel("female");
			//LoadLevel("knight");
			//LoadLevel("baseLevel");
			//LoadLevel("pyramid");
			//LoadLevel("jumpsuit");
			//LoadLevel("venom");
			unsigned int width = static_cast<unsigned int>(hWndRect.right - hWndRect.left);
			unsigned int height = static_cast<unsigned int>(hWndRect.bottom - hWndRect.top);
			mainPass = CreateRenderPass("mainPass", { DXGI_FORMAT_R8G8B8A8_UNORM }, DXGI_FORMAT_D32_FLOAT, width, height);

			CreateRenderableBoundingBox();
		}
	);
	mainPassCamera = GetCameraByName("cam.0");
	mainPassCamera->BindDestruction([]
		{
			mainPassCamera = nullptr;
		}
	);
}

void EditorModeStep()
{
	if (!levelToLoad.empty())
	{
		renderer->Flush();
		renderer->RenderCriticalFrame([]
			{
				DestroySceneObjects();
				selSO = "";
				selTemp = "";
				LoadLevel(levelToLoad);
				levelToLoad = "";
				CreateRenderableBoundingBox();
			}
		);
		mainPassCamera = GetCameraByName("cam.0");
		return;
	}
	WriteRenderableBoundingBoxConstantsBuffer();
}

void EditorModeRender()
{
	if (mainPassCamera != nullptr)
	{
		WriteConstantsBuffers();

		RenderSceneShadowMaps();

		RenderSelectedLightShadowMapChain();

		mainPass->Pass([](size_t passHash)
			{
				RenderSceneObjects(passHash, mainPassCamera);
			}
		);
		/*
		mainPass->BeginRenderPass();
		{
			RenderSceneObjects(mainPassCamera);
		}
		mainPass->EndRenderPass();
		*/

		resolvePass->Pass([](size_t passHash)
			{
				resolvePass->CopyFromRenderToTexture(mainPass->renderToTexture[0]);
				DrawEditor();
			}
		);
		/*
		resolvePass->BeginRenderPass();
		{
			resolvePass->CopyFromRenderToTexture(mainPass->renderToTexture[0]);
			DrawEditor();
		}
		resolvePass->EndRenderPass();
		*/
	}
	else
	{
		resolvePass->Pass([](size_t passHash)
			{
				DrawEditor();
			}
		);
		/*
		resolvePass->BeginRenderPass();
		{
			DrawEditor();
		}
		resolvePass->EndRenderPass();
		*/
	}
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