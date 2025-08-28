#pragma once
#include <Mesh/Mesh.h>
#include <Renderer.h>
#include "Cube.h"
#include "Decal.h"
#include "Floor.h"
#include "Pentahedron.h"
#include "UtahTeapot.h"
#include "BoxLines.h"
#include "Sphere.h"
#include "Cone.h"

extern std::shared_ptr<Renderer> renderer;
namespace Primitives {

	using namespace Templates;

	template<typename T>
	void LoadPrimitiveIntoMesh(const std::shared_ptr<MeshInstance>& mesh, void* params) {
		mesh->vertexClass = T::VertexClass;
		T p(params);

		std::vector<uint32_t> indices = p.GetIndices();
		std::vector<Vertex<T::VertexClass>> vertices = p.GetVertices();

		//BoundingBox::CreateFromPoints(mesh->boundingBox, _countof(T::vertices), &T::vertices[0].Position, sizeof(T::VertexType));
		BoundingBox::CreateFromPoints(mesh->boundingBox, vertices.size(), &vertices.at(0).Position, sizeof(T::VertexType));

		//upload the vertex buffer to the GPU and create the vertex buffer view
		//InitializeVertexBufferView(renderer->d3dDevice, renderer->commandList, T::vertices, sizeof(T::VertexType), _countof(T::vertices), mesh->vbvData);
		InitializeVertexBufferView(renderer->d3dDevice, renderer->commandList, vertices.data(), sizeof(T::VertexType), static_cast<unsigned int>(vertices.size()), mesh->vbvData);

		//upload the index buffer to the GPU and create the index buffer view
		//InitializeIndexBufferView(renderer->d3dDevice, renderer->commandList, T::indices, _countof(T::indices), mesh->ibvData);
		InitializeIndexBufferView(renderer->d3dDevice, renderer->commandList, indices.data(), static_cast<unsigned int>(indices.size()), mesh->ibvData);
	}
}

static const std::map<std::string, std::function<void(const std::shared_ptr<Templates::MeshInstance>&, void* params)>> LoadPrimitiveIntoMeshFunctions =
{
	{ "utahteapot", Primitives::LoadPrimitiveIntoMesh<UtahTeapot> },
	{ "cube", Primitives::LoadPrimitiveIntoMesh<Cube> },
	{ "pyramid", Primitives::LoadPrimitiveIntoMesh<Pentahedron> },
	{ "floor", Primitives::LoadPrimitiveIntoMesh<Floor> },
	{ "decal", Primitives::LoadPrimitiveIntoMesh<Decal> },
	{ "boxlines", Primitives::LoadPrimitiveIntoMesh<BoxLines> },
	{ "sphere", Primitives::LoadPrimitiveIntoMesh<Sphere> },
	{ "cone", Primitives::LoadPrimitiveIntoMesh<Cone> },
};
