#include "pch.h"
#include "Mesh.h"
#include "../../Primitives/Primivites.h"
#include "../../Renderer/Renderer.h"

//extern std::mutex rendererMutex;
namespace Templates {

	static std::set<std::string> meshTemplates;
	static nostd::RefTracker<std::string, std::shared_ptr<MeshInstance>> refTracker;

	//CREATE
	void CreatePrimitiveMeshTemplate(const std::string name)
	{
		meshTemplates.insert(name);
	}

	std::shared_ptr<MeshInstance> GetMeshInstance(const std::string name)
	{
		assert(meshTemplates.contains(name));

		return refTracker.AddRef(name, [name]()
			{
				std::shared_ptr<MeshInstance> instance = std::make_shared<MeshInstance>();
				instance->name = name;
				LoadPrimitiveIntoMeshFunctions.at(name)(instance);
				return instance;
			}
		);
	}

	std::shared_ptr<MeshInstance> GetMeshInstance(const std::string name, VertexClass vertexClass, void* vertexData, unsigned int vertexSize, unsigned int verticesCount, const void* indices, unsigned int indicesCount)
	{
		return refTracker.AddRef(name, [name, vertexClass, vertexData, vertexSize, verticesCount, indices, indicesCount]()
			{
				std::shared_ptr<MeshInstance> instance = std::make_shared<MeshInstance>();
				instance->name = name;
				instance->vertexClass = vertexClass;
				InitializeVertexBufferView(renderer->d3dDevice, renderer->commandList, vertexData, vertexSize, verticesCount, instance->vbvData);
				InitializeIndexBufferView(renderer->d3dDevice, renderer->commandList, indices, indicesCount, instance->ibvData);
				return instance;
			}
		);
	}

	//READ&GET
	std::vector<std::string> GetMeshesNames() {
		return nostd::GetKeysFromSet(meshTemplates);
	}

	//UPDATE

	//DESTROY
	void ReleaseMeshTemplates()
	{
		refTracker.Clear();
		meshTemplates.clear();
	}

	void DestroyMeshInstance(std::shared_ptr<MeshInstance>& mesh)
	{
		refTracker.RemoveRef(mesh->name, mesh);
	}


#if defined(_EDITOR)
	void DrawMeshPanel(std::string& mesh, ImVec2 pos, ImVec2 size, bool pop)
	{
	}
#endif
};
