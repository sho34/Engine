#pragma once

#include "Projections/Perspective.h"
#include "Projections/Orthographic.h"
#include "../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"

using namespace DeviceUtils::ConstantsBuffer;

namespace Scene::Lights { struct Light; };
namespace Scene::Camera {

  static const std::wstring CameraConstantBufferName = L"camera";
  static XMVECTOR	up = { 0.0f, 1.0f, 0.0f, 0.0f };
  static XMVECTOR	right = { 1.0f, 0.0f, 0.0f, 0.0f };

  enum ProjectionsTypes {
    Orthographic,
    Perspective
  };

  static std::vector<std::wstring> ProjectionsTypesStr = {
    L"Orthographic",
    L"Perspective"
  };

  static std::map<ProjectionsTypes, std::wstring> ProjectionTypesToStr = {
    { Orthographic, L"Orthographic" },
    { Perspective, L"Perspective" }
  };

  static std::map<std::wstring, ProjectionsTypes> StrToProjectionTypes = {
    { L"Orthographic", Orthographic },
    { L"Perspective", Perspective }
  };

  struct CameraAttributes {
    XMMATRIX viewProjection;
    XMFLOAT3 eyePosition;
    XMFLOAT3 eyeForward;
  };

  struct Camera
  {
    std::wstring name = L"";
    bool fitWindow = false;
    ProjectionsTypes projectionType = Perspective;
    union {
      Projections::Perspective perspective;
      Projections::Orthographic orthographic;
    };
    XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
    XMFLOAT2 rotation = { 0.0f, 0.0f };
    FLOAT speed = 0.05f;
    std::shared_ptr<Scene::Lights::Light> light = nullptr;

    ConstantsBufferViewDataPtr cameraCbv;

    Camera() :
      perspective{}
    {};
    ~Camera() { cameraCbv = nullptr; }

    void CreateConstantsBufferView(const std::shared_ptr<Renderer>& renderer);
    void UpdateConstantsBuffer(UINT backbufferIndex);

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
    nlohmann::json json();
#endif
  };

  std::shared_ptr<Camera> CreateCamera(nlohmann::json cameraDefinition);
  std::vector<std::shared_ptr<Camera>> GetCameras();
  std::map<std::wstring,std::shared_ptr<Camera>> GetNamedCameras();
  void DestroyCameras();
  size_t GetNumCameras();
  std::shared_ptr<Camera> GetCamera(UINT index);
  std::shared_ptr<Camera> GetCamera(std::wstring name);
};

typedef Scene::Camera::Camera CameraT;
typedef std::shared_ptr<CameraT> CameraPtr;