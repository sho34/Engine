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
			VS_Idle,
			VS_Walking,
			VS_Running,
			VS_Jumping,
			VS_Attack_1
		};

		GameStatesMachine<VenomStates> vsm;

		RenderableUUID venom;
		CameraUUID camera;
		unsigned int currentAttack1Animation = 0;

		static inline std::vector<AnimationController> Attack1Animations =
		{
			{
				.animation = "103511_Attack01",
				.speed = 2.0f
			},
			{
				.animation = "103511_Attack01",
				.speed = 2.0f
			},
			{
				.animation = "103511_Attack01",
				.speed = 2.0f
			},
			{
				.animation = "103511_Attack01",
				.speed = 2.0f
			},
			{
				.animation = "103511_Attack02",
				.speed = 2.0f
			}
		};

		virtual void Map(JUUID so);
		virtual void Unmap();
		VenomController();

		XMVECTOR GetLeftStickVector();
		void SetRotation(XMVECTOR fw);
		void MoveForward(float step);
		bool Jump();
		void Attack1();
		void EnterAttack1();
		virtual void Step(float delta);

		void None();
		void Idle();
		void Walking();
		void Running();
		void Jumping();
		void Attacking1();
	};
}
