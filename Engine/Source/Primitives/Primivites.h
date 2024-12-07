#pragma once
#include "../Templates/Mesh.h"
#include "../Renderer/DeviceUtils.h"

extern std::mutex rendererMutex;
namespace Primitives {

	template<typename T>
	Concurrency::task<void> LoadPrimitiveIntoMesh(std::shared_ptr<Renderer>& renderer, MeshPtr& mesh) {
		return concurrency::create_task([&renderer, &mesh] {
			//std::lock_guard<std::mutex> lock(rendererMutex);
			mesh->vertexClass = T::VertexClass;

			//upload the vertex buffer to the GPU and create the vertex buffer view
			InitializeVertexBufferView(renderer->d3dDevice, renderer->commandList, T::vertices, sizeof(T::VertexType), _countof(T::vertices), mesh->vbvData);

			//upload the index buffer to the GPU and create the index buffer view
			InitializeIndexBufferView(renderer->d3dDevice, renderer->commandList, T::indices, _countof(T::indices), mesh->ibvData);
		});
	}
}