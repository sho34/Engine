#pragma once
#include "../Templates/Mesh/MeshImpl.h"
#include "../Renderer/Renderer.h"
#include "Cube.h"
#include "Decal.h"
#include "Floor.h"
#include "Pentahedron.h"
#include "UtahTeapot.h"

extern std::mutex rendererMutex;
extern std::shared_ptr<Renderer> renderer;
namespace Primitives {

	template<typename T>
	Concurrency::task<void> LoadPrimitiveIntoMesh(MeshPtr& mesh) {
		return concurrency::create_task([&mesh] {
			mesh->vertexClass = T::VertexClass;

			//upload the vertex buffer to the GPU and create the vertex buffer view
			InitializeVertexBufferView(renderer->d3dDevice, renderer->commandList, T::vertices, sizeof(T::VertexType), _countof(T::vertices), mesh->vbvData);

			//upload the index buffer to the GPU and create the index buffer view
			InitializeIndexBufferView(renderer->d3dDevice, renderer->commandList, T::indices, _countof(T::indices), mesh->ibvData);
		});
	}
}

typedef Concurrency::task<void> (*LoadPrimitiveIntoMeshPtr)(MeshPtr& mesh);
static std::map<std::wstring, LoadPrimitiveIntoMeshPtr> LoadPrimitiveIntoMeshFunctions = {
	{ L"utahteapot", Primitives::LoadPrimitiveIntoMesh<UtahTeapot> },
	{ L"cube", Primitives::LoadPrimitiveIntoMesh<Cube> },
	{ L"pyramid", Primitives::LoadPrimitiveIntoMesh<Pentahedron> },
	{ L"floor", Primitives::LoadPrimitiveIntoMesh<Floor> },
	{ L"decal", Primitives::LoadPrimitiveIntoMesh<Decal> }
};