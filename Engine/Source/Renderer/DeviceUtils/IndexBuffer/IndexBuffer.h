#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils::IndexBuffer
{
	//index buffer
	struct IndexBufferViewData {
		ComPtr<ID3D12Resource>    indexBuffer;
		ComPtr<ID3D12Resource>    indexBufferUpload;
		D3D12_INDEX_BUFFER_VIEW   indexBufferView;
	};

	void InitializeIndexBufferView(ComPtr<ID3D12Device2>& d3dDevice, ComPtr<ID3D12GraphicsCommandList2>& commandList, const void* indices, UINT indicesCount, IndexBufferViewData& ibvData);
};

typedef DeviceUtils::IndexBuffer::IndexBufferViewData IndexBufferViewDataT;