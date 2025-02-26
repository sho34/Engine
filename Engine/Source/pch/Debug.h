#pragma once

inline void PrintRect(std::string name, RECT& r)
{
	std::string title = "rect-" + name + "\n";
	std::string left = "left:" + std::to_string(r.left) + "\n";
	std::string right = "right:" + std::to_string(r.right) + "\n";
	std::string top = "top:" + std::to_string(r.top) + "\n";
	std::string bottom = "bottom:" + std::to_string(r.bottom) + "\n";
	OutputDebugStringA(title.c_str());
	OutputDebugStringA(left.c_str());
	OutputDebugStringA(right.c_str());
	OutputDebugStringA(top.c_str());
	OutputDebugStringA(bottom.c_str());
}