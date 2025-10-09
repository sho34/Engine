#pragma once

#include "OverridePass.h"

namespace Scene
{
	struct Camera;
};

struct MinMaxChainPass : public OverridePass
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapChainGpuHandle1;
	CD3DX12_GPU_DESCRIPTOR_HANDLE	shadowMapChainGpuHandle2;

	MinMaxChainPass(std::shared_ptr<Scene::Camera> cam, unsigned int rpI, std::shared_ptr<RenderPassInstance> rp);
	virtual ~MinMaxChainPass() {};
	virtual void Pass();
	void Render();
};

