#include "pch.h"
#include "Camera.h"
#include "../Scene.h"
#include "../Lights/Lights.h"
#include "../../Renderer/Renderer.h"
#include <nlohmann/json.hpp>
#if defined(_EDITOR)
#include "../../Editor/Editor.h"
#include <Editor.h>
#include "imgui.h"
#endif
#include <Keyboard.h>
#include <GamePad.h>
#include <SimpleMath.h>
#include <Mouse.h>

extern RECT hWndRect;
extern std::shared_ptr<Renderer> renderer;

namespace Scene
{
	using namespace DeviceUtils;
	using namespace DirectX;

	static inline std::string defaultCamUUID = "br0ken-camuuid";

	std::vector<std::shared_ptr<Camera>> cameraByIndex;
	std::map<std::string, std::shared_ptr<Camera>> cameraByUUID;
#if defined(_EDITOR)
	std::map<std::shared_ptr<Camera>, std::vector<std::function<void()>>> cameraDestructionCallbacks;
	nlohmann::json Camera::creationJson;
	unsigned int Camera::popupModalId = 0U;
#endif

	std::shared_ptr<Camera> CreateCamera(nlohmann::json cameraj)
	{
		std::shared_ptr<Camera> camera = std::make_shared<Camera>();
		camera->json = cameraj;
		camera->this_ptr = camera;

		SetIfMissingJson(camera->json, "name", "cam." + std::to_string(cameraByIndex.size()));
		SetIfMissingJson(camera->json, "uuid", defaultCamUUID);
		SetIfMissingJson(camera->json, "fitWindow", false);
		SetIfMissingJson(camera->json, "projectionType", ProjectionTypesToStr.at(PROJ_Perspective));
		SetIfMissingJson(camera->json, "minLogLuminance", -2.0f);
		SetIfMissingJson(camera->json, "maxLogLuminance", 10.0f);
		SetIfMissingJson(camera->json, "tau", 1.1f);
		SetIfMissingJson(camera->json, "IBLIrradiance", "");
		SetIfMissingJson(camera->json, "IBLPreFilteredEnvironment", "");
		SetIfMissingJson(camera->json, "IBLBRDFLUT", "");

		bool fitToWindow = camera->json.at("fitWindow");

		switch (StrToProjectionTypes.at(camera->json.at("projectionType")))
		{
		case PROJ_Perspective:
		{
			float winWidth = fitToWindow ? static_cast<float>(hWndRect.right - hWndRect.left) : static_cast<float>(cameraj["perspective"]["width"]);
			float winHeight = fitToWindow ? static_cast<float>(hWndRect.bottom - hWndRect.top) : static_cast<float>(cameraj["perspective"]["height"]);
			float nearZ = cameraj["perspective"].contains("nearZ") ? static_cast<float>(cameraj["perspective"]["nearZ"]) : CameraProjections::Perspective::defaultNearZ;
			float farZ = cameraj["perspective"].contains("farZ") ? static_cast<float>(cameraj["perspective"]["farZ"]) : CameraProjections::Perspective::defaultFarZ;
			float fovAngleY = cameraj["perspective"].contains("fovAngleY") ? static_cast<float>(cameraj["perspective"]["fovAngleY"]) : CameraProjections::Perspective::defaultFovAngleY;

			camera->perspective = {
				.nearZ = nearZ,
				.farZ = farZ,
				.fovAngleY = fovAngleY,
				.width = winWidth,
				.height = winHeight,
			};
			camera->perspective.updateProjectionMatrix();
#if defined(_EDITOR)
			camera->editorPerspective.Copy(camera->perspective);
#endif
		}
		break;
		case PROJ_Orthographic:
		{
			float winWidth = fitToWindow ? static_cast<float>(hWndRect.right - hWndRect.left) : static_cast<float>(cameraj["orthographic"]["width"]);
			float winHeight = fitToWindow ? static_cast<float>(hWndRect.bottom - hWndRect.top) : static_cast<float>(cameraj["orthographic"]["height"]);
			float nearZ = cameraj["orthographic"].contains("nearZ") ? static_cast<float>(cameraj["orthographic"]["nearZ"]) : CameraProjections::Orthographic::defaultNearZ;
			float farZ = cameraj["orthographic"].contains("farZ") ? static_cast<float>(cameraj["orthographic"]["farZ"]) : CameraProjections::Orthographic::defaultFarZ;

			camera->orthographic = {
				.nearZ = nearZ,
				.farZ = farZ,
				.width = winWidth,
				.height = winHeight
			};
			camera->orthographic.updateProjectionMatrix();
#if defined(_EDITOR)
			camera->editorOrthographic.Copy(camera->orthographic);
#endif
		}
		break;
		default:
		{
			assert(true); //not implemented
		}
		break;
		}

		SetIfMissingJson(camera->json, "position", XMFLOAT3({ 0.0f,0.0f,0.0f }));
		SetIfMissingJson(camera->json, "rotation", XMFLOAT3({ 0.0f,0.0f,0.0f }));
		SetIfMissingJson(camera->json, "speed", 0.05f);
		SetIfMissingJson(camera->json, "white", 1.1f);

		if (cameraj.contains("light"))
		{
			camera->light = GetLight(cameraj["light"]);
		}

		camera->CreateIBLTexturesInstances();
		camera->CreateConstantsBuffer();

		cameraByIndex.push_back(camera);
		cameraByUUID.insert_or_assign(camera->json.at("uuid"), camera);

		return camera;
	}

