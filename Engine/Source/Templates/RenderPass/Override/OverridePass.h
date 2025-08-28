#pragma once

#include <Camera/Camera.h>
#include <RenderPass/RenderPass.h>

using namespace Scene;
using namespace Templates;

struct OverridePass
{
	//data from camera and renderpass
	std::shared_ptr<Camera> camera;
	unsigned int renderPassIndex;
	std::shared_ptr<RenderPassInstance> renderPassInstance;

	//fsQuad
	std::shared_ptr<MeshInstance> fsQuad;
	std::shared_ptr<MaterialInstance> fsQuadMaterial;
	std::shared_ptr<ConstantsBuffer> fsQuadConstantsBuffer;
	CComPtr<ID3D12RootSignature> rootSignature;
	CComPtr<ID3D12PipelineState> pipelineState;
	std::shared_ptr<RenderToTexture> prevPassRTT;

	OverridePass(std::shared_ptr<Camera> cam, unsigned int rpI, std::shared_ptr<RenderPassInstance> rp) { camera = cam; renderPassIndex = rpI; renderPassInstance = rp; };
	virtual ~OverridePass();
	void CreateFsQuadResources(std::string materialName, std::shared_ptr<RenderPassJson> renderPass, std::function<void(std::string, ShaderConstantsBufferVariable&)> constantsBufferPusher = [](auto a, auto b) {});
	std::shared_ptr<RenderToTexture> GetPrevPassRenderToTexture(unsigned int index = 0U);
	virtual void Pass() = 0;
};