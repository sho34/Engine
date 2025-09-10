#include "pch.h"
#include "Camera.h"
#include <Scene.h>
#include <Light/Light.h>
#include <Renderer.h>
#include <nlohmann/json.hpp>
#if defined(_EDITOR)
#include <Templates.h>
#include <Textures/Texture.h>
#endif
#include <Keyboard.h>
#include <GamePad.h>
#include <SimpleMath.h>
#include <Mouse.h>
#include <RenderPass/RenderPass.h>
#include <DeviceUtils/RenderToTexture/RenderToTexture.h>

using namespace DeviceUtils;

extern RECT hWndRect;
extern std::shared_ptr<Renderer> renderer;

namespace Scene
{
#include <Editor/JDrawersDef.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <TrackUUID/JDef.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <CameraAtt.h>
#include <JEnd.h>

	using namespace DeviceUtils;
	using namespace DirectX;

	Camera::Camera(nlohmann::json json) :SceneObject(json)
	{
#include <Attributes/JInit.h>
#include <CameraAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <CameraAtt.h>
#include <JEnd.h>
	}

	void CamerasStep()
	{
		std::set<std::shared_ptr<Camera>> cams;
		std::transform(Cameras.begin(), Cameras.end(), std::inserter(cams, cams.begin()), [](const auto& pair) { return pair.second; });

		std::set<std::shared_ptr<Camera>> camsRpi;
		std::copy_if(cams.begin(), cams.end(), std::inserter(camsRpi, camsRpi.begin()), [](const auto& cam)
			{
				return cam->dirty(Camera::Update_renderPasses);
			}
		);

		std::map<std::string, std::shared_ptr<Camera>> allCams(Cameras.begin(), Cameras.end());
		for (auto& pair : allCams)
		{
			auto [uuid, cam] = pair;
			if (!cam->dirty(Camera::Update_useSwapChain)) continue;
			cam->clean(Camera::Update_useSwapChain);
			if (cam->useSwapChain())
			{
				InsertCameraIntoSwapChainCameras(cam);
			}
			else
			{
				EraseCameraFromSwapChainCameras(cam);
			}
			camsRpi.insert(cam);
		}

		for (auto& pair : allCams)
		{
			auto [uuid, cam] = pair;
			if (!cam->dirty(Camera::Update_mouseController)) continue;
			cam->clean(Camera::Update_mouseController);
			if (cam->mouseController())
			{
				InsertCameraIntoMouseCameras(cam);
			}
			else
			{
				EraseCameraFromMouseCameras(cam);
			}
		}

		for (auto& pair : allCams)
		{
			auto [uuid, cam] = pair;
			if (
				!cam->dirty(Camera::Update_projectionType) &&
				!cam->dirty(Camera::Update_perspective) &&
				!cam->dirty(Camera::Update_orthographic) &&
				!cam->dirty(Camera::Update_fitWindow)
				) continue;
			cam->clean(Camera::Update_projectionType);
			cam->clean(Camera::Update_perspective);
			cam->clean(Camera::Update_orthographic);
			cam->clean(Camera::Update_fitWindow);
			cam->UpdateProjection();
		}

		if (camsRpi.size() > 0ULL)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([&camsRpi]
				{
					for (auto& c : camsRpi)
					{
						c->clean(Camera::Update_renderPasses);

						for (auto rpi : c->cameraRenderPasses)
						{
							if (rpi->renderCallbackOverride == RenderPassRenderCallbackOverride_Resolve)
							{
								EraseCameraFromSwapChainCameras(c);
								break;
							}
						}

						for (auto& renderable : c->renderables)
						{
							renderable->DestroyMaterialsInstances(c);
							renderable->DestroyConstantsBuffersInstances(c);
							renderable->DestroyRootSignatures(c);
							renderable->DestroyPipelineStates(c);
						}
						c->DestroyRenderPasses();

						c->CreateRenderPasses();
						for (auto& renderable : c->renderables)
						{
							renderable->CreateMaterialsInstances(c);
							renderable->CreateConstantsBuffersInstances(c);
							renderable->CreateRootSignatures(c);
							renderable->CreatePipelineStates(c);
						}

						for (auto rpi : c->cameraRenderPasses)
						{
							if (rpi->renderCallbackOverride == RenderPassRenderCallbackOverride_Resolve)
							{
								InsertCameraIntoSwapChainCameras(c);
								break;
							}
						}
					}
				}
			);
		}
	}

	void DestroyCameras()
	{
		auto cams = Cameras;
		for (auto& [_, cam] : cams)
		{
			if (cam->light().empty())
				SafeDeleteSceneObject(cam);
		}
#include <TrackUUID/JClear.h>
#include <CameraAtt.h>
#include <JEnd.h>
	}

