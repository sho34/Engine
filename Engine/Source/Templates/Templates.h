#pragma once

namespace Templates {

#if defined(_EDITOR)
	void SaveTemplates(const std::wstring folder, const std::wstring fileName, nlohmann::json data);
#endif

	typedef Concurrency::task<void> (*TemplateLoader)(std::wstring, nlohmann::json);
	void LoadTemplates(const std::wstring folder, const std::wstring fileName, TemplateLoader loader);
}

