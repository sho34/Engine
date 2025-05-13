#pragma once

#include "../../Renderer/VertexFormats.h"
#include "../../Renderer/DeviceUtils/VertexBuffer/VertexBuffer.h"
#include "../../Renderer/DeviceUtils/IndexBuffer/IndexBuffer.h"
#include "../../Renderer/DeviceUtils/RootSignature/RootSignature.h"
#include "../../Renderer/DeviceUtils/PipelineState/PipelineState.h"
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
		void CreateVerticesShaderResourceView(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
		void ExtendBoundingBox(BoundingBox& outBB, bool extend);
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

//MeshInstance to RootSignature
typedef std::map<std::shared_ptr<Templates::MeshInstance>, HashedRootSignature> MeshHashedRootSignatureMap;
//MeshInstance to PipelineState
typedef std::map<std::shared_ptr<Templates::MeshInstance>, HashedPipelineState> MeshHashedPipelineStateMap;