	std::vector<std::shared_ptr<Camera>> GetCameras()
	{
		return cameraByIndex;
	}

	std::vector<std::string> GetCamerasNames()
	{
		std::map<std::string, std::shared_ptr<Camera>> c;
		std::copy_if(cameraByUUID.begin(), cameraByUUID.end(), std::inserter(c, c.end()), [](const auto& pair)
			{
				return !pair.second->light;
			}
		);
		std::vector<std::string> cameraNames;
		std::transform(c.begin(), c.end(), std::back_inserter(cameraNames), [](const auto& pair)
			{
				return pair.second->name();
			}
		);
		return cameraNames;
	}

	std::vector<UUIDName> GetCamerasUUIDNames()
	{
		std::map<std::string, std::shared_ptr<Camera>> ret;
		std::copy_if(cameraByUUID.begin(), cameraByUUID.end(), std::inserter(ret, ret.end()), [](const auto& pair)
			{
				return !pair.second->hidden() && pair.second->light == nullptr;
			}
		);

		return GetSceneObjectsUUIDsNames(ret);
	}

#if defined(_EDITOR)
	void SelectCamera(std::string uuid, std::string& edSO)
	{
		edSO = uuid;
	}

	void DeSelectCamera(std::string& edSO)
	{
		edSO = "";
	}

