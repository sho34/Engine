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

		std::string text = resultInfo.output;
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

	void ConvertToDDS(std::filesystem::path image, std::filesystem::path dds, DXGI_FORMAT desiredFormat)
	{
		using namespace raymii;

		//get the format of the file
		std::filesystem::path parentPath = image.parent_path();
		std::string cmdConv = GetUtilsPath() + "texconv.exe " + image.string() + " -f " + dxgiFormatsToString.at(desiredFormat) + " -y";
		CommandResult result = Command::exec(cmdConv);

		std::filesystem::path ddsUpperCase = dds;
		ddsUpperCase.replace_extension(".DDS");

		std::filesystem::rename(ddsUpperCase, dds);
	}
}