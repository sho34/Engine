#include "pch.h"
#include "VertexBuffer.h"
#include "../Resources/Resources.h"
#include "../../../Common/DirectXHelper.h"

namespace DeviceUtils::VertexBuffer
{

	void InitializeVertexBufferView(ComPtr<ID3D12Device2>& d3dDevice, ComPtr<ID3D12GraphicsCommandList2>& commandList, const void* vertices, UINT vertexSize, UINT verticesCount, VertexBufferViewData& vbvData) {
		using namespace DeviceUtils::Resources;
		UpdateBufferResource(d3dDevice, commandList, vbvData.vertexBuffer, vbvData.vertexBufferUpload, verticesCount, vertexSize, vertices);
		vbvData.vertexBufferView.BufferLocation = vbvData.vertexBuffer->GetGPUVirtualAddress();
		vbvData.vertexBufferView.SizeInBytes = vertexSize * verticesCount;
		vbvData.vertexBufferView.StrideInBytes = vertexSize;
		NAME_D3D12_OBJECT(vbvData.vertexBuffer);
	}

}