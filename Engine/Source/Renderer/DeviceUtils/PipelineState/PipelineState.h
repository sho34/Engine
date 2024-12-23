#pragma once

#include "../../../d3dx12.h"
#include "../../../Templates/Mesh/MeshImpl.h"
#include "../../../Templates/Material/MaterialImpl.h"

namespace DeviceUtils::PipelineState
{
	CComPtr<ID3D12PipelineState> CreatePipelineState(CComPtr<ID3D12Device2>& d3dDevice, VertexClass vertexClass, const MaterialPtr& material, CComPtr<ID3D12RootSignature>& rootSignature);
	CComPtr<ID3D12PipelineState> CreateShadowMapPipelineState(CComPtr<ID3D12Device2>& d3dDevice, VertexClass vertexClass, const MaterialPtr& material, CComPtr<ID3D12RootSignature>& rootSignature);
};
