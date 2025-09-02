#pragma once

#include "OverridePass.h"

struct MinMaxChainResultPass : public OverridePass
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE depthGpuHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle1;
	CD3DX12_GPU_DESCRIPTOR_HANDLE	shadowMapChainGpuHandle2;

	MinMaxChainResultPass(std::shared_ptr<Camera> cam, unsigned int rpI, std::shared_ptr<RenderPassInstance> rp);
	virtual ~MinMaxChainResultPass() {};
	void CreateFSQuad(std::string material);
	virtual void Pass();
	void Render();
};