#if defined(_EDITOR)
	void WriteCamerasJson(nlohmann::json& json)
	{
		//std::map<std::string, std::shared_ptr<Camera>> filtered;
		//std::copy_if(cameraByUUID.begin(), cameraByUUID.end(), std::inserter(filtered, filtered.end()), [](const auto& pair)
		//	{
		//		auto& [uuid, camera] = pair;
		//		return !camera->hidden() && camera->light == nullptr;
		//	}
		//);
		//std::transform(filtered.begin(), filtered.end(), std::back_inserter(json), [](const auto& pair)
		//	{
		//		auto& [uuid, camera] = pair;
		//		nlohmann::json ret = camera->json;
		//		ret["uuid"] = uuid;
		//		return ret;
		//	}
		//);
	}
#endif

	XMVECTOR Camera::positionV()
	{
		XMFLOAT3 pos = position();
		return { pos.x,pos.y,pos.z,0.0f };
	}

	XMVECTOR Camera::rotationQ()
	{
		XMFLOAT3 rotV = rotation();
		float roll, pitch, yaw;
		pitch = rotV.x; yaw = rotV.y; roll = rotV.z;
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
		return rotQ;
	}

	XMVECTOR Camera::forward()
	{
		if (lightCam != nullptr && lightCam->lightType() == LT_Point)
		{
			unsigned int i = 0U;
			for (; i < 6U; i++) {
				if (lightCam->shadowMapCameras[i].get() == this)
				{
					return Scene::PointLightDirection[i];
				}
			}
		}

		FXMVECTOR dir = { 0.0f, 0.0f, 1.0f,0.0f };
		return XMVector3Normalize(XMVector3Rotate(dir, rotationQ()));
	}

	XMVECTOR Camera::up()
	{
		if (lightCam != nullptr && lightCam->lightType() == LT_Point)
		{
			unsigned int i = 0U;
			for (; i < 6U; i++)
			{
				if (lightCam->shadowMapCameras[i].get() == this)
				{
					return Scene::PointLightUp[i];
				}
			}
		}

		FXMVECTOR up = { 0.0f, 1.0f, 0.0f,0.0f };
		return XMVector3Normalize(XMVector3Rotate(up, rotationQ()));
	}

	XMVECTOR Camera::right()
	{
		return XMVector3Cross(forward(), up());
	}

	XMMATRIX Camera::world()
	{
		XMFLOAT3 posV = position();
		XMFLOAT3 rotV = rotation();
		float roll, pitch, yaw;
		pitch = rotV.x; yaw = rotV.y; roll = rotV.z;
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
		XMMATRIX rotationM = XMMatrixRotationQuaternion(rotQ);
		XMMATRIX positionM = XMMatrixTranslationFromVector({ posV.x, posV.y, posV.z });
		return XMMatrixMultiply(rotationM, positionM);
	}

	XMMATRIX Camera::view()
	{
		return XMMatrixLookToLH(positionV(), forward(), up());
	}

	XMMATRIX Camera::projection()
	{
		return (projectionType() == PROJ_Orthographic) ? orthographicProjection.projectionMatrix : perspectiveProjection.projectionMatrix;
	}

	float Camera::projectionWidth()
	{
		switch (projectionType())
		{
		case PROJ_Perspective:
		{
			return fitWindow() ? static_cast<float>(abs(hWndRect.right - hWndRect.left)) : perspective().width;
		}
		break;
		default:
			return fitWindow() ? static_cast<float>(abs(hWndRect.right - hWndRect.left)) : orthographic().width;
			break;
		}
	}

	float Camera::projectionHeight()
	{
		switch (projectionType())
		{
		case PROJ_Perspective:
			return  fitWindow() ? static_cast<float>(abs(hWndRect.top - hWndRect.bottom)) : perspective().height;
			break;
		default:
			return  fitWindow() ? static_cast<float>(abs(hWndRect.top - hWndRect.bottom)) : orthographic().height;
			break;
		}
	}

	float Camera::projectionNearZ()
	{
		return (projectionType() == PROJ_Perspective) ? perspective().nearZ : orthographic().nearZ;
	}

	float Camera::projectionFarZ()
	{
		return (projectionType() == PROJ_Perspective) ? perspective().farZ : orthographic().farZ;
	}

	float Camera::projectionfovAngleY()
	{
		return (projectionType() == PROJ_Perspective) ? perspective().fovAngleY : 0.0f;
	}

	void Camera::CreateRenderPasses()
	{
		using namespace Templates;

		for (unsigned int i = 0; i < renderPasses().size(); i++)
		{
			std::string passUUID = renderPasses().at(i);
			if (passUUID == "") continue;
			std::shared_ptr<RenderPassJson> rp = GetRenderPassTemplate(passUUID);
			if (rp->type() == RenderPassType_SwapChainPass && rp->renderCallbackOverride() != RenderPassRenderCallbackOverride_Resolve) continue;

			cameraRenderPasses.push_back(
				GetRenderPassInstance(
					this_ptr,
					i,
					passUUID,
					static_cast<unsigned int>(projectionWidth()),
					static_cast<unsigned int>(projectionHeight())
				)
			);
		}
	}

	void Camera::DestroyRenderPasses()
	{
		for (auto& rp : cameraRenderPasses)
		{
			DestroyRenderPassInstance(rp);
			rp = nullptr;
		}
		cameraRenderPasses.clear();
	}

	void Camera::ResizeReleasePasses()
	{
		for (auto& pass : cameraRenderPasses)
		{
			pass->ResizeRelease();
		}
	}

	void Camera::ResizePasses(unsigned int width, unsigned int height)
	{
		for (auto& pass : cameraRenderPasses)
		{
			pass->Resize(width, height);
		}
		switch (projectionType())
		{
		case PROJ_Perspective:
		{
			perspectiveProjection.updateProjectionMatrix(static_cast<float>(width), static_cast<float>(height));
		}
		break;
		case PROJ_Orthographic:
		{
			orthographicProjection.updateProjectionMatrix(static_cast<float>(width), static_cast<float>(height));
		}
		break;
		}
	}

	void Camera::UpdateProjection()
	{
		switch (projectionType())
		{
		case PROJ_Perspective:
		{
			perspectiveProjection = {
				.nearZ = projectionNearZ(), .farZ = projectionFarZ(),
				.fovAngleY = projectionfovAngleY(), .width = projectionWidth(), .height = projectionHeight()
			};
			perspectiveProjection.updateProjectionMatrix();
		}
		break;
		case PROJ_Orthographic:
		{
			orthographicProjection = {
				.nearZ = projectionNearZ(), .farZ = projectionFarZ(),
				.width = projectionWidth(), .height = projectionHeight()
			};
			orthographicProjection.updateProjectionMatrix();
		}
		break;
		default:
		{
			assert(true); //not implemented
		}
		break;
		}
	}

	void Camera::Initialize()
	{
#include <TrackUUID/JInsert.h>
#include <CameraAtt.h>
#include <JEnd.h>

		if (!light().empty()) {
			lightCam = FindInLights(light());
		}

		UpdateProjection();
		CreateIBLTexturesInstances();
		CreateConstantsBuffer();
		CreateRenderPasses();
	}

	void Camera::Bind(std::shared_ptr<SceneObject> sceneObject)
	{
		switch (sceneObject->JType())
		{
		case SO_Renderables:
		{
			std::shared_ptr<Renderable> r = std::dynamic_pointer_cast<Renderable>(sceneObject);
			BindRenderable(r);
		}
		break;
		}
	}

	void Camera::Unbind(std::shared_ptr<SceneObject> sceneObject)
	{
		switch (sceneObject->JType())
		{
		case SO_Renderables:
		{
			std::shared_ptr<Renderable> r = std::dynamic_pointer_cast<Renderable>(sceneObject);
			UnbindRenderable(r);
		}
		break;
		}
	}

	void Camera::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <CameraAtt.h>
