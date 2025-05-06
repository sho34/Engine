#include "pch.h"
#include "Texture.h"
#include <UUID.h>
#include "../Templates.h"
#include "../TemplateDef.h"

namespace Templates
{
	//uuid to TextureTemplates
	std::map<std::string, TextureTemplate> textures;
	TemplatesContainer<TextureTemplate>& GetTextureTemplates()
	{
		return textures;
	}

	TEMPDEF_FULL(Texture);

	void CreateTexturesTemplatesFromMaterial(nlohmann::json json)
	{
		if (!json.contains("textures")) return;
		for (auto& [textureType, texture] : json.at("textures").items())
		{
			if (texture.type() != nlohmann::json::value_t::object) continue;

			std::string pattern = "\\\\";
			std::regex reg(pattern);
			std::string replacement = "/";

			nlohmann::json texj = nlohmann::json({});
			texj["uuid"] = getUUID();
			texj["name"] = std::regex_replace(std::string(texture.at("path")), reg, replacement);
			texj["numFrames"] = texture.at("numFrames");
			texj["format"] = texture.at("format");
			CreateTexture(texj);
		}
	}

	void DestroyTexture(std::string uuid)
	{
	}

	void ReleaseTexturesTemplates()
	{
	}

#if defined(_EDITOR)
	void DrawTexturePanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop)
	{

	}

	void CreateNewTexture()
	{
	}

	void DeleteTexture(std::string uuid)
	{
	}

	void DrawTexturesPopups()
	{
	}

	void WriteTexturesJson(nlohmann::json& json)
	{
	}


#endif
}