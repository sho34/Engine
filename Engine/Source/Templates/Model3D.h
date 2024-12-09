#pragma once

#include "../Renderer/Renderer.h"
#include "../Renderer/VertexFormats.h"
//#include "../Animation/Animation3D.h"
#include "../Templates/Mesh.h"

namespace Animation { struct Animated; };
namespace Templates::Model3D {

	using namespace Templates::Mesh;

	struct Model3DDefinition {
		bool autoCreateMaterial = true;
		std::wstring materialsShader = L"";
	};

	struct Model3D
	{
		bool loading = true;

		//animation
		std::shared_ptr<Animation::Animated> animations = nullptr;

		VertexClass vertexClass;
		std::vector<std::shared_ptr<byte*>> vertices;
		std::vector<UINT> numVertices;

		std::vector<std::vector<UINT32>> indices;

		std::vector<MeshPtr> meshes;
	};


	Concurrency::task<void> CreateModel3DTemplate(std::wstring model3DName, std::wstring assetPath, std::shared_ptr<Renderer>& renderer, Model3DDefinition params = {});
	void ReleaseModel3DTemplates();
	std::shared_ptr<Model3D> GetModel3DTemplate(std::wstring model3DName);
}
typedef Templates::Model3D::Model3D Model3DT;
typedef std::shared_ptr<Model3DT> Model3DPtr;