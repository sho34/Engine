#pragma once

#include "../Renderer/Renderer.h"
#include "../Renderer/VertexFormats.h"
#include "../Renderer/DeviceUtils/VertexBuffer/VertexBuffer.h"
#include "../Renderer/DeviceUtils/IndexBuffer/IndexBuffer.h"

namespace Templates::Mesh {
	
	struct Mesh
	{
		VertexClass vertexClass;
		VertexBufferViewDataT vbvData;
		IndexBufferViewDataT	ibvData;
		bool loading = false;
	};

	typedef void LoadMeshCallback(std::shared_ptr<Mesh>& mesh);
	typedef Concurrency::task<void> LoadPrimitiveIntoMesh(std::shared_ptr<Renderer>& renderer, std::shared_ptr<Mesh>& mesh);

	Concurrency::task<void> CreateMeshTemplate(std::wstring meshName, std::shared_ptr<Renderer>& renderer, LoadPrimitiveIntoMesh meshLoader, LoadMeshCallback loadFn = nullptr);
	std::shared_ptr<Mesh> GetMeshTemplate(std::wstring meshName);
};
typedef Templates::Mesh::Mesh MeshT;
typedef std::shared_ptr<MeshT> MeshPtr;
