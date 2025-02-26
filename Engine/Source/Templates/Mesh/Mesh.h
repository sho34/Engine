#pragma once

#include "../../Renderer/VertexFormats.h"
#include "../../Renderer/DeviceUtils/VertexBuffer/VertexBuffer.h"
#include "../../Renderer/DeviceUtils/IndexBuffer/IndexBuffer.h"

namespace Templates {

	/*
	struct Mesh
	{
		std::string name;
		bool primitive = false;
	};
	*/

	using namespace DeviceUtils;

	struct MeshInstance
	{
		std::string name;
		VertexClass vertexClass;
		VertexBufferViewData vbvData;
		IndexBufferViewData	ibvData;
		BoundingBox boundingBox;
	};

	//CREATE
	void CreatePrimitiveMeshTemplate(const std::string name);
	std::shared_ptr<MeshInstance> GetMeshInstance(const std::string name);
	std::shared_ptr<MeshInstance> GetMeshInstance(const std::string name, VertexClass vertexClass, void* vertexData, unsigned int vertexSize, unsigned int verticesCount, const void* indices, unsigned int indicesCount);

	//READ&GET
	//std::shared_ptr<Mesh> GetMeshTemplate(std::string meshName);
	std::vector<std::string> GetMeshesNames();

	//UPDATE

	//DESTROY
	void ReleaseMeshTemplates();
	void DestroyMeshInstance(std::shared_ptr<MeshInstance>& mesh);

	//EDITOR
#if defined(_EDITOR)
	/*
	void SelectMesh(std::string meshName, void*& ptr);
	*/
	void DrawMeshPanel(std::string& mesh, ImVec2 pos, ImVec2 size, bool pop);
	/*
	std::string GetMeshName(void* ptr);
	*/
#endif

};
