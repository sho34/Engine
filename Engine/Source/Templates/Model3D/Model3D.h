#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>
#include "../../Renderer/VertexFormats.h"
#include "../Mesh/Mesh.h"
#include "../Material/Material.h"
#include "../../Animation/Animated.h"
#include <DirectXCollision.h>

typedef std::tuple<
	std::string, //name
	nlohmann::json //data
#if defined(_EDITOR)
	,
	std::vector<std::shared_ptr<Scene::Renderable>>
#endif
> Model3DTemplate;

namespace Animation { struct Animated; };
namespace Templates {

#if defined(_EDITOR)
	enum Model3DPopupModal
	{
		Model3DPopupModal_CannotDelete = 1,
		Model3DPopupModal_CreateNew = 2
	};
#endif

	namespace Model3D
	{
		inline static const std::string templateName = "model3d.json";
#if defined(_EDITOR)
		void ReleaseRenderablesInstances();
		void DrawEditorInformationAttributes(std::string uuid);
		void DrawEditorAssetAttributes(std::string uuid);
		void DrawEditorMaterialAttributes(std::string uuid);
#endif
	}

	struct Model3DInstance
	{
		std::string uuid;
		VertexClass vertexClass;
		std::vector<std::shared_ptr<MeshInstance>> meshes;
		std::vector<std::shared_ptr<MaterialInstance>> materials;
		std::vector<std::string> materialUUIDs;
		nlohmann::json shaderAttributes;

		//animation
		std::shared_ptr<Animation::Animated> animations = nullptr;
		std::shared_ptr<MaterialInstance> GetModel3DMaterialInstance(unsigned int meshIndex);
	};

	//CREATE
	void CreateModel3D(nlohmann::json json);
	void LoadModel3DInstance(std::shared_ptr<Model3DInstance>& model, std::string uuid, nlohmann::json shaderAttributes);

#if defined(_DEVELOPMENT)
	nlohmann::json CreateModel3DMaterialJson(std::string materialUUID, std::string materialName, std::string vertexShader, std::string pixelShader, std::filesystem::path relativePath, aiMaterial* material);
#endif
	void CreateBoundingBox(BoundingBox& boundingBox, aiMesh* aMesh);

	//READ&GET
	Model3DTemplate GetModel3DTemplate(std::string uuid);
	std::shared_ptr<Model3DInstance> GetModel3DInstance(std::string uuid, nlohmann::json shaderAttributes);
	std::vector<std::string> GetModels3DNames();
	std::string GetModel3DName(std::string uuid);
	std::vector<UUIDName> GetModels3DUUIDsNames();
	std::string GetModel3DMeshInstanceUUID(std::string uuid, unsigned int index);
	std::string GetModel3DMaterialInstanceUUID(std::string uuid, unsigned int index);
	std::string GetModel3DMaterialInstanceName(std::string model3dName, unsigned int index);

	//UPDATE

	//DESTROY
	void DestroyModel3DInstance(std::shared_ptr<Model3DInstance>& model3D);
	void ReleaseModel3DTemplates();

	//EDITOR
#if defined(_EDITOR)
	void BindNotifications(std::string uuid, std::shared_ptr<Scene::Renderable> renderable);
	void UnbindNotifications(std::string uuid, std::shared_ptr<Scene::Renderable> renderable);
	void DrawModel3DPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop);
	void CreateNewModel3D();
	void DeleteModel3D(std::string uuid);
	void DrawModels3DsPopups();
	void WriteModel3DsJson(nlohmann::json& json);
#endif
}
