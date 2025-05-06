#pragma once

enum TemplateFlags {
	None,
	Loading = 0x1,
	SystemCreated = 0x2
};

template <typename T>
using TemplatesContainer = std::map<std::string, T>;

