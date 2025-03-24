#pragma once

#include "Projections/Perspective.h"
#include "Projections/Orthographic.h"
#include "../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"

namespace Scene {

	struct Light;
	struct Renderable;

#if defined(_EDITOR)
	enum CameraFlags
	{
		CameraFlags_Destroy = 0x1
	};
#endif

	using namespace DeviceUtils;

	inline static const std::string CameraConstantBufferName = "camera";
	inline static XMVECTOR	up = { 0.0f, 1.0f, 0.0f, 0.0f };
	inline static XMVECTOR	right = { 1.0f, 0.0f, 0.0f, 0.0f };

	enum ProjectionsTypes {
		PROJ_Orthographic,
		PROJ_Perspective
	};

	inline static std::vector<std::string> ProjectionsTypesStr = {
		"Orthographic",
		"Perspective"
	};

	inline static std::map<ProjectionsTypes, std::string> ProjectionTypesToStr = {
		{ PROJ_Orthographic, "Orthographic" },
		{ PROJ_Perspective, "Perspective" }
	};

	inline static std::map<std::string, ProjectionsTypes> StrToProjectionTypes = {
		{ "Orthographic", PROJ_Orthographic },
		{ "Perspective", PROJ_Perspective }
	};

	struct CameraAttributes {
		XMMATRIX viewProjection;
		XMFLOAT3 eyePosition;
		XMFLOAT3 eyeForward;
	};

	struct Camera
	{
		Camera() : perspective{}
		{}
		~Camera() {
			Destroy();
		}
		std::string name();
		void name(std::string name);

		std::shared_ptr<Camera> this_ptr = nullptr; //dumb but efective

		nlohmann::json json;

		union {
			CameraProjections::Perspective perspective;
			CameraProjections::Orthographic orthographic;
		};
#if defined(_EDITOR)
		unsigned int cameraUpdateFlags = 0U;
		CameraProjections::Perspective editorPerspective;
		CameraProjections::Orthographic editorOrthographic;
#endif

		XMFLOAT3 position();
		XMVECTOR positionV();
		void position(XMFLOAT3 f3);
		void position(nlohmann::json f3);

		XMFLOAT3 rotation();
		void rotation(XMFLOAT3 f3);
		void rotation(nlohmann::json f3);

		std::shared_ptr<Scene::Light> light = nullptr;
		std::shared_ptr<ConstantsBuffer> cameraCbv;
		BoundingFrustum boundingFrustum;

		void Destroy();

		void CreateConstantsBuffer();
		void WriteConstantsBuffer(unsigned int backbufferIndex);

		XMMATRIX ViewMatrix();
		XMVECTOR CameraFw();
		XMVECTOR CameraUp();

		void ProcessKeyboardInput(DirectX::Keyboard::KeyboardStateTracker& tracker, DirectX::Keyboard::State& state);
		void ProcessGamepadInput(DirectX::GamePad::State& gamePadState, Vector2 gamePadCameraRotationSensitivity);
		void ProcessMouseInput(DirectX::Mouse::State& mouseState, Vector2 mouseCameraRotationSensitivity);
		void UpdateLightPosition();
		void UdateLightRotation();
		void MoveForward(float step);
		void MoveBack(float step);
		void MoveLeft(float step);
		void MoveRight(float step);

#if defined(_EDITOR)
		//nlohmann::json json();
		void DrawEditorInformationAttributes();
		void DrawEditorWorldAttributes();
		void DrawEditorCameraAttributes();
		void BindDestruction(std::function<void()>);
#endif

		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);
	};

	std::shared_ptr<Camera> CreateCamera(nlohmann::json cameraj);
	std::vector<std::shared_ptr<Camera>> GetCameras();
	std::vector<std::string> GetCamerasNames();
#if defined(_EDITOR)
	void SelectCamera(std::string cameraName, void*& ptr);
	void DeSelectCamera(void*& ptr);
	void DrawCameraPanel(void*& ptr, ImVec2 pos, ImVec2 size, bool pop);
	std::string GetCameraName(void* ptr);
	void DeleteCamera(std::string name);
#endif
	void CamerasStep();
	void DestroyCamera(std::shared_ptr<Camera>& camera);
	void DestroyCameras();

	size_t GetNumCameras();
	std::shared_ptr<Camera> GetCamera(unsigned int index);
	std::shared_ptr<Camera> GetCamera(std::string name);

};
