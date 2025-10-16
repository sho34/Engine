#pragma once

struct DeletePrompt
{
	bool showing = false;
	std::vector<nlohmann::json> references;
	std::function<void(std::vector<nlohmann::json> references)> OnDelete;
	std::function<void()> OnCancel;

	void DrawPrompt(const char* prompTitle);
};