#include <JEnd.h>
	}

	void Camera::BindRenderable(std::shared_ptr<Renderable> r)
	{
		renderables.insert(r);
		r->CreateMaterialsInstances(this_ptr);
		r->CreateConstantsBuffersInstances(this_ptr);
		r->CreateRootSignatures(this_ptr);
		r->CreatePipelineStates(this_ptr);
	}

	void Camera::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <CameraAtt.h>
#include <JEnd.h>

		lightCam = nullptr;
		Scene::UnbindFromScene(this_ptr);
		DestroyRenderPasses();
		DestroyConstantsBuffer(cameraCbv);
	}

	void Camera::UnbindRenderable(std::shared_ptr<Renderable> r)
	{
		if (!renderables.contains(r)) return;
		renderables.erase(r);
		r->DestroyMaterialsInstances(this_ptr);
		r->DestroyConstantsBuffersInstances(this_ptr);
		r->DestroyRootSignatures(this_ptr);
		r->DestroyPipelineStates(this_ptr);
	}

	void Camera::Render()
	{
		WriteConstantsBuffer(renderer->backBufferIndex);

		auto draw = [this](std::shared_ptr<RenderPassInstance> rp)
			{
				for (auto r : renderables)
				{
					r->Render(rp, this_ptr);
				}
			};

		std::vector<std::shared_ptr<RenderPassInstance>> rpiv = cameraRenderPasses;
		if (useSwapChain())
		{
			rpiv.push_back(renderer->swapChainPass);
		}

		for (auto& rp : rpiv)
		{
			rp->Pass([rp, draw]() {draw(rp); });
		}
	}

	void Camera::Destroy()
	{

	}

	void Camera::CreateIBLTexturesInstances()
	{
		//if (std::any_of(iblJsonTextures.begin(), iblJsonTextures.end(), [this](auto& tup)
		//	{
		//		std::string& attName = std::get<0>(tup);
		//		return json.at(attName) == "";
		//	}
		//))
		//	return;
		//
		//std::map<TextureShaderUsage, std::string> textures;
		//std::transform(iblJsonTextures.begin(), iblJsonTextures.end(), std::inserter(textures, textures.end()), [this](auto& tup)
		//	{
		//		std::string& attName = std::get<0>(tup);
		//		TextureShaderUsage& usage = std::get<1>(tup);
		//		return std::pair<TextureShaderUsage, std::string>(usage, json.at(attName));
		//	}
		//);
		//
		//iblTextures = GetTextures(textures);
	}

	void Camera::CreateConstantsBuffer()
	{
		cameraCbv = DeviceUtils::CreateConstantsBuffer(sizeof(CameraAttributes) * renderer->numFrames, name());
	}

	void Camera::WriteConstantsBuffer(unsigned int backbufferIndex)
	{
		CameraAttributes atts{};

		atts.viewProjection = XMMatrixMultiply(view(), projection());

		XMVECTOR camPos = positionV();
		XMVECTOR camFw = forward();
		XMVECTOR camUp = up();
		XMVECTOR camRight = right();

		atts.eyePosition = *(XMFLOAT4*)camPos.m128_f32;
		atts.eyeForward = *(XMFLOAT4*)camFw.m128_f32;
		atts.eyeUp = *(XMFLOAT4*)camUp.m128_f32;
		atts.eyeRight = *(XMFLOAT4*)camRight.m128_f32;
		atts.white = white();

		if (iblTextures.contains(TextureShaderUsage_IBLPreFilteredEnvironment))
		{
			std::shared_ptr<TextureJson> tex = GetTextureTemplate(iblTextures.at(TextureShaderUsage_IBLPreFilteredEnvironment)->materialTexture);
			atts.IBLNumEnvLevels = static_cast<float>(tex->mipLevels());
		}
		else
		{
			atts.IBLNumEnvLevels = 0.0f;
		}

		memcpy(cameraCbv->mappedConstantBuffer + cameraCbv->alignedConstantBufferSize * backbufferIndex, &atts, sizeof(atts));
	}

	void Camera::ProcessKeyboardInput(DirectX::Keyboard::KeyboardStateTracker& tracker, DirectX::Keyboard::State& state)
	{
		if (lightCam == nullptr || lightCam->lightType() == LT_Spot || lightCam->lightType() == LT_Point)
		{
			float moveSpeed = speed() * ((state.LeftShift || state.RightShift) ? 10.0f : 1.0f);
			if (state.Up) { MoveForward(moveSpeed); }
			if (state.Down) { MoveBack(moveSpeed); }
			if (state.Left) { MoveLeft(moveSpeed); }
			if (state.Right) { MoveRight(moveSpeed); }
		}
	}

	void Camera::MoveAlongFwAxis(float dz)
	{
		XMVECTOR newPos = positionV() + forward() * dz;
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::MovePerpendicularFwAxis(float dx, float dy)
	{
		XMVECTOR newPos = positionV() + up() * dy + right() * dx;
		position(*(XMFLOAT3*)newPos.m128_f32);
	}

	void Camera::Rotate(float dx, float dy)
	{
		rotation(rotation() + XMFLOAT3{ dy, dx, 0.0f });
		UdateLightRotation();
	}

	void Camera::ProcessGamepadInput(DirectX::GamePad::State& gamePadState, DirectX::SimpleMath::Vector2 gamePadCameraRotationSensitivity)
	{
		if (lightCam == nullptr || lightCam->lightType() == LT_Spot || lightCam->lightType() == LT_Point)
		{
			if (gamePadState.thumbSticks.leftY > 0) { MoveForward(speed()); }
			if (gamePadState.thumbSticks.leftY < 0) { MoveBack(speed()); }
			if (gamePadState.thumbSticks.leftX < 0) { MoveLeft(speed()); }
			if (gamePadState.thumbSticks.leftX > 0) { MoveRight(speed()); }
		}
		if (lightCam == nullptr || lightCam->lightType() == LT_Directional || lightCam->lightType() == LT_Spot)
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
	void Camera::ProcessCameraMouseRotation(DirectX::Mouse::State& mouseState, DirectX::SimpleMath::Vector2 rotationSensitivity, bool firstStep)
	{
		if (lightCam == nullptr || lightCam->lightType() == LT_Directional || lightCam->lightType() == LT_Spot)
		{
			Vector2 mouseDiff = GetMouseDiff(mouseState);
			mouseDiff = firstStep ? Vector2(0.0f, 0.0f) : mouseDiff;
			rotation(rotation() - XMFLOAT3{ mouseDiff.x * rotationSensitivity.x, mouseDiff.y * rotationSensitivity.y, 0.0f });
			UdateLightRotation();
			if (lightCam != nullptr && lightCam->lightType() == LT_Directional)
			{
				float diff = static_cast<float>(mouseState.scrollWheelValue - lastWheelValue) * wheelDiffFactor;
				orthographicProjection.expandView(diff);
			}
			else if (lightCam == nullptr)
			{
				float diff = static_cast<float>(mouseState.scrollWheelValue - lastWheelValue) * wheelDiffFactor;
			}
		}
	}

	void Camera::UpdateLightPosition()
	{
		if (lightCam == nullptr) return;

		switch (lightCam->lightType())
		{
		case LT_Spot:
		{
			lightCam->position(position());
		}
		break;
		case LT_Point:
		{
			lightCam->position(position());
			for (auto cam : lightCam->shadowMapCameras)
			{
				cam->position(position());
			}
		}
		break;
		}
	}

	void Camera::UdateLightRotation()
	{
		if (lightCam == nullptr) return;

		XMFLOAT3 rot = rotation();

		switch (lightCam->lightType())
		{
		case LT_Directional:
		{
			lightCam->rotation(rot);
			XMVECTOR camPos = XMVectorScale(XMVector3Normalize(forward()), lightCam->dirDist());
			position(*(XMFLOAT3*)camPos.m128_f32);
		}
		break;
		case LT_Spot:
		{
			lightCam->rotation(rot);
		}
		break;
		}
	}

	void Camera::MoveForward(float step)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), forward() * step);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::MoveBack(float step)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), forward() * -step);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::MoveLeft(float step)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), XMVector3Cross(forward(), up()) * -step);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::MoveRight(float step)
	{
		XMVECTOR newPos = XMVectorAdd(positionV(), XMVector3Cross(forward(), up()) * step);
		position(*(XMFLOAT3*)newPos.m128_f32);
		UpdateLightPosition();
	}

	void Camera::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox)
	{
		//bbox->position(json.at("position"));
		//bbox->scale(XMFLOAT3({ 0.3f, 0.3f, 0.3f }));
		//bbox->rotation(XMFLOAT3({ 0.0f, 0.0f, 0.0f }));
	}

	void Camera::SetIBLRootDescriptorTables(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot)
	{
		//commandList->SetGraphicsRootDescriptorTable(cbvSlot++, iblTextures.at(TextureShaderUsage_IBLIrradiance)->gpuHandle);
		//commandList->SetGraphicsRootDescriptorTable(cbvSlot++, iblTextures.at(TextureShaderUsage_IBLPreFilteredEnvironment)->gpuHandle);
		//commandList->SetGraphicsRootDescriptorTable(cbvSlot++, iblTextures.at(TextureShaderUsage_IBLBRDFLUT)->gpuHandle);
	}
}