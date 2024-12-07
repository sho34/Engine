#include "pch.h"
#include "Mesh.h"

extern std::mutex rendererMutex;

namespace Templates::Mesh {

	static std::map<std::wstring, MeshPtr> meshTemplates;

	Concurrency::task<void> CreateMeshTemplate(std::wstring meshName, std::shared_ptr<Renderer>& renderer, LoadPrimitiveIntoMesh meshLoader, LoadMeshCallback loadFn)
	{
		auto currentMesh = GetMeshTemplate(meshName);
		if (currentMesh != nullptr) {
			if (loadFn) loadFn(currentMesh);
			return concurrency::create_task([]{});
		}

		return concurrency::create_task([meshName, meshLoader, &renderer] {
			std::lock_guard<std::mutex> lock(rendererMutex); 

			MeshPtr mesh = std::make_shared<Mesh>();
			mesh->loading = true;
			meshTemplates.insert(std::pair<std::wstring, MeshPtr>(meshName, mesh));

			meshLoader(renderer, mesh).wait();
			return mesh;
		}).then([loadFn](MeshPtr mesh) {
			if (loadFn) loadFn(mesh);
			mesh->loading = false;
		});

		//return concurrency::create_task([] {});
		/*
		return concurrency::create_task([&renderer, meshName, meshLoader] {
			std::lock_guard<std::mutex> lock(rendererMutex);

			MeshPtr mesh = std::make_shared<Mesh>();
			mesh->loading = true;
			meshTemplates.insert(std::pair<std::wstring, MeshPtr>(meshName, mesh));

			meshLoader(renderer, mesh).wait();
			return mesh;
		}).then([loadFn](MeshPtr mesh) {
			if (loadFn) loadFn(mesh);
			mesh->loading = false;
		});
		*/
		/*
		std::lock_guard<std::mutex> lock(rendererMutex);

		MeshPtr mesh = std::make_shared<Mesh>();
		mesh->loading = true;
		meshTemplates.insert(std::pair<std::wstring, MeshPtr>(meshName, mesh));

		return concurrency::create_task([&renderer, &mesh, meshLoader, loadFn]() {

			meshLoader(renderer, mesh).then([loadFn,&mesh] {
				if (loadFn) loadFn(mesh);
				mesh->loading = false;
			});
		});
		*/
	}

	MeshPtr GetMeshTemplate(const std::wstring meshName)
	{
		auto it = meshTemplates.find(meshName);
		return (it != meshTemplates.end()) ? it->second : nullptr;
	}

};
