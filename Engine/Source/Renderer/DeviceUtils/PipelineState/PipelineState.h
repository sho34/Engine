#pragma once

#include <wrl.h>
#include "../../../d3dx12.h"
#include "../../../Templates/Mesh.h"
#include "../../../Templates/Material.h"

namespace DeviceUtils::PipelineState
{

	ComPtr<ID3D12PipelineState> CreatePipelineState(ComPtr<ID3D12Device2> d3dDevice, VertexClass vertexClass, const MaterialPtr& material, ComPtr<ID3D12RootSignature>& rootSignature);

	ComPtr<ID3D12PipelineState> CreateShadowMapPipelineState(ComPtr<ID3D12Device2> d3dDevice, VertexClass vertexClass, const MaterialPtr& material, ComPtr<ID3D12RootSignature>& rootSignature);
};
