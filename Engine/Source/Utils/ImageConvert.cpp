#include "pch.h"
#include "ImageConvert.h"
#include "Utils.h"
#include <DirectXTex.h>
#include <texdiag.h>

namespace Utils
{
	void GetImageAttributes(std::filesystem::path src, DXGI_FORMAT& format, unsigned int& width, unsigned int& height, unsigned int& mipLevels, unsigned int& numFrames)
	{
		using namespace raymii;

		std::string cmdInfo = GetUtilsPath() + "texdiag.exe info \"" + src.string() + "\"";
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
			format = StringToDXGI_FORMAT.at(matches[5]);
		}
		else
		{
			assert(!!!"no info found from image file");
		}
	}

	void GetImageAttributes(std::filesystem::path src, DirectX::TexMetadata& info)
	{
		std::vector<std::wstring> args = { L"", L"info", src.wstring() };
		std::vector<wchar_t*> texDiagArgs;
		std::transform(args.begin(), args.end(), std::back_inserter(texDiagArgs), [](auto& arg) { return arg.data(); });
		texDiag(static_cast<int>(texDiagArgs.size()), texDiagArgs.data(), &info);
	}

	void ConvertToDDS(std::filesystem::path image, std::filesystem::path dds, DXGI_FORMAT desiredFormat, unsigned int width, unsigned int height, unsigned int mipLevels)
	{
		using namespace raymii;

		std::filesystem::path parentPath = image.parent_path();
		std::string cmdConv = GetUtilsPath() + "texconv.exe \"" + image.string() + "\" -f " + DXGI_FORMATToString.at(desiredFormat) + " -y";
		if (width != 0U) { cmdConv += " -w " + std::to_string(width); }
		if (height != 0U) { cmdConv += " -h " + std::to_string(height); }
		if (mipLevels != 0U) { cmdConv += " -m " + std::to_string(mipLevels); }

		CommandResult result = Command::exec(cmdConv);

		std::filesystem::path ddsUpperCase = dds;
		ddsUpperCase.replace_extension(".DDS");

		std::filesystem::rename(ddsUpperCase, dds);
	}

	void ConvertToDDS(ImageConverter& conversion)
	{
		std::vector<std::wstring> args = { L"" };

		args.push_back(conversion.src.wstring());
		if (conversion.format != DXGI_FORMAT_UNKNOWN)
		{
			args.push_back(L"-f");
			args.push_back(nostd::StringToWString(DXGI_FORMATToString.at(conversion.format)));
		}
		args.push_back(L"-y");

		if (conversion.width != 0U)
		{
			args.push_back(L"-w");
			args.push_back(std::to_wstring(conversion.width));
		}
		if (conversion.height != 0U)
		{
			args.push_back(L"-h");
			args.push_back(std::to_wstring(conversion.height));
		}
		if (conversion.mipLevels != 0U)
		{
			args.push_back(L"-m");
			args.push_back(std::to_wstring(conversion.mipLevels));
		}
		args.push_back(L"-l");
		args.push_back(L"-o");
		args.push_back(conversion.dst.parent_path().relative_path().wstring());

		std::vector<wchar_t*> texConvArgs;
		std::transform(args.begin(), args.end(), std::back_inserter(texConvArgs), [](auto& arg) { return arg.data(); });
		texConv(static_cast<int>(texConvArgs.size()), texConvArgs.data());
		DirectX::TexMetadata info;
		GetImageAttributes(conversion.dst, info);
		conversion.width = static_cast<unsigned int>(info.width);
		conversion.height = static_cast<unsigned int>(info.height);
		conversion.format = info.format;
		conversion.mipLevels = static_cast<unsigned int>(info.mipLevels);
		conversion.numFrames = static_cast<unsigned int>(info.arraySize);
		conversion.type = info.IsCubemap() ? (TextureType_Cube) : (info.arraySize > 1ULL) ? TextureType_Array : TextureType_2D;
	}

	void AssembleCubeDDS(std::filesystem::path image, std::vector<std::string> facesPath, unsigned int width, unsigned int height)
	{
		using namespace raymii;

		std::filesystem::path parentPath = image.parent_path();
		std::string cmdAssemble = GetUtilsPath() + "texassemble.exe cube -o \"" + image.string() + "\" -l -y -w " + std::to_string(width) + " -h " + std::to_string(height) + " ";
		for (auto& facePath : facesPath)
		{
			cmdAssemble += std::string("\"") + facePath + "\" ";
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
		std::string cmdAssemble = GetUtilsPath() + "texassemble.exe cube-from-hc -o \"" + image.string() + "\" -l -y \"" + skybox.string() + "\"";

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
		std::string cmdAssemble = GetUtilsPath() + "texassemble.exe gif -o \"" + image.string() + "\" -l -y " + gif.string();

		OutputDebugStringA((cmdAssemble + "\n").c_str());
		CommandResult result = Command::exec(cmdAssemble);
		OutputDebugStringA((result.output + "\n").c_str());

		std::filesystem::path ddsUpperCase = image;
		ddsUpperCase.replace_extension(".DDS");

		std::filesystem::rename(ddsUpperCase, image);
	}
}