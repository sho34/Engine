#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils
{
	//vertex buffer
	struct VertexBufferViewData
	{
		CComPtr<ID3D12Resource>    vertexBuffer;
		CComPtr<ID3D12Resource>    vertexBufferUpload;
		D3D12_VERTEX_BUFFER_VIEW  vertexBufferView;
	};

	void InitializeVertexBufferView(CComPtr<ID3D12Device2> d3dDevice, CComPtr<ID3D12GraphicsCommandList2> commandList, const void* vertices, unsigned int vertexSize, unsigned int verticesCount, VertexBufferViewData& vbvData);
};
