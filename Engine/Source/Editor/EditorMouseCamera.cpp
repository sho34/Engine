#include "pch.h"
#include "EditorMouseCamera.h"

void EditorMouseCamera::Reset()
{
	leftButton = false;
	rightButton = false;
	wheelCaptured = false;
	wheelMode = false;
	wheel = 0;
	mouseX = 0;
	mouseY = 0;
}

void EditorMouseCamera::RickClick(int x, int y)
{
	rightButton = true;
	mouseX = x;
	mouseY = y;
}

bool EditorMouseCamera::RightButton()
{
	return rightButton;
}

void EditorMouseCamera::LeftClick(int x, int y)
{
	leftButton = true;
	mouseX = x;
	mouseY = y;
}

bool EditorMouseCamera::LeftButton()
{
	return leftButton;
}

bool EditorMouseCamera::WheelCaptured() const { return wheelCaptured; }

bool EditorMouseCamera::WheelMode() const
{
	return wheelMode;
}

int EditorMouseCamera::Wheel() const { return wheel; }

void EditorMouseCamera::Wheel(int w)
{
	wheel = w;
}

void EditorMouseCamera::CaptureWheel(int scrollWheelValue)
{
	wheel = scrollWheelValue;
	wheelCaptured = true;
}

void EditorMouseCamera::UpdateWheelMode(int scrollWheelValue, int x, int y)
{
	mouseX = x;
	mouseY = y;
	wheel = scrollWheelValue;
	wheelMode = true;
}

void EditorMouseCamera::UpdateMouseXY(int x, int y, int& dx, int& dy)
{
	dx = x - mouseX;
	dy = y - mouseY;
	mouseX = x;
	mouseY = y;
}


