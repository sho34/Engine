#include "pch.h"
#include "Camera.h"
#include "../Lights/Lights.h"

extern RECT hWndRect;
extern std::shared_ptr<Renderer> renderer;

namespace Scene::Camera {

	using namespace DeviceUtils::ConstantsBuffer;

	std::vector<CameraPtr> cameraByIndex;
	std::map<std::wstring, CameraPtr> cameraByNames;

	std::shared_ptr<Camera> CreateCamera(const CameraDefinition& cameraDefinition) {
		CameraPtr camera = std::make_unique<CameraT>();
		if (cameraDefinition.name == L"") {
			camera->name = L"cam(" + std::to_wstring(cameraByIndex.size()) + L")";
		} else {
			camera->name = cameraDefinition.name;
		}
		assert(!cameraByNames.contains(camera->name));
		camera->projectionType = cameraDefinition.projectionType;
		switch (cameraDefinition.projectionType) {
		case ProjectionsTypes::Perspective:
			camera->perspective = cameraDefinition.perspective;
			break;
		case ProjectionsTypes::Orthographic:
			camera->orthographic = cameraDefinition.orthographic;
			break;
		default:
			assert(true); //not implemented
			break;
		}
		camera->position = cameraDefinition.position;
		camera->rotation = cameraDefinition.rotation;
		camera->light = cameraDefinition.light;

		cameraByIndex.push_back(camera);
		cameraByNames[camera->name] = camera;

		return camera;
	}

	std::shared_ptr<Camera> CreateCamera(nlohmann::json cameraDefinition)
	{
		CameraPtr camera = std::make_unique<CameraT>();
		if (cameraDefinition["name"] == "") {
			camera->name = L"cam." + std::to_wstring(cameraByIndex.size());
		} else {
			camera->name = StringToWString(cameraDefinition["name"]);
		}
		assert(!cameraByNames.contains(camera->name));

		replaceFromJson(camera->fitWindow,cameraDefinition,"fitWindow");
		camera->projectionType = StrToProjectionTypes[StringToWString(cameraDefinition["projectionType"])];
		switch (camera->projectionType) {
		case ProjectionsTypes::Perspective:
		{
			float winWidth = cameraDefinition["fitWindow"] ? static_cast<float>(hWndRect.right - hWndRect.left) : static_cast<float>(cameraDefinition["perspective"]["width"]);
			float winHeight = cameraDefinition["fitWindow"] ? static_cast<float>(hWndRect.bottom - hWndRect.top) : static_cast<float>(cameraDefinition["perspective"]["height"]);
			camera->perspective = {
				.nearZ = cameraDefinition["perspective"]["nearZ"],
				.farZ = cameraDefinition["perspective"]["farZ"],
				.fovAngleY = cameraDefinition["perspective"]["fovAngleY"],
				.width = winWidth,
				.height = winHeight,
			};
			camera->perspective.updateProjectionMatrix();
		}
		break;
		case ProjectionsTypes::Orthographic:
		{
			float winWidth = cameraDefinition["fitWindow"] ? static_cast<float>(hWndRect.right - hWndRect.left) : static_cast<float>(cameraDefinition["orthographic"]["width"]);
			float winHeight = cameraDefinition["fitWindow"] ? static_cast<float>(hWndRect.bottom - hWndRect.top) : static_cast<float>(cameraDefinition["orthographic"]["height"]);
			camera->orthographic = {
				.nearZ = cameraDefinition["orthographic"]["nearZ"],
				.farZ = cameraDefinition["orthographic"]["farZ"],
				.width = winWidth,
				.height = winHeight
			};
			camera->orthographic.updateProjectionMatrix();
		}
		break;
		default:
			assert(true); //not implemented
			break;
		}

		camera->position = { cameraDefinition["position"][0], cameraDefinition["position"][1], cameraDefinition["position"][2] };
		camera->rotation = { cameraDefinition["rotation"][0], cameraDefinition["rotation"][1] };
		camera->speed = cameraDefinition["speed"];

		/*
		camera->light = cameraDefinition.light;
		*/

		camera->CreateConstantsBufferView(renderer);

		cameraByIndex.push_back(camera);
		cameraByNames[camera->name] = camera;

		return camera;
	}

	std::vector<std::shared_ptr<Camera>> GetCameras() {
		return cameraByIndex;
	}

