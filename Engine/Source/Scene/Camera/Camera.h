#pragma once

#include "Projections/Perspective.h"
#include "Projections/Orthographic.h"
#include "../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include <Keyboard.h>
#include <SimpleMath.h>
#include <Mouse.h>
#include <GamePad.h>
#include <map>

namespace Templates
{
	struct TextureInstance;
};

namespace Scene {

	struct Light;
	struct Renderable;

#if defined(_EDITOR)
	enum CameraFlags
	{
		CameraFlags_Destroy = 0x1
	};

	enum Camera_PopupModal
	{
		CameraPopupModal_CannotDelete = 1,
		CameraPopupModal_CreateNew = 2
	};

	enum CameraIBLTextureFlags
	{
		CameraIBLTextureFlags_Create = 0x1,
		CameraIBLTextureFlags_Destroy = 0x2,
		CameraIBLTextureFlags_Reload = CameraIBLTextureFlags_Create | CameraIBLTextureFlags_Destroy
	};
#endif

	using namespace DeviceUtils;
	using namespace Templates;

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
		XMFLOAT4 eyePosition;
		XMFLOAT4 eyeForward;
		float white;
	};

	struct Camera
	{
		Camera() : perspective{}
		{}
		~Camera() {
			Destroy();
		}

		std::string uuid();
		void uuid(std::string uuid);

		std::string name();
		void name(std::string name);

		std::shared_ptr<Camera> this_ptr = nullptr; //dumb but efective

		nlohmann::json json;

		union {
			CameraProjections::Perspective perspective;
			CameraProjections::Orthographic orthographic;
		};
#if defined(_EDITOR)
		static nlohmann::json creationJson;
		static unsigned int popupModalId;
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

		float white();
		void white(float f);

		float minLogLuminance();
		void minLogLuminance(float f);

		float maxLogLuminance();
		void maxLogLuminance(float f);

		float tau();
		void tau(float f);

		bool hidden() { return false; }

		XMMATRIX world();

		std::shared_ptr<Scene::Light> light = nullptr;
		std::shared_ptr<ConstantsBuffer> cameraCbv;
		std::map<TextureShaderUsage, std::shared_ptr<TextureInstance>> iblTextures;
		std::map<TextureShaderUsage, unsigned int> iblTexturesFlags;

		BoundingFrustum boundingFrustum;

		void Destroy();

		void CreateIBLTexturesInstances();
		void CreateConstantsBuffer();
		void WriteConstantsBuffer(unsigned int backbufferIndex);

		XMMATRIX ViewMatrix();
		XMVECTOR CameraFw();
		XMVECTOR CameraUp();

		void ProcessKeyboardInput(DirectX::Keyboard::KeyboardStateTracker& tracker, DirectX::Keyboard::State& state);
		void MoveAlongFwAxis(float dz);
		void MovePerpendicularFwAxis(float dx, float dy);
		void Rotate(float dx, float dy);
		void ProcessGamepadInput(DirectX::GamePad::State& gamePadState, Vector2 gamePadCameraRotationSensitivity);
		void ProcessCameraMouseRotation(DirectX::Mouse::State& mouseState, Vector2 mouseCameraRotationSensitivity, bool firstStep);
		void UpdateLightPosition();
		void UdateLightRotation();
		void MoveForward(float step);
		void MoveBack(float step);
		void MoveLeft(float step);
		void MoveRight(float step);

#if defined(_EDITOR)
		void DrawEditorInformationAttributes();
		void DrawEditorWorldAttributes();
		void DrawEditorCameraAttributes();
		void DrawEditorIBLAttributes();
		void BindDestruction(std::function<void()>);
#endif

		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);
	};

	std::shared_ptr<Camera> CreateCamera(nlohmann::json cameraj);
	std::vector<std::shared_ptr<Camera>> GetCameras();
	std::vector<std::string> GetCamerasNames();
	std::vector<UUIDName> GetCamerasUUIDNames();
#if defined(_EDITOR)
	void SelectCamera(std::string uuid, std::string& edSO);
	void DeSelectCamera(std::string& edSO);
	void DrawCameraPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop);
	std::string GetCameraName(std::string uuid);
	void CreateNewCamera();
	void DeleteCamera(std::string uuid);
	void DrawCamerasPopups();
	void WriteCamerasJson(nlohmann::json& json);
#endif
	void CamerasStep();
	void DestroyCamera(std::shared_ptr<Camera>& camera);
	void DestroyCameras();

	size_t GetNumCameras();
	std::shared_ptr<Camera> GetCamera(unsigned int index);
	std::shared_ptr<Camera> GetCamera(std::string uuid);
	std::shared_ptr<Camera> GetCameraByName(std::string name);
};
