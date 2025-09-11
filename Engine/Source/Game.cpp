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
//#include <HDR/LuminanceHistogram.h>
//#include <HDR/LuminanceHistogramAverage.h>

//#define RUN_TEST

#if defined(RUN_TEST)
#include <IBL/DiffuseIrradianceMap.h>
#include <IBL/PreFilteredEnvironmentMap.h>
#include <IBL/BRDFLUT.h>
#endif
#include <RenderPass/RenderPass.h>
#include <Renderable/Renderable.h>
#include <Level.h>

using namespace Scene;
using namespace DeviceUtils;
using namespace ComputeShader;
using namespace Editor;

//extern RECT hWndRect;
extern std::unique_ptr<DirectX::Mouse> mouse;

//std::shared_ptr<DeviceUtils::DescriptorHeap> mainPassHeap;
//std::shared_ptr<SwapChainPass> resolvePass;
//std::shared_ptr<RenderToTexturePass> mainPass;
//std::shared_ptr<LuminanceHistogram> hdrHistogram;
//std::shared_ptr<LuminanceHistogramAverage> luminanceHistogramAverage;
#if defined(RUN_TEST)
std::shared_ptr<DiffuseIrradianceMap> diffuseIrradianceMap;
std::shared_ptr<PreFilteredEnvironmentMap> preFilteredEnvironmentMap;
std::shared_ptr<BRDFLUT> brdflut;
#endif

GameStates gameState = GameStates::GS_None;
std::string gameAppTitle = "Culpeo Test Game";
std::shared_ptr<Renderable> bootScreen;
std::shared_ptr<Renderable> loadingScreenBar;
//std::shared_ptr<Renderable> toneMapQuad;
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

/*
std::shared_ptr<RenderToTexturePass> CreateMainPass()
{
	unsigned int width = static_cast<unsigned int>(hWndRect.right - hWndRect.left);
	unsigned int height = static_cast<unsigned int>(hWndRect.bottom - hWndRect.top);

	return CreateRenderPass("mainPass", { DXGI_FORMAT_R32G32B32A32_FLOAT }, DXGI_FORMAT_D32_FLOAT, width, height);
}
*/

void RunRender()
{
	/*
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
		return;
	}
	*/

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
	/*
	DestroyRenderable(bootScreen);
	DestroyRenderable(loadingScreenBar);
	DestroyRenderable(toneMapQuad);
	DestroyCamera(UICamera);
	resolvePass = nullptr;
	*/
}

