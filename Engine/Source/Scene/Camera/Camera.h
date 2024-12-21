#pragma once

#include <DirectXMath.h>
#include "Projections/Perspective.h"
#include "Projections/Orthographic.h"
#include "../../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace DeviceUtils::ConstantsBuffer;

namespace Scene::Lights { struct Light; };
namespace Scene::Camera {

  static const std::wstring CameraConstantBufferName = L"camera";
  static XMVECTOR	up = { 0.0f, 1.0f, 0.0f, 0.0f };
  static XMVECTOR	right = { 1.0f, 0.0f, 0.0f, 0.0f };

  enum ProjectionsTypes {
    Cylindrical,
    Fisheye,
    Omnimax,
    Orthographic,
    Panoramic,
    Perspective,
    Spherical,
    Ultrawite
  };

  static std::vector<std::wstring> ProjectionsTypesStr = {
    L"Cylindrical",
    L"Fisheye",
    L"Omnimax",
    L"Orthographic",
    L"Panoramic",
    L"Perspective",
    L"Spherical",
    L"Ultrawite"
  };

  static std::map<ProjectionsTypes, std::wstring> ProjectionTypesToStr = {
    { Cylindrical, L"Cylindrical" },
    { Fisheye, L"Fisheye" },
    { Omnimax, L"Omnimax" },
    { Orthographic, L"Orthographic" },
    { Panoramic, L"Panoramic" },
    { Perspective, L"Perspective" },
    { Spherical, L"Spherical" },
    { Ultrawite, L"Ultrawite" }
  };

  static std::map<std::wstring, ProjectionsTypes> StrToProjectionTypes = {
    { L"Cylindrical", Cylindrical },
    { L"Fisheye", Fisheye },
    { L"Omnimax", Omnimax },
    { L"Orthographic", Orthographic },
    { L"Panoramic", Panoramic },
    { L"Perspective", Perspective },
    { L"Spherical", Spherical },
    { L"Ultrawite", Ultrawite }
  };

  struct CameraAttributes {
    XMMATRIX viewProjection;
    XMFLOAT3 eyePosition;
    XMFLOAT3 eyeForward;
  };

  struct CameraDefinition {
    std::wstring name = L"";
    ProjectionsTypes projectionType = Perspective;
    Projections::Perspective perspective;
    Projections::Orthographic orthographic;
    XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
    XMFLOAT2 rotation = { 0.0f, 0.0f };
    FLOAT speed = 0.05f;
    std::shared_ptr<Scene::Lights::Light> light = nullptr;
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

  std::shared_ptr<Camera> CreateCamera(const CameraDefinition& cameraDefinition);
  std::shared_ptr<Camera> CreateCamera(nlohmann::json cameraDefinition);
  std::vector<std::shared_ptr<Camera>> GetCameras();
  void DestroyCameras();
  size_t GetNumCameras();
  std::shared_ptr<Camera> GetCamera(UINT index);
  std::shared_ptr<Camera> GetCamera(std::wstring name);
};

typedef Scene::Camera::Camera CameraT;
typedef std::shared_ptr<CameraT> CameraPtr;