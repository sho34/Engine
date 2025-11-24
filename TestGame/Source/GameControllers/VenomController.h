#pragma once
#include <Controller.h>
#include <GameStateMachine.h>

namespace Scene
{
	struct Renderable;
	struct Camera;
};

struct AnimationController
{
	std::string animation;
	float speed;
};

namespace Game
{
	struct VenomController : Controller
	{
		enum VenomStates
		{
			VS_None,
			VS_Intro,
			VS_Idle,
			VS_Walking,
			VS_Running,
			VS_Jumping,
			VS_Attack_1
		};

		enum LookingTo
		{
			LT_Right,
			LT_Left,
		};

		GameStatesMachine<VenomStates> vsm;
		LookingTo lookingTo = LookingTo::LT_Right;

		RenderableUUID venom;
		CameraUUID camera;

		XMFLOAT3 venomScale;

		XMVECTOR lastAnimPos;
		XMVECTOR lastAnimPosDelta;
		XMVECTOR lastAnimPosDelta2;

		static inline std::vector<AnimationController> Attack1Animations =
		{
			{
				.animation = "Punch1",
				.speed = 1.0f
			},
			{
				.animation = "Punch1",
				.speed = 1.0f
			},
{
				.animation = "Punch1",
				.speed = 1.0f
			},
{
				.animation = "Punch1",
				.speed = 1.0f
			},
			{
				.animation = "Punch2",
				.speed = 1.0f
			}
		};
		bool attack1Window = false;
		bool newAttack1 = false;
		int currentAttack1Animation = 0;

		virtual void Map(JUUID so);
		virtual void Unmap();
		VenomController();
		virtual void CreateV8MethodsBindings(Scripting::V8MethodsBindings& v8methods);
		virtual void Step(float delta);

		void VenomReady();
		void StartVenomNextPunchWindow();
		void EvaluateVenomNextPunch();

		XMVECTOR leftStick;
		void UpdateLeftStickVector();
		void UpdateLookTo();

		void MoveForward(float step);

		bool ShouldIdle();
		bool ShouldWalk();
		bool ShouldRun();
		bool ShouldAttackX();

		//void SetRotation(XMVECTOR fw);
		//
		//bool Jump();
		//void Attack1();
		void EnterAttack1();

		//void None();
		void EnterIntro();
		void EnterIdle();
		void EnterWalking();
		void EnterRunning();
		void Idle();
		void Walking();
		void Running();
		//void Walking();
		//void Running();
		//void Jumping();
		void Attacking1();

		void LeaveAttack1();
	};

	void VenomReady(const v8::FunctionCallbackInfo<v8::Value>& info);
	void StartVenomNextPunchWindow(const v8::FunctionCallbackInfo<v8::Value>& info);
	void EvaluateVenomNextPunch(const v8::FunctionCallbackInfo<v8::Value>& info);
}
