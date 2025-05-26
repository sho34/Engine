#include "pch.h"
#include "ImageConvert.h"
#include "Utils.h"

namespace Utils
{
	void GetImageAttributes(std::filesystem::path src, DXGI_FORMAT& format, unsigned int& width, unsigned int& height, unsigned int& mipLevels, unsigned int& numFrames)
	{
		using namespace raymii;

		std::string cmdInfo = GetUtilsPath() + "texdiag.exe info " + src.string();
		CommandResult resultInfo = Command::exec(cmdInfo);
		//OutputDebugStringA((cmdInfo + "\n").c_str());
		std::string text = resultInfo.output;
		//OutputDebugStringA((text + "\n").c_str());
		std::regex pattern("width = ([\\d]+)[\\w\\d\\n\\W]+height = ([\\d]+)[\\w\\d\\n\\W]+mipLevels = ([\\d]+)[\\w\\d\\n\\W]+arraySize = ([\\d]+)[\\w\\d\\n\\W]+format = ([\\w]+)");
		std::smatch matches;

		if (std::regex_search(text, matches, pattern))
		{
			width = std::stoi(matches[1]);
			height = std::stoi(matches[2]);
			mipLevels = std::stoi(matches[3]);
			numFrames = std::stoi(matches[4]);
			format = stringToDxgiFormat.at(matches[5]);
		}
		else
		{
			assert(!!!"no info found from image file");
		}
	}

	void ConvertToDDS(std::filesystem::path image, std::filesystem::path dds, DXGI_FORMAT desiredFormat, unsigned int width, unsigned int height, unsigned int mipLevels)
	{
		using namespace raymii;

		std::filesystem::path parentPath = image.parent_path();
		std::string cmdConv = GetUtilsPath() + "texconv.exe " + image.string() + " -f " + dxgiFormatsToString.at(desiredFormat) + " -y";
		if (width != 0U) { cmdConv += " -w " + std::to_string(width); }
		if (height != 0U) { cmdConv += " -h " + std::to_string(height); }
		if (mipLevels != 0U) { cmdConv += " -m " + std::to_string(mipLevels); }

		CommandResult result = Command::exec(cmdConv);

		std::filesystem::path ddsUpperCase = dds;
		ddsUpperCase.replace_extension(".DDS");

		std::filesystem::rename(ddsUpperCase, dds);
	}

	void AssembleCubeDDS(std::filesystem::path image, std::vector<std::string> facesPath, unsigned int width, unsigned int height)
	{
		using namespace raymii;

		std::filesystem::path parentPath = image.parent_path();
		std::string cmdAssemble = GetUtilsPath() + "texassemble.exe cube -o " + image.string() + " -l -y -w " + std::to_string(width) + " -h " + std::to_string(height) + " ";
		for (auto& facePath : facesPath)
		{
			cmdAssemble += facePath + " ";
		}
		OutputDebugStringA((cmdAssemble + "\n").c_str());
		CommandResult result = Command::exec(cmdAssemble);
		OutputDebugStringA((result.output + "\n").c_str());

		std::filesystem::path ddsUpperCase = image;
		ddsUpperCase.replace_extension(".DDS");

		std::filesystem::rename(ddsUpperCase, image);
	}

	void AssembleCubeDDSFromSkybox(std::filesystem::path image, std::filesystem::path skybox)
	{
		using namespace raymii;

		std::filesystem::path parentPath = image.parent_path();
		std::string cmdAssemble = GetUtilsPath() + "texassemble.exe cube-from-hc -o " + image.string() + " -l -y " + skybox.string();

		OutputDebugStringA((cmdAssemble + "\n").c_str());
		CommandResult result = Command::exec(cmdAssemble);
		OutputDebugStringA((result.output + "\n").c_str());

		std::filesystem::path ddsUpperCase = image;
		ddsUpperCase.replace_extension(".DDS");

		std::filesystem::rename(ddsUpperCase, image);
	}

	void AssembleArrayDDSFromGif(std::filesystem::path image, std::filesystem::path gif)
	{
		using namespace raymii;

		std::filesystem::path parentPath = image.parent_path();
		std::string cmdAssemble = GetUtilsPath() + "texassemble.exe gif -o " + image.string() + " -l -y " + gif.string();

		OutputDebugStringA((cmdAssemble + "\n").c_str());
		CommandResult result = Command::exec(cmdAssemble);
		OutputDebugStringA((result.output + "\n").c_str());

		std::filesystem::path ddsUpperCase = image;
		ddsUpperCase.replace_extension(".DDS");

		std::filesystem::rename(ddsUpperCase, image);
	}
}