	void DrawCameraPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop)
	{
		std::shared_ptr<Camera> camera = cameraByUUID.at(uuid);

		if (camera->light != nullptr)
		{
			ImGui::Text("camera is controlled by light");
			ImGui::SameLine();
			if (ImGui::TextLink(camera->light->name().c_str()))
			{
				Editor::SelectSceneObject(_SceneObjects::SO_Lights, camera->light->uuid());
			}
			return;
		}

		std::string tableName = "camera-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			camera->DrawEditorInformationAttributes();
			camera->DrawEditorWorldAttributes();
			camera->DrawEditorCameraAttributes();
			camera->DrawEditorIBLAttributes();
			ImGui::EndTable();
		}
	}

	std::string GetCameraName(std::string uuid)
	{
		std::shared_ptr<Camera> camera = cameraByUUID.at(uuid);
		return camera->json.at("name");
	}

	void CreateNewCamera()
	{
		Camera::popupModalId = RenderablePopupModal_CreateNew;
		Camera::creationJson = nlohmann::json(
			{
				{ "name", "" },
				{ "projectionType", ProjectionTypesToStr.at(PROJ_Perspective) }
			}
		);
	}

	void Camera::BindDestruction(std::function<void()> cb)
	{
		cameraDestructionCallbacks[this_ptr].push_back(cb);
	}

	void DeleteCamera(std::string uuid)
	{
		std::shared_ptr<Camera> camera = cameraByUUID.at(uuid);
		if (camera->hidden() || !camera->light)
		{
			camera->cameraUpdateFlags |= CameraFlags_Destroy;
		}
	}

	void DrawCamerasPopups()
	{
		Editor::DrawCreateWindow(Camera::popupModalId, CameraPopupModal_CreateNew, "New Camera", [](auto OnCancel)
			{
				nlohmann::json& json = Camera::creationJson;

				ImGui::PushID("camera-name");
				{
					ImGui::Text("Name");
					ImDrawJsonInputText(json, "name");
				}
				ImGui::PopID();

				ImGui::Text("Projection Type");

				ImGui::PushID("camera-projection-type");
				{
					std::string projectionType = json.at("projectionType");
					DrawComboSelection(projectionType, ProjectionsTypesStr, [&json](std::string newProjection)
						{
							json.at("projectionType") = newProjection;
						}
					);
				}
				ImGui::PopID();

				if (ImGui::Button("Cancel")) { OnCancel(); }
				ImGui::SameLine();
				bool disabledCreate = json.at("name") == "";

				DrawItemWithEnabledState([&json]
					{
						if (ImGui::Button("Create"))
						{
							Camera::popupModalId = 0;
							nlohmann::json r = {
								{ "uuid", getUUID() },
								{ "name", json.at("name") },
								{ "projectionType", json.at("projectionType") }
							};

							ProjectionsTypes projType = StrToProjectionTypes.at(json.at("projectionType"));
							switch (projType)
							{
							case PROJ_Perspective:
							{
								r["perspective"]["width"] = 1024;
								r["perspective"]["height"] = 1024;
								r["perspective"]["nearZ"] = CameraProjections::Perspective::defaultNearZ;
								r["perspective"]["farZ"] = CameraProjections::Perspective::defaultFarZ;
								r["perspective"]["fovAngleY"] = CameraProjections::Perspective::defaultFovAngleY;
							}
							break;
							case PROJ_Orthographic:
							{
								r["orthographic"]["width"] = 1024;
								r["orthographic"]["height"] = 1024;
								r["orthographic"]["nearZ"] = CameraProjections::Orthographic::defaultNearZ;
								r["orthographic"]["farZ"] = CameraProjections::Orthographic::defaultFarZ;
							}
							break;
							default:
							{}
							break;
							}

							CreateCamera(r);
						}
					}, !disabledCreate
				);
			}
		);
	}

	void WriteCamerasJson(nlohmann::json& json)
	{
		std::map<std::string, std::shared_ptr<Camera>> filtered;
		std::copy_if(cameraByUUID.begin(), cameraByUUID.end(), std::inserter(filtered, filtered.end()), [](const auto& pair)
			{
				auto& [uuid, camera] = pair;
				return !camera->hidden() && camera->light == nullptr;
			}
		);
		std::transform(filtered.begin(), filtered.end(), std::back_inserter(json), [](const auto& pair)
			{
				auto& [uuid, camera] = pair;
				nlohmann::json ret = camera->json;
				ret["uuid"] = uuid;
				return ret;
			}
		);
	}
