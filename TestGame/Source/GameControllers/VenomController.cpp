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
	static const float lookToThreshold = 0.03f;
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

	//103541_Shackle
	//103531_Descent_Loop
	//103531_Descent_End
	//Intro: 103531_Descent_Loop -> 103531_Descent_End -> 103541_Shackle

	//103501_LowCrawl_Idle_R
	//103501_LowCrawl_Move
	//Wall:

	//A4_10_R
	//A4_11_R
	//A4_12_R
	//A4_13_R
	//Jump Kick


	VenomController::VenomController()
	{
		vsm = {
			.currentState = VS_None,
			.onEnter = {
				{ VS_Intro, [this](VenomStates prevState) { EnterIntro(); }},
				{ VS_Idle, [this](VenomStates prevState) { EnterIdle(); }},
				{ VS_Walking, [this](VenomStates prevState) { EnterWalking(); } },
				{ VS_Running, [this](VenomStates prevState) { EnterRunning(); } },
				//{ VS_Jumping, [this](VenomStates prevState) { venom->SetCurrentAnimation("Jump",0.0f,1.0f,true,false); }},
				{ VS_Attack_1,[this](VenomStates prevState) { EnterAttack1(); }}
			},
			.onLeave = {
				{ VS_Attack_1,[this](VenomStates prevState) { LeaveAttack1(); }}
			},
			.onStep = {
				{ VS_None, [this]() { venomScale = venom->scale(); vsm.ChangeState(VS_Intro); }},
				{ VS_Idle, [this]() { Idle(); }},
				{ VS_Walking, [this]() { Walking(); }},
				{ VS_Running, [this]() { Running(); }},
				//{ VS_Jumping, [this]() { Jumping(); }},
				{ VS_Attack_1, [this]() { Attacking1(); }}
			}
		};
		lastAnimPos = XMVectorZero();
		lastAnimPosDelta = XMVectorZero();
		lastAnimPosDelta2 = XMVectorZero();
		attack1Window = false;
		newAttack1 = false;
	}

	void VenomController::CreateV8MethodsBindings(Scripting::V8MethodsBindings& v8methods)
	{
		v8methods.insert_or_assign("PlayerReady", Game::VenomReady);
		v8methods.insert_or_assign("StartNextPunchWindow", Game::StartVenomNextPunchWindow);
		v8methods.insert_or_assign("EvaluateNextPunch", Game::EvaluateVenomNextPunch);
	}

	void VenomController::Step(float delta)
	{
#if defined(_EDITOR)
		if (!Editor::IsPlaying() || Editor::IsPaused())
			return;
#endif

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

		XMVECTOR XMpos, XMrot, XMscl;
		XMMatrixDecompose(&XMscl, &XMrot, &XMpos, venom->animationTransformation);

		lastAnimPosDelta2 = lastAnimPosDelta;
		lastAnimPosDelta = XMVectorSubtract(XMpos, lastAnimPos);
		lastAnimPos = XMpos;

		UpdateLeftStickVector();
		UpdateLookTo();
		vsm.Step();
		XMFLOAT3 vpos = venom->position();
		XMFLOAT3 cpos = camera->position();
		cpos.x = vpos.x;
		camera->position(cpos);
	}

	void VenomReady(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::Isolate* isolate = args.GetIsolate();
		v8::HandleScope handle_scope(isolate);

		// Retrieve the C++ MyClass instance from the FunctionTemplate's data
		v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(args.Data());
		VenomController* venom = static_cast<VenomController*>(wrap->Value());
		venom->VenomReady();
	}

	void StartVenomNextPunchWindow(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::Isolate* isolate = args.GetIsolate();
		v8::HandleScope handle_scope(isolate);

		// Retrieve the C++ MyClass instance from the FunctionTemplate's data
		v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(args.Data());
		VenomController* venom = static_cast<VenomController*>(wrap->Value());
		venom->StartVenomNextPunchWindow();
	}

	void EvaluateVenomNextPunch(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::Isolate* isolate = args.GetIsolate();
		v8::HandleScope handle_scope(isolate);

		// Retrieve the C++ MyClass instance from the FunctionTemplate's data
		v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(args.Data());
		VenomController* venom = static_cast<VenomController*>(wrap->Value());
		venom->EvaluateVenomNextPunch();
	}

	void VenomController::VenomReady()
	{
		vsm.ChangeState(VS_Idle);
	}

	void VenomController::StartVenomNextPunchWindow()
	{
		attack1Window = true;
	}

	void VenomController::EvaluateVenomNextPunch()
	{
		std::vector<std::pair<std::function<bool()>, std::function<void()>>> postAttackActions = {
			{
				[&]() { return ShouldRun(); },
				[&]() { vsm.ChangeState(VS_Running); }
			},
			{
				[&]() { return ShouldWalk(); },
				[&]() { vsm.ChangeState(VS_Walking); }
			},
			{
				[&]() { return ShouldIdle(); },
				[&]() { vsm.ChangeState(VS_Idle); }
			}
		};

		if (newAttack1)
		{
			EnterAttack1();
		}
		else
		{
			for (auto& [cond, action] : postAttackActions)
			{
				if (cond())
				{
					action();
					break;
				}
			}
		}
		newAttack1 = false;
		attack1Window = false;
	}

	void VenomController::UpdateLeftStickVector()
	{
		auto pad = gamePad->GetState(0);
		if (pad.IsConnected())
			leftStick = { pad.thumbSticks.leftX, 0.0f, pad.thumbSticks.leftY, 0.0f };
		else
			leftStick = XMVectorZero();
	}

	void VenomController::UpdateLookTo()
	{
		if (vsm.currentState == VS_Intro) return;

		float len = leftStick.m128_f32[0];

		if (fabsf(len) < lookToThreshold) return;

		if (len < 0.0f)
		{
			XMFLOAT3 scale = venom->scale();
			if (scale.z > 0.0f)
			{
				scale.z = -venomScale.z;
				venom->scale(scale);
			}
		}
		else if (len > 0.0f)
		{
			XMFLOAT3 scale = venom->scale();
			if (scale.z < 0.0f)
			{
				scale.z = venomScale.z;
				venom->scale(scale);
			}
		}
	}

	void VenomController::MoveForward(float step)
	{
		float delta = static_cast<float>(timer.GetElapsedSeconds() * step);

		XMFLOAT3 scale = venom->scale();
		XMVECTOR move = XMVector3Normalize(leftStick);
		move = XMVectorScale(move, delta);
		float dz = -lastAnimPosDelta.m128_f32[2];
		if (dz > 0.0f)
		{
			move.m128_f32[0] = scale.z * std::max(-lastAnimPosDelta.m128_f32[2], 0.0f);
		}
		else
		{
			move.m128_f32[0] = scale.z * std::max(-lastAnimPosDelta2.m128_f32[2], 0.0f);
		}
		XMFLOAT3 p = venom->position();
		XMVECTOR pos = XMLoadFloat3(&p);
		pos = XMVectorAdd(pos, move);
		XMStoreFloat3(&p, pos);
		p.z = std::clamp(p.z, zBounds.x, zBounds.y);
		venom->position(p);
	}

	bool VenomController::ShouldIdle()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l < walkThreshold;
	}

	bool VenomController::ShouldWalk()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l > walkThreshold && l < runThreshold;
	}

	bool VenomController::ShouldRun()
	{
		XMVECTOR len = XMVector3Length(leftStick);
		float l = len.m128_f32[0];
		return l > runThreshold;
	}

	bool VenomController::ShouldAttackX()
	{
		return (buttons.x == GamePad::ButtonStateTracker::PRESSED);
	}

	void VenomController::EnterIntro()
	{
		venom->animationUseTransformation(true);
		venom->SetCurrentAnimation("Intro", 0.0f, 1.0f, true, false);
	}

	void VenomController::EnterIdle()
	{
		venom->animationUseTransformation(false);
		venom->SetCurrentAnimation("Idle_C", 0.0f, 1.0f, true, true);
	}

	void VenomController::EnterWalking()
	{
		venom->SetCurrentAnimation("Walk", 0.0f, 1.0f, true, true);
	}

	void VenomController::EnterRunning()
	{
		venom->SetCurrentAnimation("Run", 0.0f, 1.0f, true, true);
	}

	/*
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
	*/



	/*
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
	*/

	/*
	void VenomController::Attack1()
	{
		if (buttons.x == GamePad::ButtonStateTracker::PRESSED)
		{
			vsm.ChangeState(VS_Attack_1);
			venom->StepAnimation(0.0f);
		}
	}
	*/

	void VenomController::EnterAttack1()
	{
		auto& animControl = Attack1Animations.at(currentAttack1Animation);
		venom->SetCurrentAnimation(animControl.animation);
		currentAttack1Animation = (currentAttack1Animation + 1) % Attack1Animations.size();
	}

	void VenomController::Idle()
	{
		if (ShouldAttackX())
		{
			vsm.ChangeState(VS_Attack_1);
		}
		else if (ShouldRun())
		{
			vsm.ChangeState(VS_Running);
		}
		else if (ShouldWalk())
		{
			vsm.ChangeState(VS_Walking);
		}
	}

	void VenomController::Walking()
	{
		if (ShouldAttackX())
		{
			vsm.ChangeState(VS_Attack_1);
			return;
		}

		MoveForward(walkSpeed);
		if (ShouldIdle())
		{
			vsm.ChangeState(VS_Idle);
		}
		else if (ShouldRun())
		{
			vsm.ChangeState(VS_Running);
		}
	}

	void VenomController::Running()
	{
		if (ShouldAttackX())
		{
			vsm.ChangeState(VS_Attack_1);
			return;
		}

		MoveForward(runSpeed);
		if (ShouldIdle())
		{
			vsm.ChangeState(VS_Idle);
		}
		else if (ShouldWalk())
		{
			vsm.ChangeState(VS_Walking);
		}
	}

	/*
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
	*/

	void VenomController::Attacking1()
	{
		if (!newAttack1 && attack1Window && ShouldAttackX())
		{
			newAttack1 = true;
		}
	}
	void VenomController::LeaveAttack1()
	{
		attack1Window = false;
		newAttack1 = false;
		currentAttack1Animation = 0;
	}
}