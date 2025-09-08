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

namespace Templates
{
#include <Attributes/JOrder.h>
#include <Model3DAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
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

	namespace Model3D
	{
		inline static const std::string templateName = "model3ds.json";
		inline static const std::string defaultBaseTexture = "Assets/textures/gridmap.dds";
		inline static const std::string defaultNormalMap = "Assets/textures/bumpmapflat.dds";
	}

	//EDITOR
#if defined(_EDITOR)
	void WriteModel3DsJson(nlohmann::json& json);
#endif

	std::string GetModel3DMeshInstanceUUID(std::string uuid, unsigned int index);
	std::string GetModel3DMaterialInstanceUUID(std::string uuid, unsigned int index);
	std::string GetModel3DMaterialInstanceName(std::string model3dName, unsigned int index);

	struct Model3DInstance
	{
		std::string model3DUUID;

		Model3DInstance(std::string uuid);
		void LoadModel3DInstance();
		void CreateModel3DMaterialsTemplates(const aiScene* aiModel);
		void CreateBoundingBox(BoundingBox& boundingBox, aiMesh* aMesh);
		MaterialJson GetAssimpTexturesMaterialJson(std::filesystem::path relativePath, aiMaterial* material);
#if defined(_DEVELOPMENT)
		void PushAssimpTextureToJson(MaterialJson& j, TextureShaderUsage textureType, std::filesystem::path relativePath, aiString& aiTextureName, std::string fallbackTexture = "", DXGI_FORMAT fallbackFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		MaterialJson CreateModel3DMaterialJson(std::string materialUUID, std::string materialName, std::string vertexShader, std::string pixelShader, aiMaterial* material);
#endif
		VertexClass vertexClass;
		std::vector<std::shared_ptr<MeshInstance>> meshes;
		std::vector<std::string> materialUUIDs;
		//animation
		std::shared_ptr<Animation::Animated> animations = nullptr;
	};

	void DestroyModel3DInstance(std::shared_ptr<Model3DInstance>& model3D);

	TEMPDECL_REFTRACKER(Model3D);
}
