#include "pch.h"
#include "Utils.h"

namespace Utils
{
	std::string GetUtilsPath()
	{
		char* utilsPath;
		size_t utilsPathLen;
		_dupenv_s(&utilsPath, &utilsPathLen, "CULPEO_UTILS");

		return std::string(utilsPath);
	}
}