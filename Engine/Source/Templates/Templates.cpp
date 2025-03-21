#include "pch.h"
#include "Templates.h"

namespace Templates
{
#if defined(_EDITOR)

	void SaveTemplates(const std::string folder, const std::string fileName, nlohmann::json data)
	{
		//first create the directory if needed
		std::filesystem::path directory(folder);
		std::filesystem::create_directory(directory);

		//then create the json level file
		const std::string finalFilename = folder + fileName;
		std::filesystem::path path(finalFilename);
		std::string pathStr = path.generic_string();
		std::ofstream file;
		file.open(pathStr);
		std::string dataString = data.dump(4);
		file.write(dataString.c_str(), dataString.size());
		file.close();
	}

#endif

	void LoadTemplates(const std::string folder, const std::string fileName, std::function<void(std::string, nlohmann::json)> loader)
	{
		//first create the directory if needed
		std::filesystem::path directory(folder);
		const std::string finalFilename = folder + fileName;
		std::filesystem::path path(finalFilename);
		std::string pathStr = path.generic_string();
		std::ifstream file(pathStr);
		nlohmann::json data = nlohmann::json::parse(file);
		file.close();

		for (auto& [key, value] : data.items())
		{
			loader(key, value);
		}
	}

	void DestroyTemplates()
	{
		ReleaseSoundTemplates();
		ReleaseModel3DTemplates();
		ReleaseMeshTemplates();
		ReleaseMaterialTemplates();
		ReleaseShaderTemplates();
	}

#if defined(_EDITOR)
	void DestroyTemplatesReferences()
	{
		Sound::ReleaseSoundEffectsInstances();
		Model3D::ReleaseRenderablesInstances();
	}
#endif
}