#endif

	void CamerasStep()
	{
#if defined(_EDITOR)
		std::vector<std::shared_ptr<Camera>> camerasToDestroy;
		std::copy_if(cameraByIndex.begin(), cameraByIndex.end(), std::back_inserter(camerasToDestroy), [](const std::shared_ptr<Camera>& cam)
			{
				return cam->cameraUpdateFlags & CameraFlags_Destroy;
			}
		);

		std::map<std::shared_ptr<Camera>, std::set<TextureShaderUsage>> camerasToDestroyIBL;
		std::map<std::shared_ptr<Camera>, std::set<TextureShaderUsage>> camerasToCreateIBL;
		std::set<std::shared_ptr<Camera>> camerasToResetFlag;
		for (auto& cam : cameraByIndex)
		{
			for (auto& [usage, flag] : cam->iblTexturesFlags)
			{
				if (flag & CameraIBLTextureFlags_Destroy) camerasToDestroyIBL[cam].insert(usage);
				if (flag & CameraIBLTextureFlags_Create) camerasToCreateIBL[cam].insert(usage);
			}
			if (cam->iblTexturesFlags.size() > 0ULL) camerasToResetFlag.insert(cam);
		}

		bool goCritical = (camerasToDestroy.size() > 0ULL || camerasToDestroyIBL.size() > 0ULL || camerasToCreateIBL.size() > 0ULL);

		if (goCritical)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([&camerasToDestroy, &camerasToDestroyIBL, &camerasToCreateIBL, &camerasToResetFlag]
				{
					for (auto& [cam, usages] : camerasToDestroyIBL)
					{
						for (auto& usage : usages)
						{
							DestroyTextureInstance(cam->iblTextures.at(usage));
							cam->iblTextures.erase(usage);
						}
					}
					camerasToDestroyIBL.clear();

					for (auto& [cam, usages] : camerasToCreateIBL)
					{
						std::map<TextureShaderUsage, std::string> textures;
						for (auto& usage : usages)
						{
							if (cam->json.at(iblUsageTexture.at(usage)) != "")
							{
								textures.insert_or_assign(usage, cam->json.at(iblUsageTexture.at(usage)));
							}
						}
						std::map<TextureShaderUsage, std::shared_ptr<TextureInstance>> newInstances = GetTextures(textures);
						for (auto& [usage, texInstance] : newInstances)
						{
							cam->iblTextures.insert_or_assign(usage, texInstance);
						}
					}
					camerasToCreateIBL.clear();

					for (auto& cam : camerasToResetFlag)
					{
						cam->iblTexturesFlags.clear();
					}

					camerasToCreateIBL.clear();

					for (auto& cam : camerasToDestroy)
					{
						if (cameraDestructionCallbacks.contains(cam))
						{
							for (auto& cb : cameraDestructionCallbacks.at(cam))
							{
								cb();
							}
							cameraDestructionCallbacks.erase(cam);
						}
						DestroyCamera(cam);
					}
					camerasToDestroy.clear();
				}
			);
		}
