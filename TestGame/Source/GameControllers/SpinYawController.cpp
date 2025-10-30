#include "pch.h"
#include "SpinYawController.h"
#include <GamePad.h>
#include <Scene.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

extern std::unique_ptr<DirectX::GamePad> gamePad;
extern DirectX::GamePad::ButtonStateTracker buttons;

namespace Game
{
	void SpinYawController::Step(float delta)
	{
		using namespace Scene;

		auto o = GetSceneObjectPointer(sceneObject);

		if (!o->contains("rotation"))
			return;

		auto state = gamePad->GetState(0);
		if (!state.IsConnected())
		{
			buttons.Reset();
			return;
		}

		buttons.Update(state);
#if defined(_EDITOR)
		if (!Editor::IsPlaying() || Editor::IsPaused())
			return;
#endif
		auto pad = gamePad->GetState(0);
		float dy = pad.thumbSticks.leftX * 10;

		XMFLOAT3 rot = ToXMFLOAT3(o->at("rotation"));
		rot.y += dy;
		o->at("rotation") = FromXMFLOAT3(rot);
	}
}

