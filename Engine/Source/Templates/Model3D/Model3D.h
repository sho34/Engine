#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>
#include <VertexFormats.h>
#include <Mesh/Mesh.h>
#include <Material/Material.h>
#include <Animated.h>
#include <DirectXCollision.h>
#include <JTemplate.h>
#include <JTypes.h>

namespace Animation { struct Animated; };
namespace Templates { struct TextureJson; struct MaterialJson; };

enum SequenceChannelType
{
	SCT_Animation
};

inline static std::map<SequenceChannelType, std::string> SequenceChannelTypeToStr =
{
	{ SCT_Animation, "animation" },
};

inline static std::map<std::string, SequenceChannelType> StrToSequenceChannelType =
{
	{ "animation", SCT_Animation },
};

struct SequenceChannel
{
	SequenceChannelType type;
	std::string animation;
	int frameStart;
	int frameEnd;

	SequenceChannel()
	{
		type = SCT_Animation;
		animation = "";
		frameStart = 0;
		frameEnd = 100;
	}

	SequenceChannel(const nlohmann::json& j)
	{
		type = StrToSequenceChannelType.at(j.at("type"));
		animation = j.at("animation");
		frameStart = int(j.at("frameStart"));
		frameEnd = int(j.at("frameEnd"));
	}

	nlohmann::json json()
	{
		nlohmann::json j(
			{
				{ "type", SequenceChannelTypeToStr.at(type) },
				{ "animation", animation },
				{ "frameStart", frameStart },
				{ "frameEnd", frameEnd }
			}
		);
		return j;
	}

	bool operator==(const SequenceChannel& other) const {
		if (type != other.type) return false;
		if (animation != other.animation) return false;
		if (frameStart != other.frameStart) return false;
		if (frameEnd != other.frameEnd) return false;
		return true;
	}
};

inline static SequenceChannel ToSequenceChannel(nlohmann::json j)
{
	SequenceChannel seqChannel(j);
	return seqChannel;
}

inline static nlohmann::json FromSequenceChannel(SequenceChannel c)
{
	return c.json();
}

struct Sequence
{
	int framesPerSecond;
	int totalFrames;
	bool loop;
	std::vector<SequenceChannel> channels;

	Sequence()
	{
		framesPerSecond = 60;
		totalFrames = 60;
		loop = false;
	};

	Sequence(std::string animation, int numFrames) : Sequence()
	{
		SequenceChannel seqC;
		seqC.animation = animation;
		seqC.frameEnd = numFrames - 1;
		totalFrames = numFrames;
		channels.push_back(seqC);
	}

	Sequence(nlohmann::json j)
	{
		framesPerSecond = j.at("framesPerSecond");
		totalFrames = j.at("totalFrames");
		loop = j.at("loop");
		for (size_t i = 0ULL; i < j.at("channels").size(); i++)
		{
			channels.push_back(SequenceChannel(j.at("channels").at(i)));
		}
	}

	nlohmann::json json()
	{
		nlohmann::json jchannels = nlohmann::json::array();
		for (auto& channel : channels)
		{
			jchannels.push_back(channel.json());
		}
		nlohmann::json j(
			{
				{ "framesPerSecond", framesPerSecond },
				{ "totalFrames", totalFrames },
				{ "loop", loop },
				{ "channels", jchannels }
			}
		);
		return j;
	}

	bool operator==(const Sequence& other) const {

		if (framesPerSecond != other.framesPerSecond) return false;
		if (totalFrames != other.totalFrames) return false;
		if (loop != other.loop) return false;

		return std::equal(channels.begin(), channels.end(), other.channels.begin());
	}
};

inline static Sequence ToSequence(nlohmann::json j)
{
	Sequence seq(j);
	return seq;
}

inline static nlohmann::json FromSequence(Sequence s)
{
	return s.json();
}

struct AnimationSequences
{
	std::map<std::string, Sequence> sequences;

	AnimationSequences() {}

	AnimationSequences(nlohmann::json j)
	{
		for (nlohmann::json::iterator it = j.begin(); it != j.end(); it++)
		{
			sequences[it.key()] = Sequence(it.value());
		}
	}

	nlohmann::json json()
	{
		nlohmann::json j;
		for (auto& [name, sequence] : sequences)
		{
			j[name] = sequence.json();
		}
		return j;
	}
};

inline static AnimationSequences ToAnimationSequences(nlohmann::json j)
{
	AnimationSequences seq(j);
	return seq;
}

inline static nlohmann::json FromAnimationSequences(AnimationSequences s)
{
	return s.json();
}

namespace Templates
{
#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <Model3DAtt.h>
#include <JEnd.h>

	void Model3DJsonStep();

#endif

	namespace Model3D
	{
		inline static const std::string templateName = "model3ds.json";
		inline static const std::string defaultBaseTexture = "Assets/textures/gridmap.dds";
		inline static const std::string defaultNormalMap = "Assets/textures/bumpmapflat.dds";
		inline static const std::string fallbackMaterialName = "BaseLighting";
		inline static const TemplateType templateType = T_Models3D;
	}

	struct Model3DJson : public JTemplate
	{
		TEMPLATE_DECL(Model3D);

#include <Attributes/JFlags.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <Model3DAtt.h>
#include <JEnd.h>
	};

	TEMPDECL_FULL(Model3D);

	std::string GetModel3DMeshInstanceUUID(std::string uuid, unsigned int index);
	std::string GetModel3DMaterialInstanceUUID(std::string uuid, unsigned int index);
	std::string GetModel3DMaterialInstanceName(std::string uuid, unsigned int index);

	struct Model3DInstance
	{
		JUUID model3DUUID;

		Model3DInstance(JUUID uuid) { assert(!!!"do not use"); }
		explicit Model3DInstance(
			JUUID uuid,
			JUUID objectUUID,
			JObjectChangeCallback cb = [](JUUID) {},
			JObjectChangePostCallback postCb = [](unsigned int, unsigned int) {});
		~Model3DInstance();
		void LoadModel3DInstance();
		void CreateModel3DMaterialsTemplates(const aiScene* aiModel);
		void CreateBoundingBox(BoundingBox& boundingBox, aiMesh* aMesh);
		nlohmann::json GetAssimpTexturesMaterialJson(std::filesystem::path relativePath, const aiScene* aiModel, aiMaterial* material);
#if defined(_DEVELOPMENT)
		void PushAssimpTextureToJson(nlohmann::json& j, TextureShaderUsage textureType, std::filesystem::path relativePath, aiString& aiTextureName, std::string fallbackTexture = "", DXGI_FORMAT fallbackFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		void PushEmbeddedAsimpTextureToJson(nlohmann::json& m, const aiTexture* embeddedTexture, TextureShaderUsage textureType, std::filesystem::path relativePath, aiString& aiTextureName, DXGI_FORMAT fallbackFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		MaterialJson CreateModel3DMaterialJson(JUUID materialUUID, JNAME materialName, JUUID vertexShader, JUUID pixelShader, aiMaterial* material);
#endif
		VertexClass vertexClass;
		std::vector<MeshInstanceUUID> meshes;
		std::vector<JUUID> materialUUIDs;
		//animation
		std::unique_ptr<Animation::Animated> animations;
	};

	TEMPDECL_REFTRACKER(Model3D);
}
