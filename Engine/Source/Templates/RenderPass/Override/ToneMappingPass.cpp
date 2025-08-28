#include "pch.h"
#include "ToneMappingPass.h"
#include <Renderer.h>
#include <StepTimer.h>
#include <DeviceUtils/Resources/Resources.h>

extern std::shared_ptr<Renderer> renderer;

ToneMappingPass::ToneMappingPass(std::shared_ptr<Camera> cam, unsigned int rpI, std::shared_ptr<RenderPassInstance> rp) : OverridePass(cam, rpI, rp)
{
	auto prevPassRTT = GetPrevPassRenderToTexture();

	hdrHistogram = std::make_shared<LuminanceHistogram>(prevPassRTT);
	hdrHistogram->UpdateLuminanceHistogramParams(prevPassRTT->width, prevPassRTT->height, cam->minLogLuminance(), cam->maxLogLuminance());

	luminanceHistogramAverage = std::make_shared<LuminanceHistogramAverage>(hdrHistogram->resultCpuHandle, hdrHistogram->resultGpuHandle);
	luminanceHistogramAverage->UpdateLuminanceHistogramAverageParams(prevPassRTT->width * prevPassRTT->height,
		cam->minLogLuminance(), cam->maxLogLuminance(), 0.016f, cam->tau()
	);

	CreateFsQuadResources("ToneMap", GetRenderPassTemplate(cam->renderPasses().at(rpI)));
}

ToneMappingPass::~ToneMappingPass()
{
	hdrHistogram = nullptr;
	luminanceHistogramAverage = nullptr;
}

extern DX::StepTimer timer;
void ToneMappingPass::Pass()
{
	auto& cam = camera;

	//update the histogram and & luminance average parameters
	hdrHistogram->UpdateLuminanceHistogramParams(prevPassRTT->width, prevPassRTT->height, cam->minLogLuminance(), cam->maxLogLuminance());
	luminanceHistogramAverage->UpdateLuminanceHistogramAverageParams(prevPassRTT->width * prevPassRTT->height,
		cam->minLogLuminance(), cam->maxLogLuminance(), static_cast<float>(timer.GetElapsedSeconds()), cam->tau());

	//run the compute shaders for hdr histogram & luminance average calculation
	hdrHistogram->Compute();
	luminanceHistogramAverage->Compute();

	//render the scene using the applied luminane correction
	auto& rttPass = renderPassInstance->rendererToTexturePass;
	rttPass->BeginRenderPass();
	Render();
	rttPass->EndRenderPass();
}

void ToneMappingPass::Render()
{
	auto& commandList = renderer->commandList;
#if defined(_DEVELOPMENT)
	PIXBeginEvent(commandList.p, 0, "ResolvePassQuad");
#endif

	DeviceUtils::UAVResource(commandList, luminanceHistogramAverage->average);
	DeviceUtils::TransitionResource(commandList, luminanceHistogramAverage->average,
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);

	//commandList->SetGraphicsRootDescriptorTable(0, fsQuadConstantsBuffer->gpu_xhandle.at(renderer->backBufferIndex));
	unsigned int camSlot = 0U;
	camera->cameraCbv->SetRootDescriptorTable(commandList, camSlot, renderer->backBufferIndex);
	commandList->SetGraphicsRootDescriptorTable(1, prevPassRTT->gpuTextureHandle);
	commandList->SetGraphicsRootDescriptorTable(2, luminanceHistogramAverage->averageReadGpuHandle);

	commandList->IASetVertexBuffers(0, 1, &fsQuad->vbvData.vertexBufferView);
	commandList->IASetIndexBuffer(&fsQuad->ibvData.indexBufferView);
	commandList->DrawIndexedInstanced(fsQuad->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);

	DeviceUtils::TransitionResource(commandList, luminanceHistogramAverage->average,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON
	);

#if defined(_DEVELOPMENT)
	PIXEndEvent(commandList.p);
#endif
}
