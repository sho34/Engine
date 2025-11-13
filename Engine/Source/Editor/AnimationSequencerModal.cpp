#include "pch.h"
#include "AnimationSequencerModal.h"
#include <imgui.h>
#include <ImEditor.h>
#include <nlohmann/json.hpp>
#include <Camera/Camera.h>
#include <Light/Light.h>
#include <Renderable/Renderable.h>
#include <Renderer.h>
#include <Scene.h>
#include <NoMath.h>

extern std::unique_ptr<Renderer> renderer;
extern DX::StepTimer timer;

namespace Editor
{
	extern bool templatesModified;
};

void AnimationSequencerModal::Initialize(JUUID uuid)
{
	showing = true;
	initializing = true;
	model3dUUID = uuid;
	currentFrame = 0;
	model3D = uuid;
	animationsSequences = model3D->animationSequences();
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 1")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 2")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 3")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 4")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 5")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 6")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 7")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 8")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 9")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 10")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 11")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 12")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 13")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 14")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 15")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 16")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 17")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 18")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 19")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 20")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 21")));
	animationsSequences.sequences["sa"].sequenceChannels.push_back(SequenceChannel(std::string("Channel 22")));

}

void AnimationSequencerModal::LoadSceneObjects()
{
	using namespace Scene;

	camera = getUUID();
	ambientLight = getUUID();
	directionalLight = getUUID();
	renderable = getUUID();
	floor = getUUID();

	nlohmann::json cameraJson = {
		{ "fitWindow", false },
		//{ "fitWindow", true },
		{ "name", "cam-preview" },
		{ "perspective",
			{
				{ "farZ", 1000.0 },
				{ "fovAngleY", 20.0 },
				{ "nearZ", 0.01 },
				{ "width", 1778 },
				{ "height", 1000 }
			}
		},
		{ "position", { 0.0, 5.21317195892334, -17.224170684814453 } },
		{ "projectionType", "Perspective" },
		{ "rotation", { 12.898999214172363, 0.0, 0.0 } },
		{ "speed", 0.05000000074505806 },
		{ "uuid", camera() },
		{
			"renderPasses", { GetRenderPassUUIDByName("ModelPreviewPass")}
		},
		{ "mouseController", false },
		{ "useSwapChain", false },
		{ "modelDistanceScale", 1.0f } //dynamic magic
	};

	nlohmann::json ambientLightJson =
	{
		{ "color", { 0.25000000074505806, 0.25000000074505806, 0.25000000074505806 } },
		{ "lightType", "Ambient" },
		{ "name", "light.0.amb-preview" },
		{ "uuid", ambientLight()},
		{ "cameras", { camera() }}
	};

	nlohmann::json directionalLightJson =
	{
		{ "color", { 1.0, 1.0, 1.0} },
		{ "farZ" , 1000.0},
		{ "nearZ", 0.01},
		{ "hasShadowMaps", true },
		{ "shadowMapHeight", 4096},
		{ "shadowMapWidth", 4096},
		{ "viewHeight", 1.0},
		{ "viewWidth", 1.0},
		{ "rotation", {40.31087875366211, -10.30000039935112, 0.0} },
		{ "lightType", "Directional"},
		{ "name", "light.1.dir-preview"},
		{ "uuid", directionalLight() },
		{ "zBias", 0.000002 },
		{ "cameras", { camera() } }
	};

	nlohmann::json animableJson =
	{
		{ "castShadows", true },
		{ "shadowed", false },
		{ "model", model3dUUID()},
		{ "name", "preview-model" },
		{ "position", { 0.0, 0.0, 0.0} },
		{ "rotation", { 0.0, -90.0, 0.0 }},
		{ "scale", { 0.1, 0.1, 0.1} },
		{ "uuid", renderable() },
		{ "cameras", { camera() } }
	};

	nlohmann::json floorJson =
	{
		{ "castShadows", false },
		{ "shadowed", true },
				{
					"meshMaterials",
					{
						{
							{ "material", "ecd1688c-73d6-49d0-870f-ca916a417c49"},
							{ "mesh", "d41e5c29-49bb-4f2c-aa2b-da781fbac512" }
						}
					}
				},
		{ "name", "preview-floor" },
		{ "position", { 0.0, 0.0, 0.0} },
		{ "scale", { 100.0, 100.0, 100.0} },
		{ "uuid", floor() },
		{ "cameras", { camera() } },
		{ "floorColor", { 0.5f, 0.5f, 0.5f } }
	};

	CreateCamera(cameraJson);
	CreateLight(ambientLightJson);
	CreateLight(directionalLightJson);
	CreateRenderable(animableJson);
	CreateRenderable(floorJson);

	camera->BindToScene();
	renderable->BindToScene();
	floor->BindToScene();
	ambientLight->BindToScene();
	directionalLight->BindToScene();

	camera->UpdateProjection();
	directionalLight->shadowMapCameras[0]->UpdateProjection();

	/*
	expandedChannels.clear();
	for (auto& [seqName, sequence] : animationsSequences.sequences)
	{
		expandedChannels.insert_or_assign(seqName, std::vector<bool>(sequence.channels.size(), false));
	}

	if (!renderable->animable.empty())
	{
		animations = nostd::GetKeysFromMap(renderable->animable->animations->animationsLength);
		animations.erase(animations.begin());
	}
	*/

	selectedSequence = "";
	initializing = false;
	addNewSequence = false;
	newSequenceName = "";
}

void AnimationSequencerModal::DestroySceneObjects()
{
	using namespace Scene;

	renderable->UnbindFromScene();
	floor->UnbindFromScene();
	directionalLight->UnbindFromScene();
	ambientLight->UnbindFromScene();
	camera->UnbindFromScene();

	DeleteRenderable(renderable());
	DeleteRenderable(floor());
	DeleteLight(directionalLight());
	DeleteLight(ambientLight());
	DeleteCamera(camera());

	renderable.clear();
	floor.clear();
	directionalLight.clear();
	ambientLight.clear();
	camera.clear();
	model3D.clear();
	model3dUUID.clear();
	selectedSequence.clear();
	//animations.clear();
	//expandedChannels.clear();
	addNewSequence = false;
	newSequenceName.clear();
}

void AnimationSequencerModal::Step()
{
	XMFLOAT3 baseColor = ToXMFLOAT3(floor->at("floorColor"));
	floor->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, renderer->backBufferIndex);

	//https://stackoverflow.com/a/32836605
	renderable->StepAnimation(static_cast<FLOAT>(timer.GetElapsedSeconds()));
	BoundingBox modelBB = renderable->GetBoundingBox();
	BoundingSphere modelBBS;
	BoundingSphere::CreateFromBoundingBox(modelBBS, modelBB);

	float modelDistanceScale = camera->at("modelDistanceScale");
	float fov = XMConvertToRadians(camera->perspective().fovAngleY);
	float distance = modelDistanceScale * (modelBBS.Radius * 2.0f) / (XMScalarSin(fov) / XMScalarCos(fov));

	XMVECTOR camFwV = camera->forward();
	XMFLOAT3 BBPos = modelBB.Center;
	XMVECTOR BBPosV = XMLoadFloat3(&BBPos);
	XMVECTOR camPosV = XMVectorSubtract(BBPosV, XMVectorScale(camFwV, distance));
	XMFLOAT3 camPos;
	XMStoreFloat3(&camPos, camPosV);
	camera->position(camPos);

	//XMFLOAT3 dlRot = directionalLight->rotation();
	//dlRot.y -= 0.5f;
	//directionalLight->rotation(dlRot);
}

