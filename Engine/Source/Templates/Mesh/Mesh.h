#pragma once

#include "../../Renderer/VertexFormats.h"
#include "../../Renderer/DeviceUtils/VertexBuffer/VertexBuffer.h"
#include "../../Renderer/DeviceUtils/IndexBuffer/IndexBuffer.h"
#include <DirectXCollision.h>
#include <Application.h>

typedef std::tuple<
	std::string //name
> MeshTemplate;

namespace Templates {

	using namespace DeviceUtils;

	struct MeshInstance
	{
		std::string uuid;
		VertexClass vertexClass;
		VertexBufferViewData vbvData;
		IndexBufferViewData	ibvData;
		BoundingBox boundingBox;
	};

	//CREATE
	void CreatePrimitiveMeshTemplate(const std::string uuid, const std::string name);
	std::shared_ptr<MeshInstance> GetMeshInstance(const std::string uuid);
	std::shared_ptr<MeshInstance> GetMeshInstance(const std::string uuid, VertexClass vertexClass, void* vertexData, unsigned int vertexSize, unsigned int verticesCount, const void* indices, unsigned int indicesCount);

	//READ&GET
	std::string GetMeshName(std::string uuid);
	std::vector<UUIDName> GetMeshesUUIDsNames();
	std::string FindMeshUUIDByName(std::string name);

	//UPDATE

	//DESTROY
	void ReleaseMeshTemplates();
	void DestroyMeshInstance(std::shared_ptr<MeshInstance>& mesh);

	//EDITOR
#if defined(_EDITOR)
	void DrawMeshPanel(std::string& mesh, ImVec2 pos, ImVec2 size, bool pop);
#endif

};
