#pragma once
#include "../Templates/Mesh/Mesh.h"
#include "../Renderer/Renderer.h"
#include "Cube.h"
#include "Decal.h"
#include "Floor.h"
#include "Pentahedron.h"
#include "UtahTeapot.h"
#include "BoxLines.h"

extern std::shared_ptr<Renderer> renderer;
namespace Primitives {

	using namespace Templates;

	template<typename T>
	void LoadPrimitiveIntoMesh(const std::shared_ptr<MeshInstance>& mesh) {
		mesh->vertexClass = T::VertexClass;

		BoundingBox::CreateFromPoints(mesh->boundingBox, _countof(T::vertices), &T::vertices[0].Position, sizeof(T::VertexType));

		//upload the vertex buffer to the GPU and create the vertex buffer view
		InitializeVertexBufferView(renderer->d3dDevice, renderer->commandList, T::vertices, sizeof(T::VertexType), _countof(T::vertices), mesh->vbvData);

		//upload the index buffer to the GPU and create the index buffer view
		InitializeIndexBufferView(renderer->d3dDevice, renderer->commandList, T::indices, _countof(T::indices), mesh->ibvData);
	}
}

static const std::map<std::string, std::function<void(const std::shared_ptr<Templates::MeshInstance>&)>> LoadPrimitiveIntoMeshFunctions =
{
	{ "utahteapot", Primitives::LoadPrimitiveIntoMesh<UtahTeapot> },
	{ "cube", Primitives::LoadPrimitiveIntoMesh<Cube> },
	{ "pyramid", Primitives::LoadPrimitiveIntoMesh<Pentahedron> },
	{ "floor", Primitives::LoadPrimitiveIntoMesh<Floor> },
	{ "decal", Primitives::LoadPrimitiveIntoMesh<Decal> },
	{ "boxlines", Primitives::LoadPrimitiveIntoMesh<BoxLines> }
};
