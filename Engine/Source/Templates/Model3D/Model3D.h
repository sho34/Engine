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

namespace Animation { struct Animated; };
namespace Templates {

	namespace Model3D
	{
		inline static const std::string templateName = "model3d.json";
#if defined(_EDITOR)
		void ReleaseRenderablesInstances();
#endif
	}

	struct Model3DInstance
	{
		std::string name;
		VertexClass vertexClass;
		std::vector<std::vector<byte>> vertices;
		std::vector<std::shared_ptr<MeshInstance>> meshes;
		std::vector<std::shared_ptr<MaterialInstance>> materials;
		std::vector<std::string> materialNames;
		nlohmann::json shaderAttributes;

		//animation
		std::shared_ptr<Animation::Animated> animations = nullptr;
		BoundingBox GetAnimatedBoundingBox(XMMATRIX* bones);
		std::shared_ptr<MaterialInstance> GetModel3DMaterialInstance(unsigned int meshIndex);
	};

	//CREATE
	void CreateModel3D(std::string name, nlohmann::json json);
	void LoadModel3DInstance(std::shared_ptr<Model3DInstance>& model, std::string name, nlohmann::json shaderAttributes);

#if defined(_DEVELOPMENT)
	nlohmann::json CreateModel3DMaterialJson(std::string shader, std::filesystem::path relativePath, aiMaterial* material);
#endif
	void CreateBoundingBox(BoundingBox& boundingBox, aiMesh* aMesh);

	//READ&GET
	std::shared_ptr<Model3DInstance> GetModel3DInstance(std::string name, nlohmann::json shaderAttributes);
	std::vector<std::string> GetModels3DNames();
	std::string GetModel3DMeshInstanceName(std::string name, unsigned int index);
	std::string GetModel3DMaterialInstanceName(std::string name, unsigned int index);

	//UPDATE

	//DESTROY
	void DestroyModel3DInstance(std::shared_ptr<Model3DInstance>& model3D);
	void ReleaseModel3DTemplates();

	//EDITOR
#if defined(_EDITOR)
	void BindNotifications(std::string model, std::shared_ptr<Scene::Renderable> renderable);
	void UnbindNotifications(std::string model, std::shared_ptr<Scene::Renderable> renderable);
	void DrawModel3DPanel(std::string& model, ImVec2 pos, ImVec2 size, bool pop);
	std::string GetModel3DInstanceTemplateName(std::shared_ptr<Model3DInstance> model3D);
	void DeleteModel3D(std::string name);
	//nlohmann::json json();
#endif
}
