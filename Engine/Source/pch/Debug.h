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

inline void PrintXMVector(XMVECTOR v, std::string name = "")
{
	OutputDebugStringA(std::string(name + std::string((name != "") ? ":" : "") + std::to_string(v.m128_f32[0]) + "," + std::to_string(v.m128_f32[1]) + "," + std::to_string(v.m128_f32[2]) + "," + std::to_string(v.m128_f32[3]) + "\n").c_str());
}

inline void PrintXMFloat3(XMFLOAT3 v, std::string name = "")
{
	OutputDebugStringA(std::string(name + std::string((name != "") ? ":" : "") + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "\n").c_str());
}