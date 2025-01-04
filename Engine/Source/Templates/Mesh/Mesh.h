#pragma once

#include "../../Renderer/VertexFormats.h"
#include "../../Renderer/DeviceUtils/VertexBuffer/VertexBuffer.h"
#include "../../Renderer/DeviceUtils/IndexBuffer/IndexBuffer.h"

namespace Templates::Mesh {
	
	struct Mesh
	{
		std::string name;
		VertexClass vertexClass;
		VertexBufferViewDataT vbvData;
		IndexBufferViewDataT	ibvData;
		bool loading = false;
	};

	typedef void LoadMeshCallback(std::shared_ptr<Mesh>& mesh);
	typedef Concurrency::task<void> LoadPrimitiveIntoMesh(std::shared_ptr<Mesh>& mesh);

	Concurrency::task<void> CreatePrimitiveMeshTemplate(std::string meshName, LoadMeshCallback loadFn = nullptr);
	void ReleaseMeshTemplates();
	std::shared_ptr<Mesh> GetMeshTemplate(std::string meshName);
	std::vector<std::string> GetMeshesNames();
#if defined(_EDITOR)
	void SelectMesh(std::string meshName, void*& ptr);
	void DrawMeshPanel(void*& ptr, ImVec2 pos, ImVec2 size);
	std::string GetMeshName(void* ptr);
#endif
};
typedef Templates::Mesh::Mesh MeshT;
typedef std::shared_ptr<MeshT> MeshPtr;
