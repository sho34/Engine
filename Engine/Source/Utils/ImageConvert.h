#pragma once

namespace Utils
{
	void GetImageAttributes(std::filesystem::path src, DXGI_FORMAT& format, unsigned int& width, unsigned int& height, unsigned int& mipLevels, unsigned int& numFrames);
	void ConvertToDDS(std::filesystem::path image, std::filesystem::path dds, DXGI_FORMAT desiredFormat, unsigned int width, unsigned int height, unsigned int mipLevels);
}