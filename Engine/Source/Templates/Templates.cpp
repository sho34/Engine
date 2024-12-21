#include "pch.h"
#include "Templates.h"
#include "../Types.h"

namespace Templates {

#if defined(_EDITOR)
	void SaveTemplates(const std::wstring folder, const std::wstring fileName, nlohmann::json data) {
		//first create the directory if needed
		std::filesystem::path directory(folder);
		std::filesystem::create_directory(directory);

		//then create the json level file
		const std::wstring finalFilename = folder + fileName;
		std::filesystem::path path(finalFilename);
		std::string pathStr = path.generic_string();
		std::ofstream file;
		file.open(pathStr);
		std::string dataString = data.dump(4);
		file.write(dataString.c_str(), dataString.size());
		file.close();
	}

#endif

	void LoadTemplates(const std::wstring folder, const std::wstring fileName, TemplateLoader loader)
	{
		//first create the directory if needed
		std::filesystem::path directory(folder);
		const std::wstring finalFilename = folder + fileName;
		std::filesystem::path path(finalFilename);
		std::string pathStr = path.generic_string();
		std::ifstream file(pathStr);
		nlohmann::json data = nlohmann::json::parse(file);
		file.close();

		std::vector<concurrency::task<void>> tasks;
		for (auto& [key, value] : data.items()) {
			tasks.push_back(loader(StringToWString(key), value));
		}

		when_all(tasks.begin(), tasks.end()).wait();
	}

}