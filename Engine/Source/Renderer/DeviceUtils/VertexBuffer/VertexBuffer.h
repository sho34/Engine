#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils::VertexBuffer
{
	//vertex buffer
	struct VertexBufferViewData {
		ComPtr<ID3D12Resource>    vertexBuffer;
		ComPtr<ID3D12Resource>    vertexBufferUpload;
		D3D12_VERTEX_BUFFER_VIEW  vertexBufferView;
	};

	void InitializeVertexBufferView(ComPtr<ID3D12Device2>& d3dDevice, ComPtr<ID3D12GraphicsCommandList2>& commandList, const void* vertices, UINT vertexSize, UINT verticesCount, VertexBufferViewData& vbvData);

};

typedef DeviceUtils::VertexBuffer::VertexBufferViewData VertexBufferViewDataT;