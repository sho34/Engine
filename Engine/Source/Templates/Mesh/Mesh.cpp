#include "pch.h"
#include "Mesh.h"
#include "../../Primitives/Primivites.h"
#include "../../Renderer/Renderer.h"

//extern std::mutex rendererMutex;
namespace Templates {

	std::set<std::string> meshTemplates;
	std::map<std::string, std::shared_ptr<MeshInstance>> meshInstances;
	std::map<std::shared_ptr<MeshInstance>, unsigned int> meshInstancesRefCount;

	//CREATE
	void CreatePrimitiveMeshTemplate(const std::string name)
	{
		meshTemplates.insert(name);
	}

	std::shared_ptr<MeshInstance> GetMeshInstance(const std::string name)
	{
		assert(meshTemplates.contains(name));

		std::shared_ptr<MeshInstance> mesh = nullptr;

		if (meshInstances.contains(name))
		{
			mesh = meshInstances.at(name);
		}
		else
		{
			mesh = std::make_shared<MeshInstance>();
			mesh->name = name;
			LoadPrimitiveIntoMeshFunctions.at(name)(mesh);
			meshInstances.insert_or_assign(name, mesh);
			meshInstancesRefCount.insert_or_assign(mesh, 0U);
		}

		meshInstancesRefCount.find(mesh)->second++;
		return mesh;
	}

	std::shared_ptr<MeshInstance> GetMeshInstance(const std::string name, VertexClass vertexClass, void* vertexData, unsigned int vertexSize, unsigned int verticesCount, const void* indices, unsigned int indicesCount)
	{
		std::shared_ptr<MeshInstance> mesh = nullptr;

		if (meshInstances.contains(name))
		{
			mesh = meshInstances.at(name);
		}
		else
		{
			mesh = std::make_shared<MeshInstance>();
			mesh->name = name;
			mesh->vertexClass = vertexClass;
			InitializeVertexBufferView(renderer->d3dDevice, renderer->commandList, vertexData, vertexSize, verticesCount, mesh->vbvData);
			InitializeIndexBufferView(renderer->d3dDevice, renderer->commandList, indices, indicesCount, mesh->ibvData);
			meshInstances.insert_or_assign(name, mesh);
			meshInstancesRefCount.insert_or_assign(mesh, 0U);
		}

		meshInstancesRefCount.find(mesh)->second++;
		return mesh;
	}

	//READ&GET
	std::vector<std::string> GetMeshesNames() {
		return nostd::GetKeysFromSet(meshTemplates);
	}

	//UPDATE

	//DESTROY
	void ReleaseMeshTemplates()
	{
		meshInstancesRefCount.clear();
		meshInstances.clear();
		meshTemplates.clear();
	}

	void DestroyMeshInstance(std::shared_ptr<MeshInstance>& mesh)
	{
		if (!meshInstancesRefCount.contains(mesh)) return;

		meshInstancesRefCount.at(mesh)--;
		if (meshInstancesRefCount.at(mesh) == 0U)
		{
			std::string name = mesh->name;
			meshInstancesRefCount.erase(mesh);
			meshInstances.erase(name);
		}
		mesh = nullptr;
	}


#if defined(_EDITOR)
	void DrawMeshPanel(std::string& mesh, ImVec2 pos, ImVec2 size, bool pop)
	{
	}
#endif
};
