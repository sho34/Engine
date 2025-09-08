#include "pch.h"
#include "MousePicking.h"

void MousePicking::Reset()
{
	pickingX = 0U;
	pickingY = 0U;
	doPicking = false;
}

void MousePicking::StartPicking(DirectX::Mouse::State state)
{
	pickingX = state.x;
	pickingY = state.y;
	doPicking = false;
}

bool MousePicking::CanPick(DirectX::Mouse::State state) const
{
	/*
	std::string str = "button:" + std::string((!state.leftButton) ? "true" : "false") + " " +
	"state.x:" + std::to_string(state.x) + " " + "pickingX:" + std::to_string(pickingX) + " " +
	"state.y:" + std::to_string(state.y) + " " + "pickingY:" + std::to_string(pickingY) + " " +
	"\n";
	OutputDebugStringA(str.c_str());
	*/
	return (!state.leftButton && state.x == pickingX && state.y == pickingY);
}

void MousePicking::Pick()
{
	doPicking = true;
}

bool MousePicking::MouseMoved(DirectX::Mouse::State state) const
{
	return (state.leftButton && (state.x != pickingX || state.y != pickingY));
}
