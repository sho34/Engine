#pragma once

#include "../../Renderer/VertexFormats.h"
#include "../Mesh/MeshImpl.h"

namespace Animation { struct Animated; };
namespace Templates::Model3D {

	static const std::string templateName = "model3d.json";
	static const std::string assetsRootFolder = "Assets/models/";

	struct Model3DDefinition {
		bool autoCreateMaterial = true;
		std::string materialsShader = "";
	};

	struct Model3D
	{
		bool loading = true;
		std::string name;
		std::string assetPath;

		Model3DDefinition model3dDefinition;

		//animation
		std::shared_ptr<Animation::Animated> animations = nullptr;

		VertexClass vertexClass;
		std::vector<std::shared_ptr<byte*>> vertices;
		std::vector<UINT> numVertices;

		std::vector<std::vector<UINT32>> indices;

		std::vector<MeshPtr> meshes;
	};

	std::string BuildMeshName(std::string model3DName, UINT meshIndex);
	std::string BuildMaterialName(std::string model3DName, UINT meshIndex);

	void ReleaseModel3DTemplates();
	std::shared_ptr<Model3D> GetModel3DTemplate(std::string model3DName);
	std::vector<std::string> GetModels3DNames();

#if defined(_EDITOR)
	void SelectModel3D(std::string model3DName, void*& ptr);
	void DrawModel3DPanel(void*& ptr, ImVec2 pos, ImVec2 size);
	std::string GetModel3DName(void* ptr);
	nlohmann::json json();
#endif

	Concurrency::task<void> json(std::string, nlohmann::json);
}
typedef Templates::Model3D::Model3D Model3DT;
typedef std::shared_ptr<Model3DT> Model3DPtr;