void GetAudioListenerVectors(std::function<void(XMFLOAT3, XMVECTOR)> audioListenerCallback)
{
	if (!mainPassCamera) return;

	audioListenerCallback(mainPassCamera->position(), mainPassCamera->rotationQ());
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
std::shared_ptr<tween> bootScreenAlphaTween;
void BootScreenCreate()
{
	/*
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
	*/
}

void BootScreenStep()
{
	/*
	bootScreenAlpha = bootScreenAlphaTween->step();
	if (bootScreenAlpha == 1.0f) {
		bootScreenAlphaTween = nullptr;
		gsm.ChangeState(GS_Loading);
	}
	*/
}

void BootScreenRender()
{
	/*
	resolvePass->Pass([](size_t passHash)
		{
			bootScreen->WriteConstantsBuffer<float>("alpha", bootScreenAlpha, renderer->backBufferIndex);
			bootScreen->Render(passHash, UICamera);
		}
	);
	*/
}

void BootScreenLeave()
{
	/*
	renderer->RenderCriticalFrame([]
		{
			DestroyRenderable(bootScreen);
		}
	);
	*/
}

//Loading
std::shared_ptr<tween> loadingScreenProgressTween;
float loadingScreenProgress;
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

void CreateEditorModeBindings()
{
	DestroyRenderableBoundingBox();
	DestroyPickingPass();
}

void EditorModeCreate()
{
	renderer->RenderCriticalFrame([]
		{
			using namespace Scene::Level;

			//mainPass = CreateMainPass();
			//resolvePass = CreateRenderPass("resolvePass", mainPassHeap);

			LoadDefaultLevel();
			//LoadLevel("pyramid");
			//LoadLevel("female");
			//LoadLevel("knight");
			//LoadLevel("spartan");
			//LoadLevel("family");
			//LoadLevel("venom");
			BindSceneObjects();
			CreateEditorModeBindings();

			//hdrHistogram = std::make_shared<LuminanceHistogram>(mainPass->renderToTexture[0]);
			//hdrHistogram->UpdateLuminanceHistogramParams(mainPass->renderToTexture[0]->width, mainPass->renderToTexture[0]->height, -2.0f, 10.0f);
			//
			//luminanceHistogramAverage = std::make_shared<LuminanceHistogramAverage>(hdrHistogram->resultCpuHandle, hdrHistogram->resultGpuHandle);
			//luminanceHistogramAverage->UpdateLuminanceHistogramAverageParams(
			//	mainPass->renderToTexture[0]->width * mainPass->renderToTexture[0]->height,
			//	-2.0f, 10.0f,
			//	0.016f,
			//	1.1f
			//);

#if defined(RUN_TEST)
			//diffuseIrradianceMap = std::make_shared<DiffuseIrradianceMap>("3a6c21c2-362b-4048-b03e-7b36dcf56f43", "Assets/ibl/family/skybox_irradiance.dds");
			//preFilteredEnvironmentMap = std::make_shared<PreFilteredEnvironmentMap>("3a6c21c2-362b-4048-b03e-7b36dcf56f43", "Assets/ibl/family/skybox_prefiltered_env.dds");
			//brdflut = std::make_shared<BRDFLUT>("Assets/ibl/family/skybox_brdf_lut.dds");
#endif

			//BindSwapChainCamera();
			//CreatePickingPass(swapChainCamera);
			//BindPickingRenderables();

			//toneMapQuad = CreateRenderable(
			//	{
			//		{ "meshMaterials" ,
			//			{
			//				{
			//					{ "material", FindMaterialUUIDByName("ToneMap") },
			//					{ "mesh", FindMeshUUIDByName("decal") }
			//				}
			//			}
			//		},
			//		{ "uniqueMaterialInstance", true },
			//		{ "meshShadowMapMaterials", {} },
			//		{ "name", "toneMapQuad" },
			//		{ "uuid", getUUID() },
			//		{ "hidden", true },
			//		{ "castShadows", false },
			//		{ "visible", false },
			//		{ "position", { 0.0, 0.0, 0.0 } },
			//		{ "rotation", { 0.0, 0.0, 0.0 } },
			//		{ "scale", { 1.0, 1.0, 1.0 } } ,
			//		{ "skipMeshes", {} },
			//		{ "pipelineState" ,
			//			{
			//				{ "renderTargetsFormats", { "R8G8B8A8_UNORM" } },
			//				{ "depthStencilFormat", "UNKNOWN" }
			//			}
			//		}
			//	}
			//);
			//
			//std::shared_ptr<MaterialInstance>& toneMapMaterial = toneMapQuad->meshMaterials.begin()->second;
			//toneMapMaterial->textures.insert_or_assign(TextureShaderUsage_Base, GetTextureFromGPUHandle("toneMap", mainPass->renderToTexture[0]->gpuTextureHandle));
			//toneMapMaterial->textures.insert_or_assign(TextureShaderUsage_AverageLuminance, GetTextureFromGPUHandle("averageLuminance", luminanceHistogramAverage->averageReadGpuHandle));
			//
			//toneMapQuad->onMaterialsRebuilt = []()
			//	{
			//		std::shared_ptr<MaterialInstance>& toneMapMaterial = toneMapQuad->meshMaterials.begin()->second;
			//		toneMapMaterial->textures.insert_or_assign(TextureShaderUsage_Base, GetTextureFromGPUHandle("toneMap", mainPass->renderToTexture[0]->gpuTextureHandle));
			//		toneMapMaterial->textures.insert_or_assign(TextureShaderUsage_AverageLuminance, GetTextureFromGPUHandle("averageLuminance", luminanceHistogramAverage->averageReadGpuHandle));
			//	};
			//

			//BindMouseCamera();
			//CreateRenderableBoundingBox(swapChainCamera);
			//CreateRenderablesCameraBinding();
		}
	);

	//mainPassCamera = GetCameraByName("cam.0");
	//mainPassCamera->BindDestruction([] { mainPassCamera = nullptr; });
	//mainPassCamera->PushRenderPass(mainPass);
	//Editor::MapPickingRenderables();
}

void EditorModeStep()
{
	using namespace Scene::Level;

	if (PendingLevelToLoad())
	{
		renderer->Flush();
		renderer->RenderCriticalFrame([]
			{
				LoadPendingLevel();
				CreateEditorModeBindings();
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
		WriteConstantsBuffers();
#if defined(_EDITOR)
		RenderPickingPass(GetSwapChainCameras().at(0));
#endif
		RenderSceneShadowMaps();
#if defined(_EDITOR)
		RenderShadowMapMinMaxChain();
#endif
		RenderSceneCameras();
#if defined(_EDITOR)
		DrawEditor(GetSwapChainCameras().at(0));
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

	/*
	if (mainPassCamera != nullptr)
	{
		WriteConstantsBuffers();

		Editor::RenderPickingPass(mainPassCamera);

		RenderSceneShadowMaps();

		RenderSelectedLightShadowMapChain();

		mainPass->Pass([](size_t passHash)
			{
				RenderSceneObjects(passHash, mainPassCamera);
			}
		);

		hdrHistogram->UpdateLuminanceHistogramParams(mainPass->renderToTexture[0]->width, mainPass->renderToTexture[0]->height, mainPassCamera->minLogLuminance(), mainPassCamera->maxLogLuminance());
		luminanceHistogramAverage->UpdateLuminanceHistogramAverageParams(
			mainPass->renderToTexture[0]->width * mainPass->renderToTexture[0]->height,
			mainPassCamera->minLogLuminance(), mainPassCamera->maxLogLuminance(),
			static_cast<float>(timer.GetElapsedSeconds()),
			mainPassCamera->tau()
		);
		hdrHistogram->Compute();
		luminanceHistogramAverage->Compute();
#if defined(RUN_TEST)
		preFilteredEnvironmentMap->Compute();
		diffuseIrradianceMap->Compute();
		brdflut->Compute();
#endif

		resolvePass->Pass([](size_t passHash)
			{
				toneMapQuad->visible(true);
				DeviceUtils::UAVResource(renderer->commandList, luminanceHistogramAverage->average);
				DeviceUtils::TransitionResource(renderer->commandList, luminanceHistogramAverage->average,
					D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS
				);
				toneMapQuad->Render(passHash, mainPassCamera);
				DeviceUtils::TransitionResource(renderer->commandList, luminanceHistogramAverage->average,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON
				);
				toneMapQuad->visible(false);
				DrawEditor(mainPassCamera);
			}
		);
	}
	else
	{
		resolvePass->Pass([](size_t passHash)
			{
				DrawEditor();
			}
		);
	}
	*/
}

void EditorModePostRender()
{
	/*
#if defined(RUN_TEST)
	diffuseIrradianceMap->Solution();
	preFilteredEnvironmentMap->Solution();
	brdflut->Solution();
#endif
*/
	bool criticalFrame = (!PickingPassExists() || !RenderableBoundingBoxExists()) && GetNumSwapChainCameras() > 0ULL;

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

				if (!RenderableBoundingBoxExists())
				{
					CreateRenderableBoundingBox(GetMouseCameras().at(0));
				}
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
			/*
			Editor::DestroyPickingPass();
#if defined(RUN_TEST)
			diffuseIrradianceMap = nullptr;
			preFilteredEnvironmentMap = nullptr;
			brdflut = nullptr;
#endif
			luminanceHistogramAverage = nullptr;
			hdrHistogram = nullptr;
			mainPass = nullptr;
			mainPassCamera = nullptr;
			*/
		}
	);
}

#endif