#pragma once

#include "Projections/Perspective.h"
#include "Projections/Orthographic.h"
#include <DeviceUtils/ConstantsBuffer/ConstantsBuffer.h>
#include <Keyboard.h>
#include <DirectXMath.h>
#include <Mouse.h>
#include <GamePad.h>
#include <map>
#include <Json.h>
#include <SceneObjectDecl.h>
#include <NoMath.h>
#include <SceneObject.h>
#include <JTypes.h>
#include <ImEditor.h>
#include <set>

enum ProjectionsTypes {
	PROJ_Orthographic,
	PROJ_Perspective
};

inline static std::vector<std::string> ProjectionsTypesStr = {
	"Orthographic",
	"Perspective"
};

inline static std::map<ProjectionsTypes, std::string> ProjectionsTypesToString = {
	{ PROJ_Orthographic, "Orthographic" },
	{ PROJ_Perspective, "Perspective" }
};

inline static std::map<std::string, ProjectionsTypes> StringToProjectionsTypes = {
	{ "Orthographic", PROJ_Orthographic },
	{ "Perspective", PROJ_Perspective }
};

struct CameraAttributes {
	XMMATRIX viewProjection;
	XMFLOAT4 eyePosition;
	XMFLOAT4 eyeForward;
	XMFLOAT4 eyeUp;
	XMFLOAT4 eyeRight;
	float white;
	float IBLNumEnvLevels;
};

namespace Templates { struct TextureInstance; struct RenderPassInstance; };
namespace DeviceUtils { struct RenderToTexturePass; };

using namespace Scene::CameraProjections;

namespace Scene {
#include <TrackUUID/JDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Attributes/JOrder.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

	struct Light;
	struct Renderable;
	struct Camera;

	using namespace DeviceUtils;
	using namespace Templates;

	inline static const std::string CameraConstantBufferName = "camera";

	void CamerasStep();
	void DestroyCameras();

#if defined(_EDITOR)
	void WriteCamerasJson(nlohmann::json& json);
#endif

	struct Camera : SceneObject
	{
		SCENEOBJECT_DECL(Camera);

#include <Attributes/JFlags.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <CameraAtt.h>
#include <JEnd.h>

		union {
			CameraProjections::Perspective perspectiveProjection;
			CameraProjections::Orthographic orthographicProjection;
		};

		XMVECTOR positionV();
		XMVECTOR rotationQ();
		XMVECTOR forward();
		XMVECTOR up();
		XMVECTOR right();
		XMMATRIX world();
		XMMATRIX view();
		XMMATRIX projection();

		float projectionWidth();
		float projectionHeight();
		float projectionNearZ();
		float projectionFarZ();
		float projectionfovAngleY();

		std::vector<std::shared_ptr<RenderPassInstance>> cameraRenderPasses;
		void CreateRenderPasses();
		void DestroyRenderPasses();
		void ResizeReleasePasses();
		void ResizePasses(unsigned int width, unsigned int height);
		void UpdateProjection();

		std::set<std::shared_ptr<Renderable>> renderables;
		virtual void Initialize();
		virtual void Bind(std::shared_ptr<SceneObject> sceneObject);
		virtual void Unbind(std::shared_ptr<SceneObject> sceneObject);
		void BindRenderable(std::shared_ptr<Renderable> renderable);
		void UnbindRenderable(std::shared_ptr<Renderable> renderable);

		void Render();

		std::shared_ptr<Scene::Light> lightCam = nullptr;
		std::shared_ptr<ConstantsBuffer> cameraCbv;
		std::map<TextureShaderUsage, std::shared_ptr<TextureInstance>> iblTextures;
		std::map<TextureShaderUsage, unsigned int> iblTexturesFlags;

		BoundingFrustum boundingFrustum;

		void Destroy();

		void CreateIBLTexturesInstances();
		void CreateConstantsBuffer();
		void WriteConstantsBuffer(unsigned int backbufferIndex);

		void ProcessKeyboardInput(DirectX::Keyboard::KeyboardStateTracker& tracker, DirectX::Keyboard::State& state);
		void MoveAlongFwAxis(float dz);
		void MovePerpendicularFwAxis(float dx, float dy);
		void Rotate(float dx, float dy);
		void ProcessGamepadInput(DirectX::GamePad::State& gamePadState, DirectX::SimpleMath::Vector2 gamePadCameraRotationSensitivity);
		void ProcessCameraMouseRotation(DirectX::Mouse::State& mouseState, DirectX::SimpleMath::Vector2 mouseCameraRotationSensitivity, bool firstStep);
		void UpdateLightPosition();
		void UdateLightRotation();
		void MoveForward(float step);
		void MoveBack(float step);
		void MoveLeft(float step);
		void MoveRight(float step);

		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);
		void SetIBLRootDescriptorTables(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot);
	};

};