#endif
	}

	void DestroyCamera(std::shared_ptr<Camera>& camera)
	{
		if (camera == nullptr) return;
		nostd::vector_erase(cameraByIndex, camera);
		cameraByUUID.erase(camera->json.at("uuid"));
		camera->this_ptr = nullptr;
		camera = nullptr;
	}

	void DestroyCameras()
	{
		for (auto& cam : cameraByIndex)
		{
			cam->this_ptr = nullptr;
			cam->light = nullptr;
		}
		cameraByIndex.clear();
		cameraByUUID.clear();
#if defined(_EDITOR)
		cameraDestructionCallbacks.clear();
#endif
	}

	std::string Camera::uuid()
	{
		return json.at("uuid");
	}

	void Camera::uuid(std::string uuid)
	{
		json["uuid"] = uuid;
	}

	std::string Camera::name()
	{
		return json.at("name");
	}

	void Camera::name(std::string name)
	{
		json["name"] = name;
	}

	XMFLOAT3 Camera::position()
	{
		return XMFLOAT3(json.at("position").at(0), json.at("position").at(1), json.at("position").at(2));
	}

	XMVECTOR Camera::positionV()
	{
		XMFLOAT3 pos = position();
		return { pos.x,pos.y,pos.z,0.0f };
	}

	void Camera::position(XMFLOAT3 f3)
	{
		nlohmann::json& j = json.at("position");
		j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
	}

	void Camera::position(nlohmann::json f3)
	{
		json.at("position") = f3;
	}

	XMFLOAT3 Camera::rotation()
	{
		return XMFLOAT3(json.at("rotation").at(0), json.at("rotation").at(1), json.at("rotation").at(2));
	}

	void Camera::rotation(XMFLOAT3 f3)
	{
		nlohmann::json& j = json.at("rotation");
		j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
	}

	void Camera::rotation(nlohmann::json f3)
	{
		json.at("rotation") = f3;
	}

	float Camera::white()
	{
		return json.at("white");
	}

	void Camera::white(float f)
	{
		json.at("white") = f;
	}

	float Camera::minLogLuminance()
	{
		return json.at("minLogLuminance");
	}

	void Camera::minLogLuminance(float f)
	{
		json.at("minLogLuminance") = f;
	}

	float Camera::maxLogLuminance()
	{
		return json.at("maxLogLuminance");
	}

	void Camera::maxLogLuminance(float f)
	{
		json.at("maxLogLuminance") = f;
	}

	float Camera::tau()
	{
		return json.at("tau");
	}

	void Camera::tau(float f)
	{
		json.at("tau") = f;
	}

	XMMATRIX Camera::world()
	{
		XMFLOAT3 posV = position();
		XMFLOAT3 rotV = rotation();
		XMMATRIX rotationM = XMMatrixRotationRollPitchYawFromVector({ rotV.x, rotV.y, rotV.z, 0.0f });
		XMMATRIX positionM = XMMatrixTranslationFromVector({ posV.x, posV.y, posV.z });
		return XMMatrixMultiply(rotationM, positionM);
	}

	void Camera::Destroy()
	{
		for (auto& [_, tex] : iblTextures)
		{
			DestroyTextureInstance(tex);
		}
		iblTextures.clear();
		DestroyConstantsBuffer(cameraCbv);
	}

	void Camera::CreateIBLTexturesInstances()
	{
		if (std::any_of(iblJsonTextures.begin(), iblJsonTextures.end(), [this](auto& tup)
			{
				std::string& attName = std::get<0>(tup);
				return json.at(attName) == "";
			}
		))
			return;

		std::map<TextureShaderUsage, std::string> textures;
		std::transform(iblJsonTextures.begin(), iblJsonTextures.end(), std::inserter(textures, textures.end()), [this](auto& tup)
			{
				std::string& attName = std::get<0>(tup);
				TextureShaderUsage& usage = std::get<1>(tup);
				return std::pair<TextureShaderUsage, std::string>(usage, json.at(attName));
			}
		);

		iblTextures = GetTextures(textures);
	}

	void Camera::CreateConstantsBuffer()
	{
		cameraCbv = DeviceUtils::CreateConstantsBuffer(sizeof(CameraAttributes) * renderer->numFrames, json.at("name"));
	}

	void Camera::WriteConstantsBuffer(unsigned int backbufferIndex)
	{
		CameraAttributes atts{};
		if (StrToProjectionTypes.at(json.at("projectionType")) == PROJ_Orthographic)
		{
			atts.viewProjection = XMMatrixMultiply(ViewMatrix(), orthographic.projectionMatrix);
		}
		else
		{
			atts.viewProjection = XMMatrixMultiply(ViewMatrix(), perspective.projectionMatrix);
		}

		XMFLOAT3 pos = position();
		XMVECTOR fw = CameraFw();

		atts.eyePosition = { pos.x, pos.y, pos.z, 0.0f };
		atts.eyeForward = *(XMFLOAT4*)&fw.m128_f32;
		atts.white = white();

		memcpy(cameraCbv->mappedConstantBuffer + cameraCbv->alignedConstantBufferSize * backbufferIndex, &atts, sizeof(atts));
	}

	XMMATRIX Camera::ViewMatrix()
	{
		XMVECTOR viewUp = up;
		if (light != nullptr && light->lightType() == LT_Point)
		{
			unsigned int i = 0U;
			for (; i < 6U; i++)
			{
				if (light->shadowMapCameras[i].get() == this)
				{
					viewUp = Scene::PointLightUp[i]; break;
				}
			}
		}
		return XMMatrixLookToRH(positionV(), CameraFw(), viewUp);
	}

	XMVECTOR Camera::CameraFw()
	{
		if (light != nullptr && light->lightType() == LT_Point)
		{
			unsigned int i = 0U;
			for (; i < 6U; i++) {
				if (light->shadowMapCameras[i].get() == this)
				{
					return Scene::PointLightDirection[i];
				}
			}
		}

		XMFLOAT3 rot = rotation();

		return {
			sinf(rot.x) * cosf(rot.y),
			sinf(rot.y),
			cosf(rot.x) * cosf(rot.y)
		};
	}

	XMVECTOR Camera::CameraUp()
	{
		if (light != nullptr && light->lightType() == LT_Point)
		{
			unsigned int i = 0U;
			for (; i < 6U; i++)
			{
				if (light->shadowMapCameras[i].get() == this)
				{
					return Scene::PointLightUp[i];
				}
			}
		}

		return up;
	}

	void Camera::ProcessKeyboardInput(DirectX::Keyboard::KeyboardStateTracker& tracker, DirectX::Keyboard::State& state)
	{
		if (light == nullptr || light->lightType() == LT_Spot || light->lightType() == LT_Point)
		{
			float moveSpeed = json.at("speed") * ((state.LeftShift || state.RightShift) ? 10.0f : 1.0f);
			if (state.Up) { MoveForward(moveSpeed); }
			if (state.Down) { MoveBack(moveSpeed); }
			if (state.Left) { MoveLeft(moveSpeed); }
			if (state.Right) { MoveRight(moveSpeed); }
		}
	}

	void Camera::MoveAlongFwAxis(float dz)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), CameraFw() * dz);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::MovePerpendicularFwAxis(float dx, float dy)
	{
		XMVECTOR up = CameraUp();
		XMVECTOR right = XMVector3Cross(CameraFw(), up);
		XMVECTOR newPos = positionV();
		newPos += up * dy;
		newPos += right * dx;
		position(*(XMFLOAT3*)newPos.m128_f32);
	}

	void Camera::Rotate(float dx, float dy)
	{
		rotation(rotation() - XMFLOAT3{ dx, dy, 0.0f });
		UdateLightRotation();
	}

	void Camera::ProcessGamepadInput(DirectX::GamePad::State& gamePadState, Vector2 gamePadCameraRotationSensitivity)
	{
		if (light == nullptr || light->lightType() == LT_Spot || light->lightType() == LT_Point)
		{
			if (gamePadState.thumbSticks.leftY > 0) { MoveForward(json.at("speed")); }
			if (gamePadState.thumbSticks.leftY < 0) { MoveBack(json.at("speed")); }
			if (gamePadState.thumbSticks.leftX < 0) { MoveLeft(json.at("speed")); }
			if (gamePadState.thumbSticks.leftX > 0) { MoveRight(json.at("speed")); }
		}
		if (light == nullptr || light->lightType() == LT_Directional || light->lightType() == LT_Spot)
		{
			Vector2 stickDiff = { gamePadState.thumbSticks.rightX, gamePadState.thumbSticks.rightY };
			rotation(rotation() - XMFLOAT3{ stickDiff.x * gamePadCameraRotationSensitivity.x, stickDiff.y * gamePadCameraRotationSensitivity.y, 0.0f });
			UdateLightRotation();
		}
	}

	XMFLOAT2 lastMousePos;
	XMFLOAT2 GetMouseDiff(DirectX::Mouse::State& mouseState)
	{
		XMFLOAT2 currentMousePos = { static_cast<float>(mouseState.x) , static_cast<float>(mouseState.y) };
		XMFLOAT2 diff = { 0.0f, 0.0f };
		if (mouseState.leftButton)
		{
			diff = { currentMousePos.x - lastMousePos.x , currentMousePos.y - lastMousePos.y };
		}
		lastMousePos = currentMousePos;
		return diff;
	}

	int lastWheelValue = 0;
	float wheelDiffFactor = 0.0001f;
	void Camera::ProcessCameraMouseRotation(DirectX::Mouse::State& mouseState, Vector2 rotationSensitivity, bool firstStep)
	{
		if (light == nullptr || light->lightType() == LT_Directional || light->lightType() == LT_Spot)
		{
			Vector2 mouseDiff = GetMouseDiff(mouseState);
			mouseDiff = firstStep ? Vector2(0.0f, 0.0f) : mouseDiff;
			rotation(rotation() - XMFLOAT3{ mouseDiff.x * rotationSensitivity.x, mouseDiff.y * rotationSensitivity.y, 0.0f });
			UdateLightRotation();
			if (light != nullptr && light->lightType() == LT_Directional)
			{
				float diff = static_cast<float>(mouseState.scrollWheelValue - lastWheelValue) * wheelDiffFactor;
				orthographic.expandView(diff);
			}
			else if (light == nullptr)
			{
				float diff = static_cast<float>(mouseState.scrollWheelValue - lastWheelValue) * wheelDiffFactor;
			}
		}
	}

	void Camera::UpdateLightPosition()
	{
		if (light == nullptr) return;

		switch (light->lightType())
		{
		case LT_Spot:
		{
			light->position(position());
		}
		break;
		case LT_Point:
		{
			light->position(position());
			for (auto cam : light->shadowMapCameras)
			{
				cam->position(position());
			}
		}
		break;
		}
	}

	void Camera::UdateLightRotation()
	{
		if (light == nullptr) return;

		XMFLOAT3 rot = rotation();

		switch (light->lightType())
		{
		case LT_Directional:
		{
			light->rotation({ rot.x, rot.y });
			XMVECTOR camPos = XMVectorScale(XMVector3Normalize(CameraFw()), -light->directionalDistance());
			position(*(XMFLOAT3*)camPos.m128_f32);
		}
		break;
		case LT_Spot:
		{
			light->rotation({ rot.x, rot.y });
		}
		break;
		}
	}

	void Camera::MoveForward(float step)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), CameraFw() * step);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::MoveBack(float step)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), CameraFw() * -step);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::MoveLeft(float step)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), XMVector3Cross(CameraFw(), CameraUp()) * -step);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::MoveRight(float step)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), XMVector3Cross(CameraFw(), CameraUp()) * step);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	size_t GetNumCameras() { return cameraByIndex.size(); }
	std::shared_ptr<Camera> GetCamera(unsigned int index) { return cameraByIndex[index]; }
	std::shared_ptr<Camera> GetCamera(std::string uuid) { return cameraByUUID.at(uuid); }
	std::shared_ptr<Camera> GetCameraByName(std::string name)
	{
		for (auto& [_, cam] : cameraByUUID)
		{
			if (cam->name() == name) return cam;
		}
		return nullptr;
	}

