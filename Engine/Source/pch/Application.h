#pragma once

static const std::string defaultLevelsFolder = "Levels/";
static const std::string defaultTemplatesFolder = "Templates/";
static const std::string defaultShadersFolder = "Shaders/";
static const std::string defaultShadersBinariesFolder = "Shaders/bin/";
static const std::string defaultAssetsFolder = "Assets/";
static const std::string default3DModelsFolder = "Assets/models/";
static const std::vector<std::string> defaultTexturesFilters = {
	"JPEG files. (*.jpg,*jpeg)", "PNG files. (*.png)" , "DDS files. (*.dds)", "Gif files. (*.gif)"
};
static const std::vector<std::string> defaultTexturesExtensions = {
	"*.jpg;*jpeg", "*.png", "*.dds", "*.gif"
};

#define HWNDWIDTH	(static_cast<unsigned int>(hWndRect.right - hWndRect.left))
#define HWNDHEIGHT (static_cast<unsigned int>(hWndRect.bottom - hWndRect.top))