static ImVec2 modelPosAdj(0.0f, 21.0f);
static ImVec2 sequencerSizeAdj(0.0f, -47.0f);
static ImVec2 sequencerPosAdj(0.0f, 4.0f);
static float titleBarH = 19.0f;
void AnimationSequencerModal::DrawSequencer(const char* title, ImVec2 pos, ImVec2 size)
{
	ImGui::OpenPopup(title);

	ImVec2 modalSize(size.x, size.y + titleBarH);
	ImGui::SetNextWindowPos(pos);
	//ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowSize(modalSize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	bool exit = false;
	bool saveexit = false;

	auto onSelectSequence = [this](std::string sequence) {
		selectedSequence = sequence;
		Sequence& seq = animationsSequences.sequences.at(sequence);

		std::transform(seq.sequenceChannels.begin(), seq.sequenceChannels.end(), std::back_inserter(expandedChannels), [](auto& _) { return false; });
		//auto it = animationsSequences.sequences.find(sequence);
		//bool isAnim = it != animationsSequences.sequences.end();
		//renderable->SetCurrentAnimation(isAnim ? &(it->second) : nullptr, 0.0f, 0.0f, false, false);
		};
	auto onAddNewSequence = [this]() {
		addNewSequence = true;
		newSequenceName.clear();
		};
	auto onEraseSequence = [this, onSelectSequence](std::string sequence)
		{
			animationsSequences.sequences.erase(sequence);
			onSelectSequence("");
		};
	auto onAddNewSequenceClicked = [this](std::string seqName)
		{
			addNewSequence = false;
			animationsSequences.sequences.insert_or_assign(seqName, Sequence());
			selectedSequence = seqName;
			//int totalFrames = static_cast<int>(renderable->animable->animations->animationsLength.at(animations.at(0)) * 60);
			//totalFrames /= 1000;
			//animationsSequences.sequences.insert_or_assign(
			//	newSequenceName,
			//	Sequence(
			//		animations.at(0),
			//		totalFrames
			//	)
			//);
			//expandedChannels.insert_or_assign(newSequenceName, std::vector<bool>({ false }));
			//selectedSequence = newSequenceName;
			//auto it = animationsSequences.sequences.find(selectedSequence);
			//bool isAnim = it != animationsSequences.sequences.end();
			//renderable->SetCurrentAnimation(isAnim ? &(it->second) : nullptr, 0.0f, 0.0f, false, renderable->animationLoop());
		};
	auto onCancelAddNewSequenceClick = [this]()
		{
			addNewSequence = false;

		};

	if (ImGui::BeginPopupModal(title, nullptr, defaultChildFlag))
	{
		ImVec2 titleSize(size.x - 1, titleBarH);
		ImVec2 titlePos(pos.x + 1, pos.y);
		DrawTitleBar(title, titlePos, titleSize, exit);

		pos.y += titleBarH;
		DrawSequenceSelector(pos, onSelectSequence, onEraseSequence, onAddNewSequence);

		ImVec2 modelSize(size.y * 0.5f * 16.0f / 9.0f, size.y * 0.5f);
		ImVec2 modelPos(pos.x + (size.x - modelSize.x) * 0.5f + modelPosAdj.x, pos.y + modelPosAdj.y);

		DrawModelPreview(modelPos, modelSize);

		ImVec2 timeControllerSize(size.x, 20.0f);
		ImVec2 timeControllerPos(pos.x, modelPos.y + modelSize.y);

		if (!selectedSequence.empty())
		{
			DrawTimelineController(timeControllerPos, timeControllerSize, animationsSequences.sequences.at(selectedSequence));
		}

		ImVec2 sequencerPos(pos.x, timeControllerPos.y + timeControllerSize.y + sequencerPosAdj.y);
		ImVec2 sequencerSize(size.x + sequencerSizeAdj.x, size.y - modelSize.y - timeControllerSize.y + sequencerSizeAdj.y);
		static int selectedEntry = -1;
		static int firstFrame = 0;
		static bool expanded = true;
		//static int currentFrame = 100;

		if (!selectedSequence.empty())
		{
			//DrawSequencer(sequencerPos, sequencerSize, animationsSequences.sequences.at(selectedSequence), currentFrame, expanded, selectedEntry, firstFrame);
			DrawSequencer(sequencerPos, sequencerSize, animationsSequences.sequences.at(selectedSequence));
		}

		ImVec2 buttonsPos(pos.x, sequencerPos.y + sequencerSize.y);
		ImVec2 buttonSize(200.0f, 20.0f);
		DrawSaveAndExitButtons(buttonsPos, buttonSize, exit, saveexit);

		if (addNewSequence)
		{
			ImVec2 newSeqSize(200, 75);
			ImVec2 newSeqPos(pos.x + (size.x - newSeqSize.x) * 0.5f, pos.y + (size.y - newSeqSize.y) * 0.5f);
			DrawAddNewSequencePopup(newSeqPos, newSeqSize, newSequenceName, onAddNewSequenceClicked, onCancelAddNewSequenceClick);
		}

		switch (popup)
		{
		case SMP_AddElement:
		{
			DrawAddNewElementToTimelinePopup();
		}
		break;
		case SMP_InteractWithElement:
		{
			DrawElementInteractPopup();
		}
		break;
		}

		/*
		if (addElementPopup)
		{
			ImVec2 newElemSize(200, 75);
			ImVec2 newElemPos(pos.x + (size.x - newElemSize.x) * 0.5f, pos.y + (size.y - newElemSize.y) * 0.5f);

			DrawAddNewElementToTimelinePopup(newElemPos, newElemSize);
		}

		if (elementInteractPopup)
		{
			DrawElementInteractPopup();
		}
		*/

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	if (exit)
	{
		Exit();
	}
	if (saveexit)
	{
		SaveAndExit();
	}
}

void AnimationSequencerModal::DrawTitleBar(const char* title, ImVec2 pos, ImVec2 size, bool& exit)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(45.0f / 255.0f, 62.0f / 255.0f, 104.0f / 255.0f, 1.0f));

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::BeginChild("title-bar", size, 0);
	{
		float windowWidth = ImGui::GetWindowSize().x;
		float textWidth = ImGui::CalcTextSize(title).x;
		ImVec2 textScreenPos(pos.x + (windowWidth - textWidth) * 0.5f, pos.y + 4.0f);
		ImGui::SetCursorScreenPos(textScreenPos);
		ImGui::Text(title);

		ImVec2 closeButtonScreenPos(pos.x + windowWidth - 20.0f, pos.y);
		//ImGui::SetCursorPos(ImVec2(windowWidth - 19.0f, 0.0f));
		ImGui::SetCursorScreenPos(closeButtonScreenPos);
		if (ImGui::Button(ICON_FA_TIMES, ImVec2(19.0f, 19.0f)))
		{
			exit = true;
		}
	}
	ImGui::EndChild();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawSequenceSelector(
	ImVec2 screenPos,
	std::function<void(std::string)> onSelectSequence,
	std::function<void(std::string)> onEraseSequence,
	std::function<void()> onAddSequence
)
{
	std::string title = "Sequence";

	ImVec2 textSize = ImGui::CalcTextSize(title.c_str());
	ImVec2 selectorSize(200, 50);
	ImVec2 windowSize(textSize.x + selectorSize.x + 6, selectorSize.y);
	ImVec2 windowPos(screenPos.x + 4, screenPos.y + 2);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(windowSize);
	ImGui::BeginChild("sequence-selector", windowSize, 0);
	{
		ImGui::Text(title.c_str());

		std::vector<std::string> sequences = { "" };
		std::vector<std::string> modelSequences = nostd::GetKeysFromMap(animationsSequences.sequences);
		nostd::AppendToVector(sequences, modelSequences);

		ImGui::DrawItemWithEnabledState([this, onEraseSequence]
			{
				if (ImGui::Button(ICON_FA_TIMES))
				{
					onEraseSequence(selectedSequence);
				}
			}
		, selectedSequence != "");
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_PLUS))
		{
			onAddSequence();
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(selectorSize.x);
		ImGui::DrawComboSelection(selectedSequence, sequences, onSelectSequence);
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawModelPreview(ImVec2 curPos, ImVec2 size)
{
	if (camera.empty()) return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	ImGui::SetNextWindowSize(size, 0);
	ImGui::SetNextWindowPos(curPos, 0);
	ImGui::BeginChild("model-preview", size, 0);
	{
		auto& pass = camera->renderPassesUUID.at(0);
		ImGui::DrawTextureImage(
			(ImTextureID)
			pass->renderToTexturePass->renderToTexture[0]->gpuTextureHandle.ptr,
			pass->renderToTexturePass->renderToTexture[0]->width,
			pass->renderToTexturePass->renderToTexture[0]->height
		);
	}
	ImGui::EndChild();

	ImVec2 attsPos(curPos.x + size.x + 20.0f, curPos.y);
	ImVec2 attsSize(300.0f, 200.0f);
	ImGui::SetNextWindowPos(attsPos, 0);
	ImGui::SetNextWindowSize(attsSize, 0);
	ImGui::BeginChild("camera-atts", attsSize, 0);
	{
		ImGui::Text("camera");
		std::vector<JObject*> camV({ GetSceneObjectPointer(camera()) });
		DrawValue<XMFLOAT3, jedv_t_float3_angle>()("rotation", camV);
		DrawValue<float, jedv_t_float>()("modelDistanceScale", camV);

		ImGui::Text("floor");
		std::vector<JObject*> floorV({ GetSceneObjectPointer(floor()) });
		DrawValue<XMFLOAT3, jedv_t_color_float3>()("floorColor", floorV);
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawTimelineController(ImVec2 curPos, ImVec2 size, Sequence& sequence)
{
	float nFramesWidth = ImGui::CalcTextSize("#frames").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float inputNumFramesWidth = 100.0f;
	float fpsWidth = ImGui::CalcTextSize("FPS").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float inputFPSWidth = 100;
	float playPauseButtonWidth = ImGui::CalcTextSize(ICON_FA_PLAY).x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float stopButtonWidth = ImGui::CalcTextSize(ICON_FA_STOP).x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float loopWidth = ImGui::CalcTextSize("loop").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float loopCheckboxWidth = 30.0f;
	float timeWidth = ImGui::CalcTextSize(std::string(std::string("time:") + std::format("{:.2f}", renderable->animationTime() / 1000.0f) + "s").c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + 30.0f;
	float frameWidth = ImGui::CalcTextSize(std::string(std::string("frame:") + std::to_string(renderable->animationFrame())).c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + 30.0f;
	float animationWidth = ImGui::CalcTextSize(std::string(std::string("animation:") + std::string((renderable->animation() != "") ? renderable->animation() : "#none")).c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + 30.0f;
	float total_width =
		nFramesWidth
		+ inputNumFramesWidth
		+ fpsWidth
		+ inputFPSWidth
		+ ImGui::GetStyle().ItemSpacing.x
		+ playPauseButtonWidth
		+ stopButtonWidth
		+ loopCheckboxWidth
		+ timeWidth
		+ frameWidth
		+ animationWidth
		;


	float window_width = ImGui::GetContentRegionAvail().x;
	//float start_x = curPos.x + (window_width - total_width) * 0.5f;
	ImVec2 start(curPos.x + (window_width - total_width) * 0.5f, curPos.y);

	//ImGui::SetCursorScreenPosX(start_x);
	ImGui::SetCursorScreenPos(start);

	ImGui::Text("#frames");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputNumFramesWidth);
	ImGui::PushID("TimeControllerTotalFrames");
	if (ImGui::InputInt("##", &sequence.totalFrames, 1, 100))
	{
		sequence.totalFrames = std::max(sequence.totalFrames, 1);
	}
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::Text("FPS");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputFPSWidth);
	ImGui::PushID("TimeControllerFramesPerSecond");
	if (ImGui::InputInt("##", &sequence.framesPerSecond, 1, 100))
	{
		sequence.framesPerSecond = std::max(sequence.framesPerSecond, 1);
	}
	ImGui::PopID();

	ImGui::SameLine();
	if (!renderable->animationPlay())
	{
		if (ImGui::Button(ICON_FA_PLAY))
		{
			if (renderable->animationTimeFactor() == 0.0f)
			{
				//auto it = animationsSequences.sequences.find(selectedSequence);
				//bool isAnim = it != animationsSequences.sequences.end();
				//renderable->SetCurrentAnimation(isAnim ? &(it->second) : nullptr, 0.0f, 1.0f, false, renderable->animationLoop());

			}
			renderable->animationPlay(true);
		}
	}
	else
	{
		if (ImGui::Button(ICON_FA_PAUSE))
		{
			renderable->animationPlay(false);
		}
	}

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_STOP))
	{
		renderable->animationPlay(false);
		renderable->animationTimeFactor(0.0f);
		renderable->animationTime(0.0f);
	}

	bool loop = renderable->animationLoop();
	ImGui::SameLine();
	if (ImGui::Checkbox("loop", &loop))
	{
		renderable->animationLoop(loop);
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth(timeWidth);
	ImGui::Text(std::string(std::string("time:") + std::format("{:.2f}", renderable->animationTime() / 1000.0f) + "s").c_str());

	ImGui::SameLine();
	ImGui::SetNextItemWidth(frameWidth);
	ImGui::Text(std::string(std::string("frame:") + std::to_string(renderable->animationFrame())).c_str());

	ImGui::SameLine();
	ImGui::SetNextItemWidth(animationWidth);
	ImGui::Text(std::string(std::string("animation:") + std::string((renderable->animation() != "") ? renderable->animation() : "#none")).c_str());

	if (renderable->animationPlay() && renderable->animationTimeFactor() > 0.0f)
	{
		//currentFrame = renderable->GetCurrentAnimationFrame();
	}
}

inline ImU32 rgba(auto r, auto g, auto b, auto a)
{
	return IM_COL32(
		static_cast<unsigned int>(r),
		static_cast<unsigned int>(g),
		static_cast<unsigned int>(b),
		static_cast<unsigned int>(a * 255)
	);
}

static ImVec2 frameSize(10.0f, 18.0f);
static ImVec2 frameSizeExpanded(10.0f, 100.0f);
static unsigned int framesBetweenTexts = 5;
static ImU32 frameMarkerTextColor = rgba(74, 74, 74, 1);
static ImU32 frameColor = rgba(255, 255, 255, 1);
static ImU32 frame5Color = rgba(239, 239, 239, 1);
static ImU32 frameMouseOverColor = rgba(221, 170, 195, 1);
static ImU32 frameBorderColor = rgba(229, 229, 229, 1);
static ImU32 frameWithElementColor = rgba(148, 148, 148, 1);
static ImU32 frameWithElementBorderColor = rgba(43, 43, 43, 1);
static ImU32 frameCircleColor = rgba(43, 43, 43, 1);

static float headerHeight = 16.0f;
static ImU32 headerColor = rgba(216, 216, 216, 1);
static ImU32 headerFrameSelectedColor = rgba(203, 73, 136, 0.3);
static float headerMarkerHeight = 4.0f;
static ImU32 headerMarkerColor = rgba(129, 129, 129, 1);
static ImU32 headerAddChannelButtonLines = rgba(32, 32, 32, 1);

static ImU32 scrollbarBgColor = rgba(81, 75, 165, 1);

static ImU32 channelsBgColor = rgba(255, 255, 255, 1);
static ImU32 columnSeparatorColor = rgba(27, 28, 26, 1);
static float channelsColumnW = 200.0f;
static ImU32 channelsNameTextColor = rgba(20, 20, 20, 1);
static ImU32 channelsActionColumnColor = rgba(118, 119, 110, 1);

static ImU32 timelineBgColor = rgba(198, 198, 198, 1);

void AnimationSequencerModal::DrawSequencer(ImVec2 pos, ImVec2 size, Sequence& sequence)
{
	float scrollbarSize = ImGui::GetStyle().ScrollbarSize;
	int deleteChannel = -1;
	int expandChannel = -1;
	size.x -= 5.0f;

	auto drawRect = [](ImVec2 pos, ImVec2 size, ImU32 color)
		{
			ImVec2 p1(pos);
			ImVec2 p2(pos.x + size.x, pos.y + size.y);
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRectFilled(p1, p2, color);
		};
	auto drawBackground = [drawRect](ImVec2 pos, ImVec2 size)
		{
			ImVec2 channelsToolbarBgPos(pos.x, pos.y);
			ImVec2 channelsToolbarBgSize(channelsColumnW, headerHeight);
			drawRect(channelsToolbarBgPos, channelsToolbarBgSize, headerColor);

			ImVec2 channelsBgPos(pos.x, pos.y + headerHeight);
			ImVec2 channelsBgSize(channelsColumnW, size.y - headerHeight);
			drawRect(channelsBgPos, channelsBgSize, channelsBgColor);

			ImVec2 timelineBgPos(pos.x + channelsColumnW + 1, pos.y + headerHeight);
			ImVec2 timelineBgSize(size.x - channelsColumnW, size.y - headerHeight);
			drawRect(timelineBgPos, timelineBgSize, timelineBgColor);

			ImVec2 timelineHeaderBgPos(pos.x + channelsColumnW + 1, pos.y);
			ImVec2 timelineHeaderBgSize(size.x - channelsColumnW, headerHeight);
			drawRect(timelineHeaderBgPos, timelineHeaderBgSize, headerColor);
		};
	auto hasScrollbarX = [](float sizeX, Sequence& sequence)
		{
			return sizeX < static_cast<float>(sequence.totalFrames) * frameSize.x;
		};
	auto getFirstVisibleFrame = [](float scrollX)
		{
			return static_cast<unsigned int>(std::floor(scrollX / frameSize.x));
		};
	auto getLastVisibleFrame = [&sequence](unsigned int firstFrame, ImVec2 size)
		{
			return std::min(firstFrame + static_cast<unsigned int>(std::ceil(size.x / frameSize.x)), static_cast<unsigned int>(sequence.totalFrames));
		};
	auto getFirstVisibleChannel = [this, &sequence](float scrollY)
		{
			//float y = -scrollY;
			//for (int i = 0; i < sequence.sequenceChannels.size(); i++)
			//{
			//	y += expandedChannels.at(i) ? frameSizeExpanded.y : frameSize.y;
			//	if (y > 0.0f)
			//		return i;
			//}
			//return 0;
			return static_cast<unsigned int>(std::floor(scrollY / frameSize.y));
		};
	//auto getLastVisibleChannel = [this, &sequence](float scrollY, ImVec2 size)
	//	{
	//		float y = -scrollY;
	//		for (int i = 0; i < sequence.sequenceChannels.size(); i++)
	//		{
	//			y += expandedChannels.at(i) ? frameSizeExpanded.y : frameSize.y;
	//			if (y > size.y)
	//				return i + 1;
	//		}
	//		return static_cast<int>(sequence.sequenceChannels.size());
	//	};
	auto getLastVisibleChannel = [this, &sequence](unsigned int firstChannel, ImVec2 size)
		{
			return firstChannel + static_cast<unsigned int>(std::ceil(size.y / frameSize.y));
		};
	auto rightClickInteractWithFrame = [this, &sequence](unsigned int seqChannelId, unsigned int frame, ImVec2 mouse)
		{
			selectedTimelineChannelId = seqChannelId;
			selectedTimelineFrameId = frame;
			int elementId;
			selectedElement = GetElementInFrame(sequence, selectedTimelineChannelId, selectedTimelineFrameId, elementId);
			if (elementId == -1)
			{
				popup = SMP_AddElement;
				popupCoords = mouse;
				BuildAnimationElementsTimelinePopup();
			}
			else
			{
				popup = SMP_InteractWithElement;
				popupCoords = mouse;
			}
		};
	auto leftClickInteractWithFrame = [this](unsigned int seqChannelId, unsigned int frame)
		{
			selectedTimelineChannelId = seqChannelId;
			selectedTimelineFrameId = frame;
		};
	auto selectElementInFrame = [this, &sequence](unsigned int seqChannelId, unsigned int frame)
		{
			int elementId;
			selectedElement = GetElementInFrame(sequence, seqChannelId, frame, elementId);
			if (selectedElement != nullptr)
			{
				selectedChannel = &sequence.sequenceChannels.at(seqChannelId);
				selectedElementChannelId = seqChannelId;
				selectedElementElementId = elementId;
			}
			else
			{
				selectedChannel = nullptr;
				selectedElementChannelId = -1;
				selectedElementElementId = -1;
			}
		};
	auto hasScrollbarY = [](float sizeY, Sequence& sequence)
		{
			return sizeY < static_cast<float>(sequence.sequenceChannels.size()) * frameSize.y;
		};
	auto drawScrollbar = [this](ImVec2 pos, ImVec2 size, float thumbSize, bool vertical = true)
		{
			//get the axis we are working 
			float* ratio = vertical ? &scrollbarRatio.y : &scrollbarRatio.x;
			float* mousePos = vertical ? &scrollbarLastMousePos.y : &scrollbarLastMousePos.x;
			float axisSize = vertical ? size.y : size.x;
			bool* mouseClicked = &scrollbarMouseClicked[vertical ? 0 : 1];

			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			//Draw the scrollbar track
			ImRect trackRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
			draw_list->AddRectFilled(trackRect.Min, trackRect.Max, scrollbarBgColor);

			//set the thumb rect depending of vertical or horizontal
			ImVec2 thumbPos0;
			ImVec2 thumbPos1;
			if (vertical)
			{
				thumbPos0 = ImVec2(pos.x, pos.y + (size.y - thumbSize) * (*ratio));
				thumbPos1 = ImVec2(thumbPos0.x + size.x, thumbPos0.y + thumbSize);
			}
			else
			{
				thumbPos0 = ImVec2(pos.x + (size.x - thumbSize) * (*ratio), pos.y);
				thumbPos1 = ImVec2(thumbPos0.x + thumbSize, thumbPos0.y + size.y);
			}
			//draw the thumb
			draw_list->AddRectFilled(thumbPos0, thumbPos1, IM_COL32(150, 150, 150, 255));

			//handle the mouse movement
			ImRect thumbRect(thumbPos0, thumbPos1);
			ImGuiIO& io = ImGui::GetIO();

			auto scroll = [ratio](float qty)
				{
					*ratio += qty;
					*ratio = std::clamp((*ratio), 0.0f, 1.0f);
				};

			if (io.MouseWheel != 0.0f && trackRect.Contains(io.MousePos))
			{
				scroll(-io.MouseWheel * 0.1f);
			}

			if (ImGui::IsMouseDown(0) && (thumbRect.Contains(io.MousePos) || *mouseClicked))
			{
				float mouseAxisPos = vertical ? io.MousePos.y : io.MousePos.x;
				if (*mouseClicked)
				{
					//adjust proportional to the movement to the actual available scrollable size
					float diff = (mouseAxisPos - *mousePos) / (axisSize - thumbSize);
					scroll(diff);
				}
				*mousePos = mouseAxisPos;
				*mouseClicked = true;
			}
			else if (*mouseClicked && !ImGui::IsMouseDown(0))
			{
				*mouseClicked = false;
			}
		};
	auto drawHeaderMarkers = [this, getFirstVisibleFrame, getLastVisibleFrame](ImVec2 pos, ImVec2 clipSize, float scrollX, Sequence& sequence)
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			ImVec2 clipMin(pos.x, pos.y - clipSize.y);
			ImVec2 clipMax(pos.x + clipSize.x, pos.y);
			ImGui::PushClipRect(clipMin, clipMax, true);

			//so we don't render more than we should we get the first visible frame and last visible(or close)
			unsigned int initialFrame = getFirstVisibleFrame(scrollX);
			unsigned int lastFrame = getLastVisibleFrame(initialFrame, clipSize);

			//xpos of the line markers uses scroll but are adjusted to the initial frame position
			float xpos = pos.x - scrollX + static_cast<float>(initialFrame) * frameSize.x;
			float y0 = pos.y;
			float y1 = y0 - headerMarkerHeight;
			for (unsigned int i = initialFrame; i <= lastFrame; i++)
			{
				draw_list->AddLine(ImVec2(xpos, y0), ImVec2(xpos, y1), headerMarkerColor);
				xpos += frameSize.x;
			}

			ImGui::PushStyleColor(ImGuiCol_Text, frameMarkerTextColor);

			//we get the frame texts for only the visible ones, only frames divisible by 5 and 1
			//get get the number of markers and get the offset of the markers in the timeline
			std::vector<std::string> markersTexts;
			unsigned int numMarkers = (lastFrame - initialFrame) / 5;
			unsigned int markerOffsetX = static_cast<unsigned int>(std::floor(initialFrame / 5.0f) * 5.0f);
			for (unsigned int i = 0; i <= numMarkers; i++)
			{
				unsigned int frame = i * 5 + markerOffsetX;
				markersTexts.push_back((frame == 0U) ? "1" : std::to_string(frame));
			}

			//draw the numbers and adjust the scrolling by this offset
			float textHeight = ImGui::CalcTextSize("0").y;
			ImVec2 textPos(pos.x - scrollX + markerOffsetX * frameSize.x, y1 - textHeight + 2);
			for (int i = 0; i < markersTexts.size(); i++)
			{
				ImGui::SetCursorScreenPos(textPos);
				ImGui::Text(markersTexts.at(i).c_str());
				textPos.x += framesBetweenTexts * frameSize.x;
			}

			ImGui::PopStyleColor();
			ImGui::PopClipRect();
		};
	auto drawChannelButton = [](ImVec2 pos, ImVec2 size, bool add = true)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent background
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.5f)); // Subtle hover effect
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 0.7f)); // Subtle active effect
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // White border
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f); // 1 pixel border
			ImGui::SetCursorScreenPos(pos);
			bool ret = false;
			ret = ImGui::Button("##", size);
			ImGui::PopStyleVar(); // Pop FrameBorderSize
			ImGui::PopStyleColor(5); // Pop the four style colors

			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			ImVec2 h0(pos.x, pos.y + size.y * 0.5f - .5f);
			ImVec2 h1(pos.x + size.x, pos.y + size.y * 0.5f - .5f);
			draw_list->AddLine(h0, h1, headerAddChannelButtonLines);

			if (add)
			{
				ImVec2 v0(pos.x + size.x * 0.5f - .5f, pos.y);
				ImVec2 v1(pos.x + size.x * 0.5f - .5f, pos.y + size.y);
				draw_list->AddLine(v0, v1, headerAddChannelButtonLines);
			}

			return ret;
		};
	auto drawChannelExpand = [](ImVec2 pos, ImVec2 size, bool expanded, std::function<void(bool)> toggle)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent background
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.5f)); // Subtle hover effect
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 0.7f)); // Subtle active effect
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // White border
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.5f); // 1 pixel border
			ImGui::SetCursorScreenPos(pos);
			bool ret = false;
			//ret = ImGui::Button("##", size);
			if (!expanded)
			{
				ret = ImGui::Button("##", size);
			}
			else
			{
				ret = ImGui::Button("##", size);
			}


			ImVec2 textPos(pos);
			textPos.y -= 3.0f;
			textPos.x += 2.0f;
			ImGui::SetCursorScreenPos(textPos);
			ImGui::Text(expanded ? "c" : "e");

			ImGui::PopStyleVar(); // Pop FrameBorderSize
			ImGui::PopStyleColor(5); // Pop the four style colors

			if (ret)
			{
				toggle(!expanded);
			}

			return ret;
		};
	auto drawChannelToolbar = [this, drawChannelButton, &deleteChannel, drawChannelExpand, &expandChannel](ImVec2 pos, ImVec2 size, unsigned int seqChannelId, std::string& channelName)
		{
			std::vector<ImVec2> titlePos = {
				ImVec2(pos.x,pos.y),
				ImVec2(pos.x + size.x,pos.y),
				ImVec2(pos.x + size.x,pos.y + size.y),
				ImVec2(pos.x,pos.y + size.y),
			};
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddPolyline(titlePos.data(), static_cast<unsigned int>(titlePos.size()), columnSeparatorColor, 0, 1.0f);

			ImVec2 nameSize = ImGui::CalcTextSize(channelName.c_str());
			ImVec2 namePos = ImVec2(pos.x, pos.y);
			ImGui::SetCursorScreenPos(namePos);
			std::string name = channelName;
			std::string inputId = "seqChannel" + std::to_string(seqChannelId);
			float inputWidth = size.x - 40;
			ImGui::PushID(inputId.c_str());
			ImGui::PushItemWidth(inputWidth);

			ImGui::PushStyleColor(ImGuiCol_Text, channelsNameTextColor);
			if (ImGui::InputText("##", &name))
			{
				channelName = name;
			}
			ImGui::PopStyleColor();
			ImGui::PopItemWidth();
			ImGui::PopID();

			ImVec2 deleteChannelBtnSize(headerHeight - 6, headerHeight - 6);
			ImVec2 deleteChannelBtnPos(pos.x + inputWidth + 0.5f * (size.x - inputWidth - deleteChannelBtnSize.x), pos.y + 0.5f * (size.y - deleteChannelBtnSize.y));

			std::string deleteChannelButtonId = "DeleteChannelButton" + std::to_string(seqChannelId);
			ImGui::PushID(deleteChannelButtonId.c_str());
			if (drawChannelButton(deleteChannelBtnPos, deleteChannelBtnSize, false))
			{
				deleteChannel = seqChannelId;
			}
			ImGui::PopID();

			std::string expandChannelButtonId = "ExpandChannelButton" + std::to_string(seqChannelId);
			ImGui::PushID(expandChannelButtonId.c_str());
			ImVec2 expandChannelBtnSize(deleteChannelBtnSize);
			ImVec2 expandChannelBtnPos(deleteChannelBtnPos.x + 2 + deleteChannelBtnSize.x, deleteChannelBtnPos.y);
			drawChannelExpand(expandChannelBtnPos, expandChannelBtnSize, expandedChannels.at(seqChannelId), [seqChannelId, &expandChannel](bool state)
				{
					expandChannel = seqChannelId;
				}
			);
			ImGui::PopID();
		};
	auto drawChannelFrame = [this, rightClickInteractWithFrame, leftClickInteractWithFrame, selectElementInFrame](SequenceChannel& seqChannel, unsigned int seqChannelId, unsigned int frame, ImVec2 p0, ImVec2 p1, ImU32 color, ImRect timelineRect, ImVec2 timelineSize)
		{
			ImGuiIO& io = ImGui::GetIO();
			ImVec2 mouse = io.MousePos;
			ImRect thumbRect(p0, p1);

			bool mouseInFrame = timelineRect.Contains(mouse) && thumbRect.Contains(mouse) && popup == SMP_None;

			bool selectedFrameAndChannel = selectedTimelineChannelId == seqChannelId && frame == selectedTimelineFrameId;
			bool elementBoundFromLeft = false;
			bool elementBoundFromRight = false;
			bool frameHasElement = std::any_of(seqChannel.elements.begin(), seqChannel.elements.end(), [&elementBoundFromLeft, &elementBoundFromRight, frame](ChannelElement& elem)
				{
					return elem.ElementInFrame(frame, elementBoundFromLeft, elementBoundFromRight);
				}
			);

			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			if (!frameHasElement)
			{
				draw_list->AddRectFilled(p0, p1, (mouseInFrame || selectedFrameAndChannel) ? frameMouseOverColor : color);
				draw_list->AddRect(p0, p1, frameBorderColor, 0.0f, 0, 0.5f);
			}
			else
			{
				draw_list->AddRectFilled(p0, p1, (mouseInFrame || selectedFrameAndChannel) ? frameMouseOverColor : frameWithElementColor);

				draw_list->AddLine(ImVec2(p0.x, p0.y), ImVec2(p1.x, p0.y), frameWithElementBorderColor);
				draw_list->AddLine(ImVec2(p0.x, p1.y), ImVec2(p1.x, p1.y), frameWithElementBorderColor);
				if (elementBoundFromLeft)
				{
					draw_list->AddLine(ImVec2(p0.x, p0.y), ImVec2(p0.x, p1.y), frameWithElementBorderColor);
					ImVec2 center(p0.x + (p1.x - p0.x) * 0.5f,
						p0.y + (p1.y - p0.y) * 0.6f
					);
					float radius = (p1.x - p0.x) / 4.0f;
					draw_list->AddCircleFilled(center, radius, frameCircleColor, 10);
				}
				if (elementBoundFromRight)
				{
					draw_list->AddLine(ImVec2(p1.x, p0.y), ImVec2(p1.x, p1.y), frameWithElementBorderColor);
				}
			}

			if (ImGui::IsMouseDown(0) && mouseInFrame)
			{
				if (!timelineLeftClickPressed)
				{
					timelineLeftClickPressed = true;
					timelineLeftClickLastCoords = mouse;
					timelineLeftClickXsum = 0.0f;
					leftClickInteractWithFrame(seqChannelId, frame);
					if (frameHasElement)
					{
						selectElementInFrame(seqChannelId, frame);
					}
				}
			}

			if (ImGui::IsMouseDown(1) && mouseInFrame)
			{
				rightClickInteractWithFrame(seqChannelId, frame, mouse);
			}
		};
	auto drawChannelTimeline = [getFirstVisibleFrame, getLastVisibleFrame, drawChannelFrame](ImVec2 pos, ImRect timelineRect, ImVec2 timelineSize, ImVec2 scroll, unsigned int totalFrames, SequenceChannel& seqChannel, unsigned int seqChannelId)
		{
			//so we don't render more than we should we get the first visible frame and last visible(or close)
			unsigned int initialFrame = getFirstVisibleFrame(scroll.x);
			unsigned int lastFrame = getLastVisibleFrame(initialFrame, timelineSize);

			ImVec2 framePos(pos.x - scroll.x + static_cast<float>(initialFrame) * frameSize.x, pos.y - scroll.y);

			for (unsigned int i = initialFrame; i <= lastFrame; i++)
			{
				ImVec2 p0(framePos.x, framePos.y);
				ImVec2 p1(framePos.x + frameSize.x, framePos.y + frameSize.y);
				drawChannelFrame(seqChannel, seqChannelId, i, p0, p1, (i % 5 != 0 || i == 0) ? frameColor : frame5Color, timelineRect, timelineSize);
				framePos.x += frameSize.x;
			}
		};
	auto drawChannels = [this, getFirstVisibleChannel, getLastVisibleChannel, drawChannelTimeline, drawChannelToolbar, scrollbarSize](ImVec2 pos, ImVec2 size, ImVec2 scroll, Sequence& sequence)
		{
			ImVec2 channelPos(pos.x, pos.y);
			ImVec2 channelTimelineSize(size.x - channelsColumnW - 1, frameSize.y);

			ImVec2 clipMin;
			ImVec2 clipMax;

			//get the first and last channel that can be rendered in the bounds of the timeline
			unsigned int initialChannel = getFirstVisibleChannel(scroll.y);
			//unsigned int lastChannel = std::min(1U + getLastVisibleChannel(scroll.y, size), static_cast<unsigned int>(sequence.sequenceChannels.size()));
			unsigned int lastChannel = std::min(1U + getLastVisibleChannel(initialChannel, size), static_cast<unsigned int>(sequence.sequenceChannels.size()));

			//set a clip area of the renderable rect
			clipMin = ImVec2(pos.x, pos.y);
			clipMax = ImVec2(pos.x + channelsColumnW, pos.y + size.y);
			ImGui::PushClipRect(clipMin, clipMax, true);
			{
				//make the toolbar position to be adjusted to the initialChannel position
				ImVec2 channelTitleSize(channelsColumnW - 1, frameSize.y);
				ImVec2 toolbarPos(pos.x, pos.y - scroll.y + static_cast<float>(initialChannel) * frameSize.y);
				for (unsigned int i = initialChannel; i < lastChannel; i++)
				{
					SequenceChannel& seqChannel = sequence.sequenceChannels.at(i);
					drawChannelToolbar(toolbarPos, channelTitleSize, i, seqChannel.name);
					toolbarPos.y += frameSize.y;
				}
			}
			ImGui::PopClipRect();

			clipMin = ImVec2(pos.x + channelsColumnW + 1, pos.y);
			clipMax = ImVec2(pos.x + size.x, pos.y + size.y);
			ImRect timelineRect(clipMin, clipMax);
			ImGui::PushClipRect(clipMin, clipMax, true);
			{
				ImVec2 timelinePos(pos.x + channelsColumnW + 1, pos.y + static_cast<float>(initialChannel) * frameSize.y);
				for (unsigned int i = initialChannel; i < lastChannel; i++)
				{
					SequenceChannel& seqChannel = sequence.sequenceChannels.at(i);
					drawChannelTimeline(timelinePos, timelineRect, channelTimelineSize, scroll, sequence.totalFrames, seqChannel, i);
					timelinePos.y += frameSize.y;
				}
			}
			ImGui::PopClipRect();
		};
	auto handleSelectedChannelDragAndDrop = [this, &sequence]()
		{
			ImGuiIO& io = ImGui::GetIO();
			ImVec2 mouse = io.MousePos;
			float dx = mouse.x - timelineLeftClickLastCoords.x;
			timelineLeftClickXsum += dx;
			timelineLeftClickLastCoords = mouse;
			float framesX = timelineLeftClickXsum / frameSize.x;
			if (std::fabs(framesX) >= 1.0f)
			{
				int frames = framesX >= 1.0f ? static_cast<int>(std::floor(framesX)) : static_cast<int>(std::ceil(framesX));
				selectedChannel->MoveElement(selectedElementElementId, frames, sequence.totalFrames);
				timelineLeftClickXsum = std::fmodf(timelineLeftClickXsum, frameSize.x);
			}
		};
	auto drawSelectedFrameVerticalLine = [this, getFirstVisibleFrame, getLastVisibleFrame](ImVec2 pos, ImVec2 clipSize, ImVec2 scroll)
		{
			unsigned int initialFrame = getFirstVisibleFrame(scroll.x);
			unsigned int lastFrame = getLastVisibleFrame(initialFrame, clipSize);

			if (selectedTimelineFrameId == -1 || !nostd::in_between(selectedTimelineFrameId, initialFrame, lastFrame)) return;

			//xpos of the line markers uses scroll but are adjusted to the initial frame position
			float xpos = pos.x - scroll.x + static_cast<float>(selectedTimelineFrameId) * frameSize.x;
			ImVec2 p0(xpos, pos.y);
			ImVec2 p1(xpos + frameSize.x, pos.y + clipSize.y);

			ImVec2 clipMin(pos.x, pos.y - clipSize.y);
			ImVec2 clipMax(pos.x + clipSize.x, pos.y);

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRectFilled(p0, p1, headerFrameSelectedColor);
		};
	auto handleHeaderMousePicking = [this, getFirstVisibleFrame, getLastVisibleFrame](ImVec2 pos, ImVec2 size, ImVec2 scroll)
		{
			if (headerLeftClickPressed) return;

			ImRect headerRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
			//ImDrawList* draw_list = ImGui::GetWindowDrawList();
			//draw_list->AddRectFilled(headerRect.Min, headerRect.Max, headerFrameSelectedColor);

			ImGuiIO& io = ImGui::GetIO();

			if (ImGui::IsMouseDown(0) && headerRect.Contains(io.MousePos))
			{
				int frame = static_cast<int>(std::floor((scroll.x + (io.MousePos.x - pos.x)) / frameSize.x));
				selectedTimelineFrameId = frame;
				headerLeftClickPressed = true;
				headerLeftClickLastCoords = io.MousePos;
				headerLeftClickXsum = 0.0f;
			}
		};
	auto setFrameAtMouseXCoord = [this](ImVec2 mouse, ImVec2 pos, ImVec2 scroll, int totalFrames)
		{
			selectedTimelineFrameId = std::clamp(static_cast<int>(std::floor((mouse.x - pos.x + scroll.x) / frameSize.x)), 0, totalFrames);
		};
	auto scrollAndSetFrame = [this, setFrameAtMouseXCoord](ImVec2 mouse, ImVec2 pos, ImVec2 scroll, float mouseDiff, float axisSize, float thumbSize, int totalFrames)
		{
			float* ratio = &scrollbarRatio.x;
			auto move = [ratio](float qty)
				{
					*ratio += qty;
					*ratio = std::clamp((*ratio), 0.0f, 1.0f);
				};
			float diff = (mouseDiff) / (axisSize - thumbSize);
			move(diff);
			setFrameAtMouseXCoord(mouse, pos, scroll, totalFrames);
		};
	auto handleHeaderMouseMovement = [this, setFrameAtMouseXCoord, scrollAndSetFrame](ImVec2 pos, ImVec2 size, ImVec2 scroll, float thumbWidth, int totalFrames)
		{
			ImRect headerRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
			ImVec2 mouseMinMax(pos.x + size.x * 0.25f, pos.x + size.x * 0.75f);

			if (!ImGui::IsMouseDown(0))
			{
				headerLeftClickPressed = false;
				return;
			}

			ImGuiIO& io = ImGui::GetIO();
			ImVec2 mouse = io.MousePos;

			if (nostd::in_between(mouse.x, mouseMinMax.x, mouseMinMax.y))
			{
				setFrameAtMouseXCoord(mouse, pos, scroll, totalFrames);
			}
			else if (mouse.x > mouseMinMax.y)
			{
				scrollAndSetFrame(mouse, pos, scroll, 0.1f * (mouse.x - mouseMinMax.y), size.x, thumbWidth, totalFrames);
			}
			else if (mouse.x < mouseMinMax.x)
			{
				scrollAndSetFrame(mouse, pos, scroll, 0.1f * (mouse.x - mouseMinMax.x), size.x, thumbWidth, totalFrames);
			}
		};

	//draw the timeline
	drawBackground(pos, size);

	ImVec2 addChannelButtonPos(pos.x + 5, pos.y + 2);
	ImVec2 addChannelButtonSize(headerHeight - 6, headerHeight - 6);
	ImGui::PushID("AddChannelButton");
	if (drawChannelButton(addChannelButtonPos, addChannelButtonSize))
	{
		std::string name = "Channel " + std::to_string(1 + sequence.sequenceChannels.size());
		sequence.sequenceChannels.push_back(SequenceChannel(name));
	}
	ImGui::PopID();

	bool verticalScrollbar = hasScrollbarY(size.y, sequence);
	bool horizontalScrollbar = hasScrollbarX(size.x - channelsColumnW - scrollbarSize, sequence);
	float vThumbHeight = 0.0f;
	float hThumbWidth = 0.0f;

	if (verticalScrollbar)
	{
		ImVec2 vScrollbarPos = ImVec2(pos.x + size.x - scrollbarSize + 1, pos.y + headerHeight);
		ImVec2 vScrollbarSize = ImVec2(scrollbarSize, size.y - headerHeight);
		vThumbHeight = (std::floor(vScrollbarSize.y / frameSize.y)) * (vScrollbarSize.y / static_cast<float>(sequence.sequenceChannels.size()));
		drawScrollbar(vScrollbarPos, vScrollbarSize, vThumbHeight, true);
	}
	if (horizontalScrollbar)
	{
		ImVec2 hScrollbarPos = ImVec2(pos.x + channelsColumnW + 1, pos.y + size.y);
		ImVec2 hScrollbarSize = ImVec2(size.x - channelsColumnW - scrollbarSize, scrollbarSize);
		hThumbWidth = (std::floor(hScrollbarSize.x / frameSize.x)) * (hScrollbarSize.x / static_cast<float>(sequence.totalFrames));
		drawScrollbar(hScrollbarPos, hScrollbarSize, hThumbWidth, false);
	}

	ImVec2 channelsPos(pos.x, pos.y + headerHeight);
	ImVec2 channelsSize(size.x - scrollbarSize, size.y - headerHeight);
	ImVec2 channelsScroll(
		scrollbarRatio.x * (((sequence.totalFrames + 3) * frameSize.x) - (size.x - channelsColumnW)),
		scrollbarRatio.y * ((sequence.sequenceChannels.size() * frameSize.y) - channelsSize.y)
	);

	drawChannels(channelsPos, channelsSize, channelsScroll, sequence);

	ImVec2 headersMarkersPos(pos.x + channelsColumnW, pos.y + headerHeight);
	ImVec2 headersMarkersClipSize(size.x - channelsColumnW, headerHeight);
	drawHeaderMarkers(headersMarkersPos, headersMarkersClipSize, channelsScroll.x, sequence);

	ImVec2 headerMousePickingPos(pos.x + channelsColumnW, pos.y);
	ImVec2 headerMousePickingSize(size.x - channelsColumnW, headerHeight);
	handleHeaderMousePicking(headerMousePickingPos, headerMousePickingSize, channelsScroll);

	drawSelectedFrameVerticalLine(ImVec2(headersMarkersPos.x, headersMarkersPos.y - headerHeight), ImVec2(channelsSize.x, channelsSize.y + headerHeight), channelsScroll);

	if (timelineLeftClickPressed && selectedChannel != nullptr)
	{
		if (ImGui::IsMouseDown(0))
		{
			handleSelectedChannelDragAndDrop();
		}
		else
		{
			timelineLeftClickPressed = false;
			timelineLeftClickXsum = 0.0f;
			selectedElement = nullptr;
			selectedChannel = nullptr;
			selectedElementChannelId = -1;
			selectedElementElementId = -1;
		}
	}
	else if (headerLeftClickPressed)
	{
		if (ImGui::IsMouseDown(0))
		{
			handleHeaderMouseMovement(headerMousePickingPos, headerMousePickingSize, channelsScroll, hThumbWidth, sequence.totalFrames);
		}
		else
		{
			headerLeftClickPressed = false;
		}
	}
	else if (!ImGui::IsMouseDown(0))
	{
		headerLeftClickPressed = false;
		timelineLeftClickPressed = false;
		timelineLeftClickXsum = 0.0f;
		selectedElement = nullptr;
		selectedChannel = nullptr;
		selectedElementChannelId = -1;
		selectedElementElementId = -1;
	}

	if (deleteChannel != -1)
	{
		nostd::vector_erase_index(sequence.sequenceChannels, deleteChannel);
	}

	if (expandChannel != -1)
	{
		expandedChannels[expandChannel] = !expandedChannels[expandChannel];
	}
}

