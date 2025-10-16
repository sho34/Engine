#include "pch.h"
#include "AnimationSequencerModal.h"
#include <imgui.h>
#include <ImEditor.h>
#include <nlohmann/json.hpp>
#include <Camera/Camera.h>
#include <Light/Light.h>
#include <Renderable/Renderable.h>
#include <Renderer.h>

extern std::shared_ptr<Renderer> renderer;

namespace Editor
{
	extern bool templatesModified;
};

void AnimationSequencerModal::Initialize(std::string uuid)
{
	showing = true;
	initializing = true;
	model3dUUID = uuid;
	model3D = GetModel3DTemplate(uuid);
	animationsSequences = model3D->animationSequences();
}

void AnimationSequencerModal::LoadSceneObjects()
{
	std::string cameraUUID = getUUID();
	std::string ambLightUUID = getUUID();
	std::string dirLightUUID = getUUID();
	std::string animableUUID = getUUID();
	std::string floorUUID = getUUID();

	nlohmann::json cameraJson = {
		{ "fitWindow", false },
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
		{ "uuid", cameraUUID },
			{
				"renderPasses", { FindRenderPassUUIDByName("ModelPreviewPass")}
			},
		{ "mouseController", false },
		{ "useSwapChain", false }
	};

	nlohmann::json ambientLightJson =
	{
		{ "color", { 0.05000000074505806, 0.05000000074505806, 0.05000000074505806 } },
		{ "lightType", "Ambient" },
		{ "name", "light.0.amb-preview" },
		{ "uuid", ambLightUUID },
		{ "cameras", { cameraUUID } }
	};

	nlohmann::json directionalLightJson =
	{
		{ "color", { 1.0, 1.0, 1.0} },
		{ "farZ" , 1000.0},
		{ "nearZ", 0.009999999776482582},
		{ "hasShadowMaps", true },
		{ "rotation", {40.31087875366211, -0.30000039935112, 0.0} },
		{ "lightType", "Directional"},
		{ "name", "light.1.dir-preview"},
		{ "uuid", dirLightUUID },
		{ "zBias", 0.0002 },
		{ "cameras", { cameraUUID } }
	};

	nlohmann::json animableJson =
	{
		{ "castShadows", true },
		{ "model", model3dUUID },
		{ "name", "preview-model" },
		{ "position", { 0.0, 0.0, 0.0} },
		{ "rotation", { 0.0, -90.0, 0.0 }},
		{ "scale", { 0.1, 0.1, 0.1} },
		{ "uuid", animableUUID },
		{ "cameras", { cameraUUID } }
	};

	nlohmann::json floorJson =
	{
		{ "castShadows", false },
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
		{ "uuid", floorUUID },
		{ "cameras", { cameraUUID } }
	};

	camera = Scene::CreateSceneObjectFromJson<Scene::Camera>(cameraJson);
	ambientLight = Scene::CreateSceneObjectFromJson<Scene::Light>(ambientLightJson);
	directionalLight = Scene::CreateSceneObjectFromJson<Scene::Light>(directionalLightJson);
	renderable = Scene::CreateSceneObjectFromJson<Scene::Renderable>(animableJson);
	floor = Scene::CreateSceneObjectFromJson<Scene::Renderable>(floorJson);

	camera->BindToScene();
	ambientLight->BindToScene();
	directionalLight->BindToScene();
	renderable->BindToScene();
	floor->BindToScene();

	camera->UpdateProjection();

	expandedChannels.clear();
	for (auto& [seqName, sequence] : animationsSequences.sequences)
	{
		expandedChannels.insert_or_assign(seqName, std::vector<bool>(sequence.channels.size(), false));
	}

	animations = nostd::GetKeysFromMap(renderable->animable->animations->animationsLength);
	animations.erase(animations.begin());

	selectedSequence = "";
	initializing = false;
	addNewSequence = false;
	newSequenceName = "";
}

void AnimationSequencerModal::DestroySceneObjects()
{
	Scene::SafeDeleteSceneObject<Renderable>(renderable);
	Scene::SafeDeleteSceneObject<Renderable>(floor);
	Scene::SafeDeleteSceneObject<Light>(directionalLight);
	Scene::SafeDeleteSceneObject<Light>(ambientLight);
	Scene::SafeDeleteSceneObject<Camera>(camera);

	renderable = nullptr;
	floor = nullptr;
	directionalLight = nullptr;
	ambientLight = nullptr;
	camera = nullptr;
	model3D = nullptr;
}

