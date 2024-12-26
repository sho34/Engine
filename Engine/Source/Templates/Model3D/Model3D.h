#pragma once

#include "../../Renderer/VertexFormats.h"
#include "../Mesh/MeshImpl.h"

namespace Animation { struct Animated; };
namespace Templates::Model3D {

	static const std::wstring templateName = L"model3d.json";
	static const std::wstring assetsRootFolder = L"Assets/models/";

	struct Model3DDefinition {
		bool autoCreateMaterial = true;
		std::wstring materialsShader = L"";
	};

	struct Model3D
	{
		bool loading = true;
		std::wstring assetPath;

		Model3DDefinition model3dDefinition;

		//animation
		std::shared_ptr<Animation::Animated> animations = nullptr;

		VertexClass vertexClass;
		std::vector<std::shared_ptr<byte*>> vertices;
		std::vector<UINT> numVertices;

		std::vector<std::vector<UINT32>> indices;

		std::vector<MeshPtr> meshes;
	};

	std::wstring getMeshName(std::wstring model3DName, UINT meshIndex);
	std::wstring getMaterialName(std::wstring model3DName, UINT meshIndex);

	void ReleaseModel3DTemplates();
	std::shared_ptr<Model3D> GetModel3DTemplate(std::wstring model3DName);
	std::map<std::wstring, std::shared_ptr<Model3D>> GetNamedModels3D();
	std::vector<std::wstring> GetModels3DNames();

#if defined(_EDITOR)
	nlohmann::json json();
#endif
	Concurrency::task<void> json(std::wstring, nlohmann::json);
}
typedef Templates::Model3D::Model3D Model3DT;
typedef std::shared_ptr<Model3DT> Model3DPtr;