void AnimationSequencerModal::DrawSaveAndExitButtons(ImVec2 curPos, ImVec2 size, bool& exit, bool& saveexit)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	ImGui::SetNextWindowPos(curPos, 0);
	ImGui::SetNextWindowSize(size, 0);
	ImGui::BeginChild("saveexit", size, 0);
	{
		ImGui::DrawItemWithEnabledState([this, &saveexit]
			{
				if (ImGui::Button("Save & Exit"))
				{
					saveexit = true;
				}
			}
		, true);

		ImGui::SameLine();
		if (ImGui::Button("Exit"))
		{
			exit = true;
		}
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawAddNewSequencePopup(ImVec2 curPos, ImVec2 size, std::string& newSeqName, std::function<void(std::string)> onAddNewSequenceClicked, std::function<void()> onCancelAddNewSequenceClick)
{
	std::set<std::string> existingSequences = { "" };
	for (auto& [seqName, _] : animationsSequences.sequences)
	{
		existingSequences.insert(seqName);
	}

	const char* title = "Add new sequence";
	ImGui::OpenPopup(title);

	ImGui::SetNextWindowPos(curPos);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	if (ImGui::BeginPopupModal(title, nullptr, popupChildFlag))
	{
		ImGui::SetNextItemWidth(size.x);
		ImGui::InputText("##", &newSeqName);

		float button1_width = ImGui::CalcTextSize("Add").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float button2_width = ImGui::CalcTextSize("Cancel").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float total_width = button1_width + button2_width + ImGui::GetStyle().ItemSpacing.x;

		float window_width = ImGui::GetContentRegionAvail().x;
		float start_x = (window_width - total_width) * 0.5f;

		ImGui::SetCursorPosX(start_x);

		ImGui::DrawItemWithEnabledState([this, newSeqName, onAddNewSequenceClicked]
			{
				if (ImGui::Button("Add"))
				{
					onAddNewSequenceClicked(newSeqName);
				}
			}
		, !existingSequences.contains(newSeqName));

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			onCancelAddNewSequenceClick();
			addNewSequence = false;
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::BuildAnimationElementsTimelinePopup()
{
	addElementAnimationSelectables = nostd::GetKeysFromMap(renderable->animable->animations->animationsLength);
	addElementAnimationSelectables.erase(addElementAnimationSelectables.begin());
	addElementAnimationSelected = *addElementAnimationSelectables.begin();
}

void AnimationSequencerModal::DrawAddNewElementToTimelinePopup()
{
	ImGui::OpenPopup(SequencerModalPopupToString.at(SMP_AddElement).c_str());

	ImVec2 size(200, 75);

	if (addElementType == SCET_Animation)
	{
		size.y += 20;
	}

	//ImGui::SetCursorScreenPos(popupCoords);
	ImGui::SetNextWindowPos(popupCoords);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	if (ImGui::BeginPopupModal(SequencerModalPopupToString.at(SMP_AddElement).c_str(), nullptr, popupChildFlag))
	{
		ImGui::SetNextItemWidth(size.x);

		std::vector<std::string> selectables = nostd::GetKeysFromMap(StrToSequenceChannelElementType);
		std::string selected = SequenceChannelElementTypeToStr.at(addElementType);
		bool changedToAnim = false;
		ImGui::PushID("NewElementType");
		ImGui::DrawComboSelection(selected, selectables, [this, &changedToAnim](std::string newElementType)
			{
				addElementType = StrToSequenceChannelElementType.at(newElementType);
				if (addElementType == SCET_Animation)
				{
					changedToAnim = true;
					BuildAnimationElementsTimelinePopup();
				}
			}
		);
		ImGui::PopID();

		if (addElementType == SCET_Animation && !changedToAnim)
		{
			ImGui::SetNextItemWidth(size.x);

			ImGui::PushID("ElementTypeAnimationName");
			ImGui::DrawComboSelection(addElementAnimationSelected, addElementAnimationSelectables, [this](std::string selectedAnim)
				{
					addElementAnimationSelected = selectedAnim;
				}
			);
			ImGui::PopID();
		}

		float button1_width = ImGui::CalcTextSize("Add").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float button2_width = ImGui::CalcTextSize("Cancel").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float total_width = button1_width + button2_width + ImGui::GetStyle().ItemSpacing.x;

		float window_width = ImGui::GetContentRegionAvail().x;
		float start_x = (window_width - total_width) * 0.5f;

		ImGui::SetCursorPosX(start_x);

		if (ImGui::Button("Add"))
		{
			OnAddAnimationToChannelClicked();
			popup = SMP_None;
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			OnCancelAddAnimationToChannelClicked();
			popup = SMP_None;
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
}

void AnimationSequencerModal::OnAddAnimationToChannelClicked()
{
	Sequence& sequence = animationsSequences.sequences.at(selectedSequence);
	float time = renderable->animable->animations->animationsLength.at(addElementAnimationSelected);
	unsigned int numFrames = static_cast<unsigned int>(std::ceil(time / sequence.framesPerSecond));
	unsigned int frameEnd = selectedTimelineFrameId + numFrames;

	int framesToFit = frameEnd - sequence.totalFrames - 1;
	if (framesToFit > 0)
	{
		sequence.totalFrames += framesToFit;
	}

	ChannelElement element;
	element.type = SCET_Animation;
	SequenceChannelElementAnimation& animation = element.animation;
	animation.frameStart = selectedTimelineFrameId;
	animation.frameEnd = frameEnd;
	animation.animation = addElementAnimationSelected;
	animation.framesToSkipFromLeft = 0;
	animation.framesToSkipFromRight = 0;

	SequenceChannel& seqChannel = sequence.sequenceChannels.at(selectedTimelineChannelId);
	seqChannel.elements.push_back(element);
}

void AnimationSequencerModal::OnCancelAddAnimationToChannelClicked()
{
}

void AnimationSequencerModal::DrawElementInteractPopup()
{
	ImGui::OpenPopup(SequencerModalPopupToString.at(SMP_InteractWithElement).c_str());

	ImVec2 size(200, 75);

	ImGui::SetNextWindowPos(popupCoords);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 2.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	if (ImGui::BeginPopupModal(SequencerModalPopupToString.at(SMP_InteractWithElement).c_str(), nullptr, popupChildFlag))
	{
		if (ImGui::MenuItem("Delete")) {
			DeleteElement();
			popup = SMP_None;
		}
		if (ImGui::MenuItem("Split")) {
			SplitElement();
			popup = SMP_None;
		}
		if (ImGui::MenuItem("Cancel"))
		{
			popup = SMP_None;
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
}

void AnimationSequencerModal::DeleteElement()
{
	int elementId;
	Sequence& sequence = animationsSequences.sequences.at(selectedSequence);
	GetElementInFrame(sequence, selectedTimelineChannelId, selectedTimelineFrameId, elementId);
	if (elementId == -1) return;

	SequenceChannel& seqChannel = sequence.sequenceChannels.at(selectedTimelineChannelId);
	seqChannel.EraseElement(elementId);

	selectedTimelineChannelId = -1;
	selectedTimelineFrameId = -1;
}

void AnimationSequencerModal::SplitElement()
{
	int elementId;
	Sequence& sequence = animationsSequences.sequences.at(selectedSequence);
	GetElementInFrame(sequence, selectedTimelineChannelId, selectedTimelineFrameId, elementId);
	if (elementId == -1) return;

	SequenceChannel& seqChannel = sequence.sequenceChannels.at(selectedTimelineChannelId);
	seqChannel.SplitElement(elementId, selectedTimelineFrameId);

	selectedTimelineChannelId = -1;
	selectedTimelineFrameId = -1;
}

void AnimationSequencerModal::Exit()
{
	destroying = true;
	showing = false;
}

void AnimationSequencerModal::SaveAndExit()
{
	destroying = true;
	showing = false;
	//model3D->animationSequences(animationsSequences);
	//model3D->flag(Model3DJson::Update_animationSequences);
	//Editor::templatesModified = true;
}

ChannelElement* AnimationSequencerModal::GetElementInFrame(Sequence& sequence, unsigned int seqChannelId, unsigned int frame, int& elementId)
{
	SequenceChannel& seqChannel = sequence.sequenceChannels.at(seqChannelId);
	int i = 0;
	for (auto& it : seqChannel.elements)
	{
		if (it.InFrame(frame))
		{
			elementId = i;
			return &it;
		}
		i++;
	}
	elementId = -1;
	return nullptr;
}


/*
void AnimationSequencerModal::OnCurrentFrameChanged(int frame)
{
	renderable->SetCurrentAnimationFrame(frame);
}
*/

/*
int AnimationSequencerModal::GetFrameMin() const
{
	return 0;
}
*/

/*
int AnimationSequencerModal::GetFrameMax() const
{
	return animationsSequences.sequences.at(selectedSequence).totalFrames;
}
*/

/*
int AnimationSequencerModal::GetItemCount() const
{
	return (int)animationsSequences.sequences.at(selectedSequence).channels.size();
}
*/

/*
int AnimationSequencerModal::GetItemTypeCount() const
{
	return static_cast<int>(animations.size());
}
*/

/*
const char* AnimationSequencerModal::GetItemTypeName(int typeIndex) const
{
	return animations.at(typeIndex).c_str();
}
*/

/*
const char* AnimationSequencerModal::GetItemLabel(int index) const
{
	static char tmps[512];
	snprintf(tmps, 512, "[%02d] %s", index, animationsSequences.sequences.at(selectedSequence).channels.at(index).animation.c_str());
	return tmps;
}
*/

/*
void AnimationSequencerModal::Get(int index, int** start, int** end, int* type, unsigned int* color)
{
	auto& item = animationsSequences.sequences.at(selectedSequence).channels.at(index);
	if (color)
	{
		*color = 0xFFAA8080; // same color for everyone, return color based on type
	}
	if (start)
	{
		*start = &item.frameStart;
	}
	if (end)
	{
		*end = &item.frameEnd;
	}
	if (type)
	{
		*type = (int)item.type;
	}
}
*/

/*
void AnimationSequencerModal::Add(int type)
{
	SequenceChannel seqC;
	seqC.animation = animations.at(type);
	animationsSequences.sequences.at(selectedSequence).channels.push_back(seqC);
	expandedChannels.at(selectedSequence).push_back(false);
}
*/

/*
void AnimationSequencerModal::Del(int index) {
	if (animationsSequences.sequences.at(selectedSequence).channels.size() == 1ULL) return;
	animationsSequences.sequences.at(selectedSequence).channels.erase(animationsSequences.sequences.at(selectedSequence).channels.begin() + index);
	expandedChannels.at(selectedSequence).erase(expandedChannels.at(selectedSequence).begin() + index);
}
*/

/*
void AnimationSequencerModal::Duplicate(int index)
{
	SequenceChannel seqC = animationsSequences.sequences.at(selectedSequence).channels.at(index);
	animationsSequences.sequences.at(selectedSequence).channels.push_back(seqC);
	expandedChannels.at(selectedSequence).push_back(false);
}
*/

/*
size_t AnimationSequencerModal::GetCustomHeight(int index)
{
	return (expandedChannels.contains(selectedSequence) && expandedChannels.at(selectedSequence).at(index)) ? 300 : 0;
}
*/

/*
void AnimationSequencerModal::DoubleClick(int index) {
	if (expandedChannels.at(selectedSequence)[index])
	{
		expandedChannels.at(selectedSequence)[index] = false;
		return;
	}
	for (size_t i = 0ULL; i < expandedChannels.at(selectedSequence).size(); i++)
	{
		expandedChannels.at(selectedSequence)[i] = false;
	}
	expandedChannels.at(selectedSequence)[index] = !expandedChannels.at(selectedSequence)[index];
}
*/