#if defined(_EDITOR)

	void Camera::DrawEditorInformationAttributes()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "camera-information-atts";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string currentName = name();
			if (ImGui::InputText("name", &currentName))
			{
				name(currentName);
			}
			ImGui::EndTable();
		}
	}

	void Camera::DrawEditorWorldAttributes()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "camera-world-atts";
		if (ImGui::BeginTable(tableName.c_str(), 4, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::PushID("position");
			ImGui::Text("position");

			ImGui::TableSetColumnIndex(1);
			float pos0 = json.at("position").at(0);
			if (ImGui::InputFloat("x", &pos0)) { json.at("position")[0] = pos0; }

			ImGui::TableSetColumnIndex(2);
			float pos1 = json.at("position").at(1);
			if (ImGui::InputFloat("y", &pos1)) { json.at("position")[1] = pos1; }

			ImGui::TableSetColumnIndex(3);
			float pos2 = json.at("position").at(2);
			if (ImGui::InputFloat("z", &pos2)) { json.at("position")[2] = pos2; }

			ImGui::PopID();

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::PushID("rotation");
			ImGui::Text("rotation");

			ImGui::TableSetColumnIndex(1);
			float rot0 = json.at("rotation").at(0);
			if (ImGui::SliderAngle("azimuthal", &rot0)) { json.at("rotation")[0] = rot0; }

			ImGui::TableSetColumnIndex(2);
			float rot1 = json.at("rotation").at(1);
			if (ImGui::SliderAngle("polar", &rot1)) { json.at("rotation")[1] = rot1; }

			ImGui::PopID();

			ImGui::EndTable();
		}
	}

	void Camera::DrawEditorCameraAttributes()
	{
		std::vector<std::string> selectables = ProjectionsTypesStr;
		std::string selected = json.at("projectionType");

		DrawComboSelection(selected, selectables, [this](std::string projection)
			{
				json["projectionType"] = StrToProjectionTypes[projection];
				switch (StrToProjectionTypes.at(json.at("projectionType"))) {
				case PROJ_Orthographic:
				{
					orthographic.Copy(editorOrthographic);
				}
				break;
				case PROJ_Perspective:
				{
					perspective.Copy(editorPerspective);
				}
				break;
				}
			},
			"Projection"
		);

		auto updateEditorPerspective = [this]()
			{
				perspective.updateProjectionMatrix();
				editorPerspective.Copy(perspective);
			};

		auto updateEditorOrthographic = [this]()
			{
				orthographic.updateProjectionMatrix();
				editorOrthographic.Copy(orthographic);
			};

		if (StrToProjectionTypes.at(json.at("projectionType")) == PROJ_Perspective)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string tableName = "camera-projection-perspective-atts";
			if (ImGui::BeginTable(tableName.c_str(), 3, ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::PushID("perspective");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("perspective");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::InputFloat("nearZ", &perspective.nearZ))
				{
					updateEditorPerspective();
				}
				ImGui::TableSetColumnIndex(1);
				if (ImGui::InputFloat("farZ", &perspective.farZ))
				{
					updateEditorPerspective();
				}
				ImGui::TableSetColumnIndex(2);
				if (ImGui::SliderAngle("fovAngleY", &perspective.fovAngleY, 10.0f, 150.0f))
				{
					updateEditorPerspective();
				}
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::InputFloat("width", &perspective.width))
				{
					updateEditorPerspective();
				}
				ImGui::TableSetColumnIndex(1);
				if (ImGui::InputFloat("height", &perspective.height))
				{
					updateEditorPerspective();
				}

				ImGui::PopID();
				ImGui::EndTable();
			}
		}
		else
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string tableName = "camera-projection-orthographic-atts";
			if (ImGui::BeginTable(tableName.c_str(), 2, ImGuiTableFlags_NoSavedSettings))
			{
				ImGui::PushID("orthographic");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("orthographic");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::InputFloat("nearZ", &orthographic.nearZ))
				{
					updateEditorOrthographic();
				}
				ImGui::TableSetColumnIndex(1);
				if (ImGui::InputFloat("farZ", &orthographic.farZ))
				{
					updateEditorOrthographic();
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::InputFloat("width", &orthographic.width))
				{
					updateEditorOrthographic();
				}
				ImGui::TableSetColumnIndex(1);
				if (ImGui::InputFloat("height", &orthographic.height))
				{
					updateEditorOrthographic();
				}

				ImGui::PopID();
				ImGui::EndTable();
			}
		}

		float w = white();
		if (ImGui::InputFloat("white", &w))
		{
			white(w);
		}

		float minLogL = minLogLuminance();
		if (ImGui::InputFloat("minLogLuminance", &minLogL))
		{
			minLogLuminance(minLogL);
		}

		float maxLogL = maxLogLuminance();
		if (ImGui::InputFloat("maxLogLuminance", &maxLogL))
		{
			maxLogLuminance(maxLogL);
		}

		float tauf = tau();
		if (ImGui::InputFloat("tau", &tauf))
		{
			tau(tauf);
		}
	}

	void Camera::DrawEditorIBLAttributes()
	{
		std::vector<UUIDName> selectables = GetTexturesUUIDsNames();
		SortUUIDByName(selectables);

		for (auto& ibl : iblJsonTextures)
		{
			std::string name = std::get<0>(ibl);
			TextureShaderUsage usage = std::get<1>(ibl);

			ImGui::PushID(name.c_str());
			{
				UUIDName selected = std::make_tuple("", "");
				if (json.at(name) != "")
					selected = std::make_tuple(std::string(json.at(name)), GetTextureName(std::string(json.at(name))));

				DrawComboSelection(selected, selectables, [this, name, usage](UUIDName selection)
					{
						std::string texUUID = std::get<0>(selection);
						json.at(name) = texUUID;
						if (texUUID == "")
						{
							iblTexturesFlags.insert_or_assign(usage, CameraIBLTextureFlags_Destroy);
						}
						else
						{
							iblTexturesFlags.insert_or_assign(usage, CameraIBLTextureFlags_Create);
						}
					}, name
				);

				if (iblTextures.contains(usage))
				{
					std::shared_ptr<TextureInstance> texInstance = iblTextures.at(usage);
					nlohmann::json& texJson = GetTextureTemplate(texInstance->materialTexture);
					ImDrawTextureImage(texInstance->gpuHandle.ptr, static_cast<unsigned int>(texJson.at("width")), static_cast<unsigned int>(texJson.at("height")));
				}
			}
			ImGui::PopID();
		}
	}
#endif

	void Camera::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox)
	{
		bbox->position(json.at("position"));
		bbox->scale(XMFLOAT3({ 0.3f, 0.3f, 0.3f }));
		bbox->rotation(XMFLOAT3({ 0.0f, 0.0f, 0.0f }));
	}
}