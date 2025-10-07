#pragma once

struct EditorMouseCamera
{
	bool leftButton = false;
	bool rightButton = false;
	bool wheelCaptured = false;
	bool wheelMode = false;
	int wheel = 0;
	int mouseX = 0;
	int mouseY = 0;

	void Reset();
	void RickClick(int x, int y);
	bool RightButton();
	void LeftClick(int x, int y);
	bool LeftButton();
	bool WheelCaptured() const;
	bool WheelMode() const;
	int Wheel() const;
	void Wheel(int);
	void CaptureWheel(int scrollWheelValue);
	void UpdateWheelMode(int scrollWheelValue, int x, int y);
	void UpdateMouseXY(int x, int y, int& dx, int& dy);
};