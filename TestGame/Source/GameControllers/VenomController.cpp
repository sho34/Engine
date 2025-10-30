#include "pch.h"
#include "VenomController.h"
#include <Scene.h>
#include <SceneObject.h>
#include <Renderable/Renderable.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <GamePad.h>
#include <StepTimer.h>
#include <Camera/Camera.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

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
	static XMFLOAT2 zBounds = { -4.1f ,1.7f };

	void VenomController::Map(JUUID so)
	{
		using namespace Scene;
		Controller::Map(so);
		SceneObjectType type = GetSceneObjectType(so);
		if (type == SO_Renderables)
		{
			venom = so;
		}
		if (GetCountFromMouseCameras() > 0ULL)
		{
			camera = *GetMouseCameras().begin();
		}
	}

	void VenomController::Unmap()
	{
		Controller::Unmap();
		venom.clear();
		camera.clear();
	}

	VenomController::VenomController()
	{
		vsm = {
			.currentState = VS_None,
			.onEnter = {
				{ VS_Idle, [this](VenomStates prevState) { venom->SetCurrentAnimation("Idle_C",0.0f,1.0f,true,true); }},
				{ VS_Walking, [this](VenomStates prevState) { venom->SetCurrentAnimation("Walk_Fwd_C",0.0f,1.0f,true,true); }},
				{ VS_Running, [this](VenomStates prevState) { venom->SetCurrentAnimation("Run_Fwd_C",0.0f,1.0f,true,true); }},
				{ VS_Jumping, [this](VenomStates prevState) { venom->SetCurrentAnimation("Jump",0.0f,1.0f,true,false); }},
				{ VS_Attack_1,[this](VenomStates prevState) { EnterAttack1(); }}
				//{ VS_Jumping, [this]() { venom->SetCurrentAnimation(std::vector<std::string>({"Jump_Start_F_C","Jump_Falling_F_C","Jump_Land_F_C"}),0.0f,0.05f,true,false); }}
			},
			.onStep = {
				{ VS_None, [this]() { vsm.ChangeState(VS_Idle); }},
				{ VS_Idle, [this]() { Idle(); }},
				{ VS_Walking, [this]() { Walking(); }},
				{ VS_Running, [this]() { Running(); }},
				{ VS_Jumping, [this]() { Jumping(); }},
				{ VS_Attack_1, [this]() { Attacking1(); }}
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

	void VenomController::SetRotation(XMVECTOR fw)
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

	void VenomController::MoveForward(float step)
	{
		float delta = static_cast<float>(timer.GetElapsedSeconds() * step);

		XMFLOAT3 p = venom->position();
		XMVECTOR pos = XMLoadFloat3(&p);
		XMVECTOR fw = XMVector3Rotate(baseForward, venom->rotationQ());
		XMVECTOR dp = XMVectorScale(fw, delta);
		pos = XMVectorAdd(pos, dp);
		XMStoreFloat3(&p, pos);
		p.z = std::clamp(p.z, zBounds.x, zBounds.y);
		venom->position(p);
	}

	bool VenomController::Jump()
	{
		if (buttons.a == GamePad::ButtonStateTracker::PRESSED)
		{
			vsm.ChangeState(VS_Jumping);
			venom->StepAnimation(0.0f);
			return true;
		}
		return false;
	}

	void VenomController::Attack1()
	{
		if (buttons.x == GamePad::ButtonStateTracker::PRESSED)
		{
			vsm.ChangeState(VS_Attack_1);
			venom->StepAnimation(0.0f);
		}
	}

	void VenomController::EnterAttack1()
	{
		auto& animControl = Attack1Animations.at(currentAttack1Animation);
		venom->SetCurrentAnimation(animControl.animation, 0.0f, animControl.speed, true, false);
		currentAttack1Animation = (currentAttack1Animation + 1) % Attack1Animations.size();

	}

	void VenomController::Step(float delta)
	{
		float dt = static_cast<float>(timer.GetElapsedSeconds());

		auto state = gamePad->GetState(0);
		if (state.IsConnected())
		{
			buttons.Update(state);
		}
		else
		{
			buttons.Reset();
		}
#if defined(_EDITOR)
		if (!Editor::IsPlaying() || Editor::IsPaused())
			return;
#endif
		vsm.Step();
		venom->StepAnimation(dt);
		XMFLOAT3 vpos = venom->position();
		XMFLOAT3 cpos = camera->position();
		cpos.x = vpos.x;
		camera->position(cpos);
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
		else
		{
			Jump();
			Attack1();
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
		else
		{
			Jump();
			Attack1();
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
		else
		{
			Jump();
			Attack1();
		}
	}

	void VenomController::Jumping()
	{
		XMVECTOR stick = GetLeftStickVector();
		XMVECTOR len = XMVector3Length(stick);
		float l = len.m128_f32[0];

		SetRotation(stick);

		bool animEnded = venom->AnimationEnded();

		if (l > runThreshold)
		{
			MoveForward(runSpeed);
			if (animEnded)
			{
				vsm.ChangeState(VS_Running);
				venom->StepAnimation(0.0f);
			}
		}
		else if (l > walkThreshold)
		{
			MoveForward(walkSpeed);
			if (animEnded)
			{
				vsm.ChangeState(VS_Walking);
				venom->StepAnimation(0.0f);
			}
		}
		else if (animEnded)
		{
			if (!Jump())
			{
				vsm.ChangeState(VS_Idle);
				venom->StepAnimation(0.0f);
			}
		}
	}

	void VenomController::Attacking1()
	{
		bool animEnded = venom->AnimationEnded();
		if (!animEnded) return;

		XMVECTOR stick = GetLeftStickVector();
		XMVECTOR len = XMVector3Length(stick);
		float l = len.m128_f32[0];

		SetRotation(stick);

		if (l > runThreshold)
		{
			MoveForward(runSpeed);
			if (animEnded)
			{
				vsm.ChangeState(VS_Running);
				venom->StepAnimation(0.0f);
			}
		}
		else if (l > walkThreshold)
		{
			MoveForward(walkSpeed);
			if (animEnded)
			{
				vsm.ChangeState(VS_Walking);
				venom->StepAnimation(0.0f);
			}
		}
		else if (animEnded)
		{
			if (!Jump())
			{
				vsm.ChangeState(VS_Idle);
				venom->StepAnimation(0.0f);
			}
		}
	}
}