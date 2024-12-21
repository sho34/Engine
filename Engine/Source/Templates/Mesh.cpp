#include "pch.h"
#include "Mesh.h"
#include "../Primitives/Primivites.h"
#include "../Renderer/Renderer.h"

extern std::shared_ptr<Renderer> renderer;
extern std::mutex rendererMutex;

namespace Templates::Mesh {

	static std::map<std::wstring, MeshPtr> meshTemplates;

	Concurrency::task<void> CreatePrimitiveMeshTemplate(std::wstring meshName, LoadMeshCallback loadFn)
	{
		auto currentMesh = GetMeshTemplate(meshName);
		if (currentMesh != nullptr) {
			if (loadFn) loadFn(currentMesh);
			return concurrency::create_task([] {});
		}

		return concurrency::create_task([meshName] {
			std::lock_guard<std::mutex> lock(rendererMutex);

			MeshPtr mesh = std::make_shared<Mesh>();
			mesh->name = meshName;
			mesh->loading = true;
			meshTemplates.insert(std::pair<std::wstring, MeshPtr>(meshName, mesh));

			LoadPrimitiveIntoMeshFunctions[meshName](renderer, mesh).wait();
			return mesh;
		}).then([loadFn](MeshPtr mesh) {
			if (loadFn) loadFn(mesh);
			mesh->loading = false;
		});
	}

	void ReleaseMeshTemplates()
	{

		for (auto& [name, mesh] : meshTemplates) {
			mesh->vbvData.vertexBuffer = nullptr;
			mesh->vbvData.vertexBufferUpload = nullptr;
			mesh->ibvData.indexBuffer = nullptr;
			mesh->ibvData.indexBufferUpload = nullptr;
		}

		meshTemplates.clear();
	}

	MeshPtr GetMeshTemplate(const std::wstring meshName)
	{
		auto it = meshTemplates.find(meshName);
		return (it != meshTemplates.end()) ? it->second : nullptr;
	}

};
