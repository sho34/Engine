#pragma once

#include "../../Renderer/VertexFormats.h"
#include "../../Renderer/DeviceUtils/VertexBuffer/VertexBuffer.h"
#include "../../Renderer/DeviceUtils/IndexBuffer/IndexBuffer.h"

namespace Templates::Mesh {
	
	struct Mesh
	{
		std::wstring name;
		VertexClass vertexClass;
		VertexBufferViewDataT vbvData;
		IndexBufferViewDataT	ibvData;
		bool loading = false;
	};

	typedef void LoadMeshCallback(std::shared_ptr<Mesh>& mesh);
	typedef Concurrency::task<void> LoadPrimitiveIntoMesh(std::shared_ptr<Mesh>& mesh);

	Concurrency::task<void> CreatePrimitiveMeshTemplate(std::wstring meshName, LoadMeshCallback loadFn = nullptr);
	void ReleaseMeshTemplates();
	std::shared_ptr<Mesh> GetMeshTemplate(std::wstring meshName);
	std::vector<std::wstring> GetMeshesNames();
};
typedef Templates::Mesh::Mesh MeshT;
typedef std::shared_ptr<MeshT> MeshPtr;
