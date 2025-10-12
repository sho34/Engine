#pragma once
#include <Controller.h>
#include <GameStateMachine.h>

namespace Scene
{
	struct Renderable;
	struct Camera;
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
			VS_Jumping
		};

		GameStatesMachine<VenomStates> vsm;

		std::shared_ptr<Scene::Renderable> venom;
		std::shared_ptr<Scene::Camera> camera;

		virtual void Map(std::shared_ptr<Scene::SceneObject> so);
		virtual void Unmap();
		VenomController();

		XMVECTOR GetLeftStickVector();
		void SetRotation(XMVECTOR fw) const;
		void MoveForward(float step) const;
		bool Jump();
		virtual void Step(float delta);

		void Idle();
		void Walking();
		void Running();
		void Jumping();
	};
}
