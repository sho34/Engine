#include "pch.h"
#include "VenomController.h"
#include <SceneObject.h>
#include <Renderable/Renderable.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <GamePad.h>

//Mouse
extern std::unique_ptr<DirectX::Mouse> mouse;
//Keyboard
extern std::unique_ptr<DirectX::Keyboard> keyboard;
extern DirectX::Keyboard::KeyboardStateTracker keys;
//GamePad
extern std::unique_ptr<DirectX::GamePad> gamePad;
extern DirectX::GamePad::ButtonStateTracker buttons;
//Timer
extern DX::StepTimer timer;

namespace Game
{
	static const float walkThreshold = 0.05f;
	static const float runThreshold = 0.4f;
	static const float minVectorLen = 0.02f;
	static const XMVECTOR baseForward = { 0.0f, 0.0f, -1.0f, 0.0f };
	static const float walkSpeed = 3.0f;
	static const float runSpeed = 10.0f;

	void VenomController::Map(std::shared_ptr<Scene::SceneObject> so)
	{
		Controller::Map(so);
		if (so->JType() == SO_Renderables)
		{
			venom = std::dynamic_pointer_cast<Scene::Renderable>(so);
		}
	}

	void VenomController::Unmap()
	{
		Controller::Unmap();
		if (venom != nullptr)
		{
			venom = nullptr;
		}
	}

	VenomController::VenomController()
	{
		vsm = {
			.currentState = VS_None,
			.onEnter = {
				{ VS_Idle, [this]() { venom->SetCurrentAnimation("Idle_C",0.0f,1.0f,true,true); }},
				{ VS_Walking, [this]() { venom->SetCurrentAnimation("Walk_Fwd_C",0.0f,1.0f,true,true); }},
				{ VS_Running, [this]() { venom->SetCurrentAnimation("Run_Fwd_C",0.0f,1.0f,true,true); }},
			},
			.onStep = {
				{ VS_None, [this]() { vsm.ChangeState(VS_Idle); }},
				{ VS_Idle, [this]() { Idle(); }},
				{ VS_Walking, [this]() { Walking(); }},
				{ VS_Running, [this]() { Running(); }},
			}
		};
	}

	XMVECTOR VenomController::GetLeftStickVector()
	{
		auto pad = gamePad->GetState(0);
		if (!pad.IsConnected()) return XMVectorZero();
		XMVECTOR dp = { pad.thumbSticks.leftX, 0.0f, pad.thumbSticks.leftY, 0.0f };
		return dp;
	}

	void VenomController::SetRotation(XMVECTOR fw) const
	{
		XMVECTOR len = XMVector3Length(fw);
		float l = len.m128_f32[0];

		if (l > minVectorLen)
		{
			XMVECTOR angleBetween = XMVector3AngleBetweenVectors(baseForward, fw);
			float yaw = XMConvertToDegrees(angleBetween.m128_f32[0]);

			if (fw.m128_f32[0] > 0.0f)
				yaw = -yaw;

			XMFLOAT3 rot = venom->rotation();
			rot.y = yaw;
			venom->rotation(rot);
		}
	}

	void VenomController::MoveForward(float step) const
	{
		float delta = timer.GetElapsedSeconds() * step;

		XMFLOAT3 p = venom->position();
		XMVECTOR pos = XMLoadFloat3(&p);
		XMVECTOR fw = XMVector3Rotate(baseForward, venom->rotationQ());
		XMVECTOR dp = XMVectorScale(fw, delta);
		pos = XMVectorAdd(pos, dp);
		XMStoreFloat3(&p, pos);
		venom->position(p);
	}

	void VenomController::Step(float delta)
	{
		vsm.Step();
		venom->StepAnimation(timer.GetElapsedSeconds());
	}

	void VenomController::Idle()
	{
		XMVECTOR stick = GetLeftStickVector();
		XMVECTOR len = XMVector3Length(stick);
		float l = len.m128_f32[0];

		if (l > runThreshold)
		{
			vsm.ChangeState(VS_Running);
		}
		else if (l > walkThreshold)
		{
			vsm.ChangeState(VS_Walking);
		}
	}

	void VenomController::Walking()
	{
		XMVECTOR stick = GetLeftStickVector();
		XMVECTOR len = XMVector3Length(stick);
		float l = len.m128_f32[0];

		SetRotation(stick);
		MoveForward(walkSpeed);

		if (l > runThreshold)
		{
			vsm.ChangeState(VS_Running);
		}
		else if (l < walkThreshold)
		{
			vsm.ChangeState(VS_Idle);
		}
	}

	void VenomController::Running()
	{
		XMVECTOR stick = GetLeftStickVector();
		XMVECTOR len = XMVector3Length(stick);
		float l = len.m128_f32[0];

		SetRotation(stick);
		MoveForward(runSpeed);

		if (l < walkThreshold)
		{
			vsm.ChangeState(VS_Idle);
		}
		else if (l < runThreshold)
		{
			vsm.ChangeState(VS_Walking);
		}
	}
}