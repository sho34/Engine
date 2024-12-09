#include "pch.h"
#include "IndexBuffer.h"
#include "../Resources/Resources.h"
#include "../../../Common/DirectXHelper.h"

namespace DeviceUtils::IndexBuffer
{

	void InitializeIndexBufferView(CComPtr<ID3D12Device2>& d3dDevice, CComPtr<ID3D12GraphicsCommandList2>& commandList, const void* indices, UINT indicesCount, IndexBufferViewData& ibvData) {
		using namespace DeviceUtils::Resources;
		UpdateBufferResource(d3dDevice, commandList, ibvData.indexBuffer, ibvData.indexBufferUpload, indicesCount, sizeof(UINT32), indices);
		ibvData.indexBufferView.BufferLocation = ibvData.indexBuffer->GetGPUVirtualAddress();
		ibvData.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		ibvData.indexBufferView.SizeInBytes = sizeof(UINT32) * indicesCount;
	}

};