void AnimationSequencerModal::Step()
{
	XMFLOAT3 baseColor = { 0.5f, 0.5f, 0.5f };
	floor->WriteConstantsBuffer<XMFLOAT3>("baseColor", baseColor, renderer->backBufferIndex);

	//https://stackoverflow.com/a/32836605
	BoundingBox modelBB = renderable->GetBoundingBox();
	BoundingSphere modelBBS;
	BoundingSphere::CreateFromBoundingBox(modelBBS, modelBB);

	float fov = XMConvertToRadians(camera->perspective().fovAngleY);
	float distance = (modelBBS.Radius * 2.0f) / (XMScalarSin(fov) / XMScalarCos(fov));

	XMVECTOR camFwV = camera->forward();
	XMFLOAT3 BBPos = modelBB.Center;
	XMVECTOR BBPosV = XMLoadFloat3(&BBPos);
	XMVECTOR camPosV = XMVectorSubtract(BBPosV, XMVectorScale(camFwV, distance));
	XMFLOAT3 camPos;
	XMStoreFloat3(&camPos, camPosV);
	camera->position(camPos);
}

static ImVec2 modelPosAdj(0.0f, 21.0f);
static ImVec2 sequencerSizeAdj(-7.0f, -47.0f);
static ImVec2 sequencerPosAdj(0.0f, 4.0f);
void AnimationSequencerModal::DrawSequencer(const char* title, ImVec2 pos, ImVec2 size)
{
	ImGui::OpenPopup(title);

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	if (ImGui::BeginPopupModal(title, nullptr, defaultChildFlag))
	{
		DrawSequenceSelector(pos);

		ImVec2 modelSize(size.y * 0.5f * 16.0f / 9.0f, size.y * 0.5f);
		ImVec2 modelPos(pos.x + (size.x - modelSize.x) * 0.5f + modelPosAdj.x, pos.y + modelPosAdj.y);

		DrawModelPreview(modelPos, modelSize);

		ImVec2 timeControllerSize(size.x, 20.0f);
		ImVec2 timeControllerPos(pos.x, modelPos.y + modelSize.y);
		if (!selectedSequence.empty())
		{
			DrawTimelineController(timeControllerPos, timeControllerSize);
		}

		ImVec2 sequencerPos(pos.x, timeControllerPos.y + timeControllerSize.y + sequencerPosAdj.y);
		ImVec2 sequencerSize(size.x + sequencerSizeAdj.x, size.y - modelSize.y - timeControllerSize.y + sequencerSizeAdj.y);
		static int selectedEntry = -1;
		static int firstFrame = 0;
		static bool expanded = true;
		static int currentFrame = 100;
		if (!selectedSequence.empty())
		{
			DrawSequencer(sequencerPos, sequencerSize, currentFrame, expanded, selectedEntry, firstFrame);
		}

		ImVec2 buttonsPos(pos.x, sequencerPos.y + sequencerSize.y);
		ImVec2 buttonSize(200.0f, 20.0f);
		DrawSaveAndExitButtons(buttonsPos, buttonSize);

		if (addNewSequence)
		{
			ImVec2 newSeqSize(200, 75);
			ImVec2 newSeqPos(pos.x + (size.x - newSeqSize.x) * 0.5f, pos.y + (size.y - newSeqSize.y) * 0.5f);
			DrawAddNewSequencePopup(newSeqPos, newSeqSize);
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawSequenceSelector(ImVec2 curPos)
{
	std::string title = "Sequence";

	ImVec2 textSize = ImGui::CalcTextSize(title.c_str());
	ImVec2 selectorSize(200, 50);
	ImVec2 windowSize(textSize.x + selectorSize.x + 6, selectorSize.y);
	ImVec2 windowPos(curPos.x + 4, curPos.y + 2);

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

		ImGui::DrawItemWithEnabledState([this]
			{
				if (ImGui::Button(ICON_FA_TIMES))
				{
					animationsSequences.sequences.erase(selectedSequence);
					selectedSequence = "";
				}
			}
		, selectedSequence != "");
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_PLUS))
		{
			addNewSequence = true;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(selectorSize.x);
		ImGui::DrawComboSelection(selectedSequence, sequences, [this](std::string sequence)
			{
				selectedSequence = sequence;
				auto it = animationsSequences.sequences.find(sequence);
				bool isAnim = it != animationsSequences.sequences.end();
				renderable->SetCurrentAnimation(isAnim ? &(it->second) : nullptr, 0.0f, 0.0f, false, false);
			}
		);
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawModelPreview(ImVec2 curPos, ImVec2 size)
{
	if (!camera) return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	ImGui::SetNextWindowSize(size, 0);
	ImGui::SetNextWindowPos(curPos, 0);
	ImGui::BeginChild("model-preview", size, 0);
	{
		auto& pass = camera->cameraRenderPasses.at(0);
		ImGui::DrawTextureImage(
			(ImTextureID)
			pass->renderToTexturePass->renderToTexture[0]->gpuTextureHandle.ptr,
			pass->renderToTexturePass->renderToTexture[0]->width,
			pass->renderToTexturePass->renderToTexture[0]->height
		);
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawTimelineController(ImVec2 curPos, ImVec2 size)
{
	float nFramesWidth = ImGui::CalcTextSize("#frames").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float inputNumFramesWidth = 100.0f;
	float fpsWidth = ImGui::CalcTextSize("FPS").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float inputFPSWidth = 100;
	float playPauseButtonWidth = ImGui::CalcTextSize(ICON_FA_PLAY).x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float stopButtonWidth = ImGui::CalcTextSize(ICON_FA_STOP).x + ImGui::GetStyle().FramePadding.x * 2.0f;
	//float loopWidth = ImGui::CalcTextSize("loop").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	float loopCheckboxWidth = 30.0f;
	float timeWidth = ImGui::CalcTextSize(std::string(std::string("time:") + std::format("{:.2f}", renderable->animationTime() / 1000.0f) + "s").c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + 30.0f;
	float frameWidth = ImGui::CalcTextSize(std::string(std::string("frame:") + std::to_string(renderable->animationFrame())).c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + 30.0f;
	float animationWidth = ImGui::CalcTextSize(std::string(std::string("animation:") + renderable->animation()).c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + 30.0f;
	float total_width =
		nFramesWidth +
		inputNumFramesWidth +
		fpsWidth +
		inputFPSWidth +
		ImGui::GetStyle().ItemSpacing.x +
		playPauseButtonWidth +
		stopButtonWidth +
		loopCheckboxWidth +
		timeWidth +
		frameWidth +
		animationWidth;

	float window_width = ImGui::GetContentRegionAvail().x;
	float start_x = (window_width - total_width) * 0.5f;

	ImGui::SetCursorPosX(start_x);

	ImGui::Text("#frames");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputNumFramesWidth);
	ImGui::PushID("TimeControllerTotalFrames");
	if (ImGui::InputInt("##", &animationsSequences.sequences.at(selectedSequence).totalFrames, 1, 100))
	{
		animationsSequences.sequences.at(selectedSequence).totalFrames = max(animationsSequences.sequences.at(selectedSequence).totalFrames, 1);
	}
	ImGui::PopID();

	ImGui::SameLine();
	ImGui::Text("FPS");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputFPSWidth);
	ImGui::PushID("TimeControllerFramesPerSecond");
	if (ImGui::InputInt("##", &animationsSequences.sequences.at(selectedSequence).framesPerSecond, 1, 100))
	{
		animationsSequences.sequences.at(selectedSequence).framesPerSecond = max(animationsSequences.sequences.at(selectedSequence).framesPerSecond, 1);
	}
	ImGui::PopID();

	ImGui::SameLine();
	if (!renderable->animationPlay())
	{
		if (ImGui::Button(ICON_FA_PLAY))
		{
			if (renderable->animationTimeFactor() == 0.0f)
			{
				auto it = animationsSequences.sequences.find(selectedSequence);
				bool isAnim = it != animationsSequences.sequences.end();
				renderable->SetCurrentAnimation(isAnim ? &(it->second) : nullptr, 0.0f, 1.0f, false, false);

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
	ImGui::Text(std::string(std::string("animation:") + renderable->animation()).c_str());
}

void AnimationSequencerModal::DrawSequencer(ImVec2 curPos, ImVec2 size, int& currentFrame, bool& expanded, int& selectedEntry, int& firstFrame)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

	ImGui::SetNextWindowSize(size, 0);
	ImGui::SetNextWindowPos(curPos, 0);
	ImGui::BeginChild("sequencer", size, 0);
	{
		ExImSequencer::Sequencer(this, &currentFrame, &expanded, &selectedEntry, &firstFrame, ExImSequencer::SEQUENCER_EDIT_STARTEND | ExImSequencer::SEQUENCER_ADD | ExImSequencer::SEQUENCER_DEL | ExImSequencer::SEQUENCER_COPYPASTE | ExImSequencer::SEQUENCER_CHANGE_FRAME);
	}
	ImGui::EndChild();

	ImGui::PopStyleColor();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawSaveAndExitButtons(ImVec2 curPos, ImVec2 size)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	ImGui::SetNextWindowPos(curPos, 0);
	ImGui::SetNextWindowSize(size, 0);
	ImGui::BeginChild("saveexit", size, 0);
	{
		ImGui::DrawItemWithEnabledState([this]
			{
				if (ImGui::Button("Save & Exit"))
				{
					destroying = true;
					showing = false;
					model3D->animationSequences(animationsSequences);
					Editor::templatesModified = true;
				}
			}
		, true);

		ImGui::SameLine();
		if (ImGui::Button("Exit"))
		{
			destroying = true;
			showing = false;
		}
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void AnimationSequencerModal::DrawAddNewSequencePopup(ImVec2 curPos, ImVec2 size)
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
	if (ImGui::BeginPopupModal(title, nullptr, defaultChildFlag))
	{
		ImGui::SetNextItemWidth(size.x);
		ImGui::InputText("##", &newSequenceName);

		float button1_width = ImGui::CalcTextSize("Add").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float button2_width = ImGui::CalcTextSize("Cancel").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		float total_width = button1_width + button2_width + ImGui::GetStyle().ItemSpacing.x;

		float window_width = ImGui::GetContentRegionAvail().x;
		float start_x = (window_width - total_width) * 0.5f;

		ImGui::SetCursorPosX(start_x);

		ImGui::DrawItemWithEnabledState([this]
			{
				if (ImGui::Button("Add"))
				{
					addNewSequence = false;
					animationsSequences.sequences.insert_or_assign(
						newSequenceName,
						Sequence(
							animations.at(0)
						)
					);
					expandedChannels.insert_or_assign(newSequenceName, std::vector<bool>({ false }));
					selectedSequence = newSequenceName;
				}
			}
		, !existingSequences.contains(newSequenceName));

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			addNewSequence = false;
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

int AnimationSequencerModal::GetFrameMin() const
{
	return 0;
}

int AnimationSequencerModal::GetFrameMax() const
{
	return animationsSequences.sequences.at(selectedSequence).totalFrames;
}

int AnimationSequencerModal::GetItemCount() const
{
	return (int)animationsSequences.sequences.at(selectedSequence).channels.size();
}

int AnimationSequencerModal::GetItemTypeCount() const
{
	return static_cast<int>(animations.size());
}

const char* AnimationSequencerModal::GetItemTypeName(int typeIndex) const
{
	return animations.at(typeIndex).c_str();
}

const char* AnimationSequencerModal::GetItemLabel(int index) const
{
	static char tmps[512];
	snprintf(tmps, 512, "[%02d] %s", index, animationsSequences.sequences.at(selectedSequence).channels.at(index).animation.c_str());
	return tmps;
}

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

void AnimationSequencerModal::Add(int type)
{
	SequenceChannel seqC;
	seqC.animation = animations.at(type);
	animationsSequences.sequences.at(selectedSequence).channels.push_back(seqC);
	expandedChannels.at(selectedSequence).push_back(false);
}

void AnimationSequencerModal::Del(int index) {
	if (animationsSequences.sequences.at(selectedSequence).channels.size() == 1ULL) return;
	animationsSequences.sequences.at(selectedSequence).channels.erase(animationsSequences.sequences.at(selectedSequence).channels.begin() + index);
	expandedChannels.at(selectedSequence).erase(expandedChannels.at(selectedSequence).begin() + index);
}

void AnimationSequencerModal::Duplicate(int index)
{
	SequenceChannel seqC = animationsSequences.sequences.at(selectedSequence).channels.at(index);
	animationsSequences.sequences.at(selectedSequence).channels.push_back(seqC);
	expandedChannels.at(selectedSequence).push_back(false);
}

size_t AnimationSequencerModal::GetCustomHeight(int index)
{
	return (expandedChannels.contains(selectedSequence) && expandedChannels.at(selectedSequence).at(index)) ? 300 : 0;
}

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
