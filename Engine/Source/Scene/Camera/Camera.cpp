#include "pch.h"
#include "Camera.h"
#include "../Lights/Lights.h"
#include "../../Renderer/Renderer.h"
#include <nlohmann/json.hpp>
#if defined(_EDITOR)
#include "../../Editor/Editor.h"
#include "imgui.h"
#endif

extern RECT hWndRect;
extern std::shared_ptr<Renderer> renderer;

namespace Scene
{
	using namespace DeviceUtils;

	std::vector<std::shared_ptr<Camera>> cameraByIndex;
	std::map<std::string, std::shared_ptr<Camera>> cameraByNames;

	std::shared_ptr<Camera> CreateCamera(nlohmann::json cameraj)
	{
		std::shared_ptr<Camera> camera = std::make_shared<Camera>();
		if (cameraj["name"] == "")
		{
			camera->name = "cam." + std::to_string(cameraByIndex.size());
		}
		else
		{
			camera->name = cameraj["name"];
		}
		assert(!cameraByNames.contains(camera->name));

		ReplaceFromJson(camera->fitWindow, cameraj, "fitWindow");
		camera->projectionType = StrToProjectionTypes[cameraj["projectionType"]];
		bool fitToWindow = (cameraj.contains("fitWindow") && cameraj["fitWindow"]);
		switch (camera->projectionType)
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

		JsonToFloat3(camera->position, cameraj, "position");
		if (cameraj.contains("rotation"))
		{
			XMFLOAT2 rot2 = JsonToFloat2(cameraj["rotation"]);
			camera->rotation = { rot2.x, rot2.y, 0.0f };
		}

		if (cameraj.contains("speed"))
		{
			camera->speed = cameraj["speed"];
		}

		if (cameraj.contains("light"))
		{
			camera->light = GetLight(cameraj["light"]);
		}

		camera->CreateConstantsBuffer();

		cameraByIndex.push_back(camera);
		cameraByNames[camera->name] = camera;
		camera->this_ptr = camera;
		return camera;
	}

	std::vector<std::shared_ptr<Camera>> GetCameras()
	{
		return cameraByIndex;
	}

	std::vector<std::string> GetCamerasNames()
	{
		std::map<std::string, std::shared_ptr<Camera>> c;
		std::copy_if(cameraByNames.begin(), cameraByNames.end(), std::inserter(c, c.end()), [](const auto& pair)
			{
				return !pair.second->light;
			}
		);
		return nostd::GetKeysFromMap(c);
	}

#if defined(_EDITOR)
	void SelectCamera(std::string cameraName, void*& ptr)
	{
		ptr = cameraByNames.at(cameraName).get();
	}

	void DeSelectCamera(void*& ptr)
	{
		ptr = nullptr;
	}

	void DrawCameraPanel(void*& ptr, ImVec2 pos, ImVec2 size, bool pop)
	{
		Camera* camera = (Camera*)ptr;

		if (camera->light != nullptr)
		{
			ImGui::Text("camera is controlled by light");
			ImGui::SameLine();
			if (ImGui::TextLink(camera->light->name.c_str()))
			{
				Editor::SelectSceneObject(_SceneObjects::SO_Lights, camera->light.get());
			}
			return;
		}

		std::string tableName = "camera-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			camera->DrawEditorInformationAttributes();
			camera->DrawEditorWorldAttributes();
			camera->DrawEditorCameraAttributes();
			ImGui::EndTable();
		}
	}

	std::string GetCameraName(void* ptr)
	{
		Camera* cam = (Camera*)ptr;
		return cam->name;
	}
