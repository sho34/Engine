#pragma once

namespace Templates {

#if defined(_EDITOR)
	void SaveTemplates(const std::string folder, const std::string fileName, nlohmann::json data);
#endif

	typedef Concurrency::task<void> (*TemplateLoader)(std::string, nlohmann::json);
	void LoadTemplates(const std::string folder, const std::string fileName, TemplateLoader loader);
}

