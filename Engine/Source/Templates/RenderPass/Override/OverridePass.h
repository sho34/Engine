#pragma once

struct OverridePass
{
	//data from camera and renderpass
	CameraUUID camera;
	unsigned int renderPassIndex;
	RenderPassInstanceUUID renderPassInstance;

	//fsQuad
	JUUID fsQuad;
	MaterialInstanceUUID fsQuadMaterial;
	ConstantsBufferUUID fsQuadConstantsBuffer;
	CComPtr<ID3D12RootSignature> rootSignature;
	CComPtr<ID3D12PipelineState> pipelineState;

	OverridePass() { assert(!!!"do not use"); }
	explicit OverridePass(JUUID cam, unsigned int rpI, JUUID rp) { camera = cam; renderPassIndex = rpI; renderPassInstance = rp; };
	virtual ~OverridePass();
	virtual void Initialize() {};
	void CreateFsQuadResources(std::string materialName, JUUID renderPassJson, std::function<void(std::string, ShaderConstantsBufferVariable&)> constantsBufferPusher = [](auto a, auto b) {});
	JUUID GetPrevPassRenderToTexture(unsigned int index = 0U);
	virtual void Pass() = 0;
};