#endif

	void DestroyCamera(std::shared_ptr<Camera>& camera)
	{
		if (camera == nullptr) return;
		nostd::vector_erase(cameraByIndex, camera);
		cameraByNames.erase(camera->name);
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
		cameraByNames.clear();
	}

	void Camera::Destroy()
	{
		DestroyConstantsBuffer(cameraCbv);
	}

	void Camera::CreateConstantsBuffer()
	{
		cameraCbv = DeviceUtils::CreateConstantsBuffer(sizeof(CameraAttributes) * renderer->numFrames, name);
	}

	void Camera::WriteConstantsBuffer(unsigned int backbufferIndex)
	{
		CameraAttributes atts{};
		if (projectionType == PROJ_Orthographic)
		{
			atts.viewProjection = XMMatrixMultiply(ViewMatrix(), orthographic.projectionMatrix);
		}
		else
		{
			atts.viewProjection = XMMatrixMultiply(ViewMatrix(), perspective.projectionMatrix);
		}

		atts.eyePosition = { position.x, position.y, position.z };
		atts.eyeForward = { *((XMFLOAT3*)CameraFw().m128_f32) };

		memcpy(cameraCbv->mappedConstantBuffer + cameraCbv->alignedConstantBufferSize * backbufferIndex, &atts, sizeof(atts));
	}

	XMMATRIX Camera::ViewMatrix()
	{
		XMVECTOR viewUp = up;
		if (light != nullptr && light->lightType == LT_Point)
		{
			unsigned int i = 0U;
			for (; i < 6U; i++)
			{
				if (light->shadowMapCameras[i].get() == this)
				{
					viewUp = Scene::PointLight::PointLightUp[i]; break;
				}
			}
		}
		return XMMatrixLookToRH({ position.x ,position.y, position.z }, CameraFw(), viewUp);
	}

	XMVECTOR Camera::CameraFw()
	{
		if (light != nullptr && light->lightType == LT_Point)
		{
			unsigned int i = 0U;
			for (; i < 6U; i++) {
				if (light->shadowMapCameras[i].get() == this)
				{
					return Scene::PointLight::PointLightDirection[i];
				}
			}
		}

		return {
			sinf(rotation.x) * cosf(rotation.y),
			sinf(rotation.y),
			cosf(rotation.x) * cosf(rotation.y)
		};
	}

	XMVECTOR Camera::CameraUp()
	{
		if (light != nullptr && light->lightType == LT_Point)
		{
			UINT i = 0U;
			for (; i < 6U; i++)
			{
				if (light->shadowMapCameras[i].get() == this)
				{
					return Scene::PointLight::PointLightUp[i];
				}
			}
		}

		return up;
	}

	void Camera::ProcessKeyboardInput(DirectX::Keyboard::KeyboardStateTracker& tracker, DirectX::Keyboard::State& state)
	{
		if (light == nullptr || light->lightType == LT_Spot || light->lightType == LT_Point)
		{
			float moveSpeed = speed * ((state.LeftShift || state.RightShift) ? 10.0f : 1.0f);
			if (state.Up) { MoveForward(moveSpeed); }
			if (state.Down) { MoveBack(moveSpeed); }
			if (state.Left) { MoveLeft(moveSpeed); }
			if (state.Right) { MoveRight(moveSpeed); }
		}
	}

	void Camera::ProcessGamepadInput(DirectX::GamePad::State& gamePadState, Vector2 gamePadCameraRotationSensitivity)
	{
		if (light == nullptr || light->lightType == LT_Spot || light->lightType == LT_Point)
		{
			if (gamePadState.thumbSticks.leftY > 0) { MoveForward(speed); }
			if (gamePadState.thumbSticks.leftY < 0) { MoveBack(speed); }
			if (gamePadState.thumbSticks.leftX < 0) { MoveLeft(speed); }
			if (gamePadState.thumbSticks.leftX > 0) { MoveRight(speed); }
		}
		if (light == nullptr || light->lightType == LT_Directional || light->lightType == LT_Spot)
		{
			Vector2 stickDiff = { gamePadState.thumbSticks.rightX, gamePadState.thumbSticks.rightY };
			rotation = rotation - XMFLOAT3{ stickDiff.x * gamePadCameraRotationSensitivity.x, stickDiff.y * gamePadCameraRotationSensitivity.y, 0.0f };
			UdateLightRotation();
		}
	}

	XMFLOAT2 lastMousePos;
	XMFLOAT2 GetMouseDiff(DirectX::Mouse::State& mouseState)
	{
		XMFLOAT2 currentMousePos = { static_cast<FLOAT>(mouseState.x) , static_cast<FLOAT>(mouseState.y) };
		XMFLOAT2 diff = { 0.0f, 0.0f };
		if (mouseState.leftButton)
		{
			diff = { currentMousePos.x - lastMousePos.x , currentMousePos.y - lastMousePos.y };
		}
		lastMousePos = currentMousePos;
		return diff;
	}

	INT lastWheelValue = 0;
	FLOAT wheelDiffFactor = 0.0001f;
	void Camera::ProcessMouseInput(DirectX::Mouse::State& mouseState, Vector2 rotationSensitivity)
	{
		if (light == nullptr || light->lightType == LT_Directional || light->lightType == LT_Spot)
		{
			Vector2 mouseDiff = GetMouseDiff(mouseState);
			rotation = rotation - XMFLOAT3{ mouseDiff.x * rotationSensitivity.x, mouseDiff.y * rotationSensitivity.y, 0.0f };
			UdateLightRotation();
			if (light != nullptr && light->lightType == LT_Directional)
			{
				float diff = static_cast<FLOAT>(mouseState.scrollWheelValue - lastWheelValue) * wheelDiffFactor;
				orthographic.expandView(diff);
			}
		}
	}

	void Camera::UpdateLightPosition()
	{
		if (light == nullptr) return;

		switch (light->lightType)
		{
		case LT_Spot:
		{
			light->spot.position = position;
		}
		break;
		case LT_Point:
		{
			light->point.position = position;
			for (auto cam : light->shadowMapCameras)
			{
				cam->position = position;
			}
		}
		break;
		}
	}

	void Camera::UdateLightRotation()
	{
		if (light == nullptr) return;

		switch (light->lightType)
		{
		case LT_Directional:
		{
			light->directional.rotation = { rotation.x, rotation.y };
			XMVECTOR camPos = XMVectorScale(XMVector3Normalize(CameraFw()), -light->directional.distance);
			position = *(XMFLOAT3*)camPos.m128_f32;
		}
		break;
		case LT_Spot:
		{
			light->spot.rotation = { rotation.x, rotation.y };
		}
		break;
		}
	}

	void Camera::MoveForward(float step)
	{
		XMVECTOR newPos = XMVectorAdd({ position.x ,position.y, position.z }, CameraFw() * step);
		position = { newPos.m128_f32[0], newPos.m128_f32[1], newPos.m128_f32[2] };
		UpdateLightPosition();
	}

	void Camera::MoveBack(float step)
	{
		XMVECTOR newPos = XMVectorAdd({ position.x ,position.y, position.z }, CameraFw() * -step);
		position = { newPos.m128_f32[0], newPos.m128_f32[1], newPos.m128_f32[2] };
		UpdateLightPosition();
	}

	void Camera::MoveLeft(float step)
	{
		XMVECTOR newPos = XMVectorAdd({ position.x ,position.y, position.z }, XMVector3Cross(CameraFw(), CameraUp()) * -step);
		position = { newPos.m128_f32[0], newPos.m128_f32[1], newPos.m128_f32[2] };
		UpdateLightPosition();
	}

	void Camera::MoveRight(float step)
	{
		XMVECTOR newPos = XMVectorAdd({ position.x ,position.y, position.z }, XMVector3Cross(CameraFw(), CameraUp()) * step);
		position = { newPos.m128_f32[0], newPos.m128_f32[1], newPos.m128_f32[2] };
		UpdateLightPosition();
	}

	size_t GetNumCameras() { return cameraByIndex.size(); }
	std::shared_ptr<Camera> GetCamera(unsigned int index) { return cameraByIndex[index]; }
	std::shared_ptr<Camera> GetCamera(std::string name) { return cameraByNames.at(name); }

