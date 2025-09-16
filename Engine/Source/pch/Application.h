#pragma once

static const std::string defaultLevelsFolder = "Levels/";
static const std::string defaultTemplatesFolder = "Templates/";
static const std::string defaultShadersFolder = "Shaders/";
static const std::string defaultShadersBinariesFolder = "Shaders/bin/";
static const std::string defaultAssetsFolder = "Assets/";
static const std::string default3DModelsFolder = "Assets/models/";
static const std::string defaultSoundsFolder = "Assets/sounds/";
static const std::vector<std::string> defaultTexturesFilters = {
	"All Image files. (*.jpg,*jpeg,*.png)","JPEG files. (*.jpg,*jpeg)", "PNG files. (*.png)"
};
static const std::vector<std::string> defaultTexturesExtensions = {
	"*.jpg;*jpeg;*.png","*.jpg;*jpeg", "*.png"
};
static const std::vector<std::string> defaultAnimatedTexturesFilters = {
	"Gif files. (*.gif)"
};
static const std::vector<std::string> defaultAnimatedTexturesExtensions = {
	"*.gif"
};
static const std::vector<std::string> cubeTextureAxesNames = { "X+" , "X-" , "Y+" , "Y-" , "Z+" , "Z-" , };

#define HWNDWIDTH	(static_cast<unsigned int>(hWndRect.right - hWndRect.left))
#define HWNDHEIGHT (static_cast<unsigned int>(hWndRect.bottom - hWndRect.top))