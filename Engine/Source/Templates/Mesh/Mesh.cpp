#include "pch.h"
#include "Mesh.h"
#include "../../Primitives/Primivites.h"
#include "../../Renderer/Renderer.h"
#include <set>
#include <memory>
#include <NoStd.h>
#include "../Templates.h"
#include <Application.h>

//extern std::mutex rendererMutex;
namespace Templates {

	std::map<std::string, MeshTemplate> meshes;

	namespace Mesh
	{
		static nostd::RefTracker<std::string, std::shared_ptr<MeshInstance>> refTracker;
	}

	//CREATE
	void CreatePrimitiveMeshTemplate(const std::string uuid, const std::string name)
	{
		MeshTemplate t;

		std::string& n = std::get<0>(t);
		n = name;

		meshes.insert_or_assign(uuid, t);
	}

	std::shared_ptr<MeshInstance> GetMeshInstance(const std::string uuid)
	{
		if (!meshes.contains(uuid))
		{
			assert(!!!"mesh uuid not present");
		}

		std::string name = std::get<0>(meshes.at(uuid));

		using namespace Mesh;
		return refTracker.AddRef(uuid, [uuid, name]()
			{
				std::shared_ptr<MeshInstance> instance = std::make_shared<MeshInstance>();
				instance->uuid = uuid;
				LoadPrimitiveIntoMeshFunctions.at(name)(instance);
				return instance;
			}
		);
	}

	std::shared_ptr<MeshInstance> GetMeshInstance(const std::string uuid, VertexClass vertexClass, void* vertexData, unsigned int vertexSize, unsigned int verticesCount, const void* indices, unsigned int indicesCount)
	{
		using namespace Mesh;
		return refTracker.AddRef(uuid, [uuid, vertexClass, vertexData, vertexSize, verticesCount, indices, indicesCount]()
			{
				std::shared_ptr<MeshInstance> instance = std::make_shared<MeshInstance>();
				instance->uuid = uuid;
				instance->vertexClass = vertexClass;
				InitializeVertexBufferView(renderer->d3dDevice, renderer->commandList, vertexData, vertexSize, verticesCount, instance->vbvData);
				InitializeIndexBufferView(renderer->d3dDevice, renderer->commandList, indices, indicesCount, instance->ibvData);
				return instance;
			}
		);
	}

	void CreateMeshVerticesShaderResourceView(std::shared_ptr<MeshInstance>& mesh, CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
	{
		AllocCSUDescriptor(cpuHandle, gpuHandle);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = mesh->vbvData.vertexBufferView.SizeInBytes / mesh->vbvData.vertexBufferView.StrideInBytes;
		srvDesc.Buffer.StructureByteStride = mesh->vbvData.vertexBufferView.StrideInBytes;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		renderer->d3dDevice->CreateShaderResourceView(mesh->vbvData.vertexBuffer, &srvDesc, cpuHandle);
	}

	std::string GetMeshName(std::string uuid)
	{
		return std::get<0>(meshes.at(uuid));
	}

	std::vector<UUIDName> GetMeshesUUIDsNames()
	{
		return GetUUIDsNames(meshes);
	}

	std::string FindMeshUUIDByName(std::string name)
	{
		for (auto& [meshUUID, meshTemplate] : meshes)
		{
			if (std::get<0>(meshTemplate) == name) return meshUUID;
		}

		assert(!!!"mesh not found");
		return "";
	}

	//UPDATE

	//DESTROY
	void ReleaseMeshTemplates()
	{
		using namespace Mesh;
		refTracker.Clear();
		meshes.clear();
	}

	void DestroyMeshInstance(std::shared_ptr<MeshInstance>& mesh)
	{
		using namespace Mesh;
		refTracker.RemoveRef(mesh->uuid, mesh);
	}


#if defined(_EDITOR)
	void DrawMeshPanel(std::string& mesh, ImVec2 pos, ImVec2 size, bool pop)
	{
	}
#endif
};