#if defined(_EDITOR)
	nlohmann::json Camera::json()
	{
		nlohmann::json j = nlohmann::json({});

		j["name"] = name;
		j["fitWindow"] = fitWindow;
		j["projectionType"] = ProjectionsTypesStr[projectionType];

		auto buildOrthographicJsonCamera = [this](auto& j)
			{
				j["orthographic"]["nearZ"] = orthographic.nearZ;
				j["orthographic"]["farZ"] = orthographic.farZ;
				if (!fitWindow)
				{
					j["orthographic"]["width"] = orthographic.width;
					j["orthographic"]["height"] = orthographic.height;
				}
			};

		auto buildPerspectiveJsonCamera = [this](auto& j)
			{
				j["perspective"]["nearZ"] = perspective.nearZ;
				j["perspective"]["farZ"] = perspective.farZ;
				j["perspective"]["fovAngleY"] = perspective.fovAngleY;
				if (!fitWindow)
				{
					j["perspective"]["width"] = perspective.width;
					j["perspective"]["height"] = perspective.height;
				}
			};

		switch (projectionType)
		{
		case PROJ_Orthographic:
		{
			buildOrthographicJsonCamera(j);
		}
		break;
		case PROJ_Perspective:
		{
			buildPerspectiveJsonCamera(j);
		}
		break;
		}

		j["position"] = { position.x, position.y, position.z };
		j["rotation"] = { rotation.x, rotation.y };
		j["speed"] = speed;

		return j;
	}

	void Camera::DrawEditorInformationAttributes()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "camera-information-atts";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string currentName = name;
			if (ImGui::InputText("name", &currentName))
			{
				if (!cameraByNames.contains(currentName))
				{
					cameraByNames[currentName] = cameraByNames[name];
					cameraByNames.erase(name);
				}
				name = currentName;
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
			ImGui::InputFloat("x", &position.x);
			ImGui::TableSetColumnIndex(2);
			ImGui::InputFloat("y", &position.y);
			ImGui::TableSetColumnIndex(3);
			ImGui::InputFloat("z", &position.z);
			ImGui::PopID();

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::PushID("rotation");
			ImGui::Text("rotation");
			ImGui::TableSetColumnIndex(1);
			ImGui::SliderAngle("azimuthal", &rotation.x);
			ImGui::TableSetColumnIndex(2);
			ImGui::SliderAngle("polar", &rotation.y);
			ImGui::PopID();

			ImGui::EndTable();
		}
	}

	void Camera::DrawEditorCameraAttributes()
	{
		std::vector<std::string> selectables = ProjectionsTypesStr;
		std::string selected = ProjectionTypesToStr[projectionType];

		DrawComboSelection(selected, selectables, [this](std::string projection)
			{
				projectionType = StrToProjectionTypes[projection];
				switch (projectionType) {
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

		if (projectionType == PROJ_Perspective)
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
	}
#endif

	void Camera::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox)
	{
		bbox->position = position;
		bbox->scale = { 0.3f, 0.3f, 0.3f };
		bbox->rotation = { 0.0f, 0.0f, 0.0f };
	}
}