	void DestroyCameras()
	{
		for (auto& cam : cameraByIndex) {
			if (cam->cameraCbv) {
				cam->cameraCbv->constantBuffer.Release();
				cam->cameraCbv->constantBuffer = nullptr;
				cam->cameraCbv.reset();
				cam->cameraCbv = nullptr;
			}
			cam.reset();
			cam = nullptr;
		}
		cameraByIndex.clear();
		cameraByNames.clear();
	}

	void Camera::CreateConstantsBufferView(const std::shared_ptr<Renderer>& renderer)
	{
		cameraCbv = CreateConstantsBufferViewData(renderer, sizeof(CameraAttributes) * renderer->numFrames, L"camera");
	}

	void Camera::UpdateConstantsBuffer(UINT backbufferIndex)
	{

		CameraAttributes atts{};
		if (projectionType == Orthographic) {
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

	XMMATRIX Camera::ViewMatrix() {
		XMVECTOR viewUp = up;
		if (light != nullptr && light->lightType == Scene::Lights::LightType::Point) {
			UINT i = 0U;
			for (; i < 6U; i++) {
				if (light->shadowMapCameras[i].get() == this) {
					viewUp = Scene::Lights::PointLightUp[i]; break;
				}
			}
		}
		return XMMatrixLookToRH({ position.x ,position.y, position.z }, CameraFw(), viewUp);
	}

	XMVECTOR Camera::CameraFw() {
		if (light != nullptr && light->lightType == Scene::Lights::LightType::Point) {
			UINT i = 0U;
			for (; i < 6U; i++) {
				if (light->shadowMapCameras[i].get() == this) {
					return Scene::Lights::PointLightDirection[i];
				}
			}
		}

		return {
				sinf(rotation.x) * cosf(rotation.y),
				sinf(rotation.y),
				cosf(rotation.x) * cosf(rotation.y)
		};
	}

	XMVECTOR Camera::CameraUp() {
		if (light != nullptr && light->lightType == Scene::Lights::LightType::Point) {
			UINT i = 0U;
			for (; i < 6U; i++) {
				if (light->shadowMapCameras[i].get() == this) {
					return Scene::Lights::PointLightUp[i];
				}
			}
		}

		return up;
	}

	void Camera::ProcessKeyboardInput(DirectX::Keyboard::KeyboardStateTracker& tracker, DirectX::Keyboard::State& state) {
		if (light == nullptr || light->lightType == Lights::Spot || light->lightType == Lights::Point) {
			float moveSpeed = speed * ((state.LeftShift || state.RightShift) ? 10.0f : 1.0f);
			if (state.Up) { MoveForward(moveSpeed); }
			if (state.Down) { MoveBack(moveSpeed); }
			if (state.Left) { MoveLeft(moveSpeed); }
			if (state.Right) { MoveRight(moveSpeed); }
		}
	}

	void Camera::ProcessGamepadInput(DirectX::GamePad::State& gamePadState, Vector2 gamePadCameraRotationSensitivity) {
		if (light == nullptr || light->lightType == Lights::Spot || light->lightType == Lights::Point) {
			if (gamePadState.thumbSticks.leftY > 0) { MoveForward(speed); }
			if (gamePadState.thumbSticks.leftY < 0) { MoveBack(speed); }
			if (gamePadState.thumbSticks.leftX < 0) { MoveLeft(speed); }
			if (gamePadState.thumbSticks.leftX > 0) { MoveRight(speed); }
		}
		if (light == nullptr || light->lightType == Lights::Directional || light->lightType == Lights::Spot) {
			Vector2 stickDiff = { gamePadState.thumbSticks.rightX, gamePadState.thumbSticks.rightY };
			rotation = rotation - stickDiff * gamePadCameraRotationSensitivity;
			UdateLightRotation();
		}
	}

	XMFLOAT2 lastMousePos;
	XMFLOAT2 GetMouseDiff(DirectX::Mouse::State& mouseState) {
		XMFLOAT2 currentMousePos = { static_cast<FLOAT>(mouseState.x) , static_cast<FLOAT>(mouseState.y) };
		XMFLOAT2 diff = { 0.0f, 0.0f };
		if (mouseState.leftButton) {
			diff = { currentMousePos.x - lastMousePos.x , currentMousePos.y - lastMousePos.y };
		}
		lastMousePos = currentMousePos;
		return diff;
	}

	INT lastWheelValue = 0;
	FLOAT wheelDiffFactor = 0.0001f;
	void Camera::ProcessMouseInput(DirectX::Mouse::State& mouseState, Vector2 rotationSensitivity) {
		if (light == nullptr || light->lightType == Lights::Directional || light->lightType == Lights::Spot) {
			
			Vector2 mouseDiff = GetMouseDiff(mouseState);
			rotation = rotation - mouseDiff * rotationSensitivity;
			UdateLightRotation();

			if (light != nullptr && light->lightType == Lights::Directional) {

				float diff = static_cast<FLOAT>(mouseState.scrollWheelValue - lastWheelValue) * wheelDiffFactor;
				orthographic.expandView(diff);
			}

		}
	}

	void Camera::UpdateLightPosition() {
		if (light != nullptr) {
			switch (light->lightType) {
			case Lights::Spot: {
				light->spot.position = position;
			}
			break;
			case Lights::Point: {
				light->point.position = position;
				for (auto cam : light->shadowMapCameras) {
					cam->position = position;
				}
			}
			break;
			}
		}
	}

	void Camera::UdateLightRotation() {
		if (light != nullptr) {
			switch (light->lightType) {
			case Lights::Directional: {
				light->directional.rotation = rotation;
				XMVECTOR camPos = XMVectorScale(XMVector3Normalize(CameraFw()), -light->directional.distance);
				position = *(XMFLOAT3*)camPos.m128_f32;
			}
			break;
			case Lights::Spot: {
				light->spot.rotation = rotation;
			}
			break;
			}
		}
	}

	void Camera::MoveForward(float step) {
		XMVECTOR newPos = XMVectorAdd({ position.x ,position.y, position.z }, CameraFw() * step);
		position = { newPos.m128_f32[0], newPos.m128_f32[1], newPos.m128_f32[2] };
		UpdateLightPosition();
	}

	void Camera::MoveBack(float step) {
		XMVECTOR newPos = XMVectorAdd({ position.x ,position.y, position.z }, CameraFw() * -step);
		position = { newPos.m128_f32[0], newPos.m128_f32[1], newPos.m128_f32[2] };
		UpdateLightPosition();
	}

	void Camera::MoveLeft(float step) {
		XMVECTOR newPos = XMVectorAdd({ position.x ,position.y, position.z }, XMVector3Cross(CameraFw(), CameraUp()) * -step);
		position = { newPos.m128_f32[0], newPos.m128_f32[1], newPos.m128_f32[2] };
		UpdateLightPosition();
	}

	void Camera::MoveRight(float step) {
		XMVECTOR newPos = XMVectorAdd({ position.x ,position.y, position.z }, XMVector3Cross(CameraFw(), CameraUp()) * step);
		position = { newPos.m128_f32[0], newPos.m128_f32[1], newPos.m128_f32[2] };
		UpdateLightPosition();
	}

	size_t GetNumCameras() { return cameraByIndex.size(); }
	CameraPtr GetCamera(UINT index) { return cameraByIndex[index]; }
	CameraPtr GetCamera(std::wstring name) { return cameraByNames[name]; }

#if defined(_EDITOR)
	nlohmann::json Camera::json() {
		nlohmann::json j = nlohmann::json({});

		j["name"] = WStringToString(name);
		j["fitWindow"] = fitWindow;
		j["projectionType"] = WStringToString(ProjectionsTypesStr[projectionType]);

		auto buildOrthographicJsonCamera = [this](auto& j) {
			j["orthographic"]["nearZ"] = orthographic.nearZ;
			j["orthographic"]["farZ"] = orthographic.farZ;
			if (!fitWindow) {
				j["orthographic"]["width"] = orthographic.width;
				j["orthographic"]["height"] = orthographic.height;
			}
		};

		auto buildPerspectiveJsonCamera = [this](auto& j) {
			j["perspective"]["nearZ"] = perspective.nearZ;
			j["perspective"]["farZ"] = perspective.farZ;
			j["perspective"]["fovAngleY"] = perspective.fovAngleY;
			if (!fitWindow) {
				j["perspective"]["width"] = perspective.width;
				j["perspective"]["height"] = perspective.height;
			}
		};

		switch (projectionType) {
		case Orthographic:
			buildOrthographicJsonCamera(j);
			break;
		case Perspective:
			buildPerspectiveJsonCamera(j);
			break;
		}

		j["position"] = { position.x, position.y, position.z };
		j["rotation"] = { rotation.x, rotation.y };
		j["speed"] = speed;

		return j;
	}
#endif

}