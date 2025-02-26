#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace DeviceUtils
{
	//index buffer
	struct IndexBufferViewData
	{
		CComPtr<ID3D12Resource>    indexBuffer;
		CComPtr<ID3D12Resource>    indexBufferUpload;
		D3D12_INDEX_BUFFER_VIEW   indexBufferView;
	};

	void InitializeIndexBufferView(CComPtr<ID3D12Device2>& d3dDevice, CComPtr<ID3D12GraphicsCommandList2>& commandList, const void* indices, unsigned int indicesCount, IndexBufferViewData& ibvData);
};
