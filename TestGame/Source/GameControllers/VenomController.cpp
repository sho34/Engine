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
				//{ VS_Attack_1,[this](VenomStates prevState) { EnterAttack1(); }}
			},
			.onStep = {
				{ VS_None, [this]() { venomScale = venom->scale(); vsm.ChangeState(VS_Intro); }},
				{ VS_Idle, [this]() { Idle(); }},
				{ VS_Walking, [this]() { Walking(); }},
				{ VS_Running, [this]() { Running(); }},
				//{ VS_Jumping, [this]() { Jumping(); }},
				//{ VS_Attack_1, [this]() { Attacking1(); }}
			}
		};
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
		UpdateLeftStickVector();
		UpdateLookTo();
		vsm.Step();
		/*
		venom->StepAnimation(dt);
		*/
		XMFLOAT3 vpos = venom->position();
		XMFLOAT3 cpos = camera->position();
		cpos.x = vpos.x;
		camera->position(cpos);
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

		XMVECTOR move = XMVector3Normalize(leftStick);
		move = XMVectorScale(move, delta);
		XMFLOAT3 p = venom->position();
		XMVECTOR pos = XMLoadFloat3(&p);
		pos = XMVectorAdd(pos, move);
		XMStoreFloat3(&p, pos);
		p.z = std::clamp(p.z, zBounds.x, zBounds.y);
		venom->position(p);
	}

	void VenomController::Roar()
	{
		if (roar.empty())
		{
			roar = getUUID();
			nlohmann::json json =
			{
				{ "uuid", roar() },
				{ "name", "roar" },
				{ "sound", GetSoundUUIDByName("venom/intro/roar") },
				{ "volume", 0.3 },
				{ "autoPlay", true }
			};
			CreateSceneObject(SO_SoundEffects, json);
		}
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
		venom->SetCurrentAnimation("Intro", 0.0f, 1.0f, true, false);
		/*
		venom->SetCurrentAnimation("Intro", 0.0f, 1.0f, true, false,
			{
				{ {.type = TimeCallbackType_End }, [this] {  vsm.ChangeState(VS_Idle); }, },
				{ {.type = TimeCallbackType_Frame, .frame = 127 }, [this] { Roar(); }, }
			}
		);
		*/
	}

	void VenomController::EnterIdle()
	{
		//venom->SetCurrentAnimation("Idle_C", 0.0f, 1.0f, true, true);
	}

	void VenomController::EnterWalking()
	{
		//venom->SetCurrentAnimation("Walk_Fwd_C", 0.0f, 1.5f, true, true);
	}

	void VenomController::EnterRunning()
	{
		//venom->SetCurrentAnimation("Run_Fwd_C", 0.0f, 1.5f, true, true);
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

	/*
	void VenomController::EnterAttack1()
	{
		auto& animControl = Attack1Animations.at(currentAttack1Animation);
		venom->SetCurrentAnimation(animControl.animation, 0.0f, animControl.speed, true, false);
		currentAttack1Animation = (currentAttack1Animation + 1) % Attack1Animations.size();

	}
	*/

	void VenomController::Idle()
	{
		if (ShouldRun())
		{
			vsm.ChangeState(VS_Running);
		}
		else if (ShouldWalk())
		{
			vsm.ChangeState(VS_Walking);
		}

		/*
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
			//Jump();
			//Attack1();
		}
		*/
	}

	void VenomController::Walking()
	{
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
	*/

	/*
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
	*/

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

	/*
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
	*/
}