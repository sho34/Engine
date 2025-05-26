#pragma once
#include <filesystem>
#include <dxgiformat.h>

namespace Utils
{
	void GetImageAttributes(std::filesystem::path src, DXGI_FORMAT& format, unsigned int& width, unsigned int& height, unsigned int& mipLevels, unsigned int& numFrames);
	void ConvertToDDS(std::filesystem::path image, std::filesystem::path dds, DXGI_FORMAT desiredFormat, unsigned int width, unsigned int height, unsigned int mipLevels);
	void AssembleCubeDDS(std::filesystem::path image, std::vector<std::string> facesPath, unsigned int width, unsigned int height);
	void AssembleCubeDDSFromSkybox(std::filesystem::path image, std::filesystem::path skybox);
	void AssembleArrayDDSFromGif(std::filesystem::path image, std::filesystem::path gif);
}