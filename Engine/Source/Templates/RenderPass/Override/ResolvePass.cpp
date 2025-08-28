#include "pch.h"
#include "ResolvePass.h"
#include <Renderer.h>
#include <Material/Material.h>
#include <Shader/Shader.h>

extern std::shared_ptr<Renderer> renderer;

ResolvePass::ResolvePass(std::shared_ptr<Camera> cam, unsigned int rpI, std::shared_ptr<RenderPassInstance> rp) : OverridePass(cam, rpI, rp)
{
	std::shared_ptr<RenderPassJson> prevPassJ = GetRenderPassTemplate(cam->renderPasses().at(rpI - 1));
	mode = ResolveMode_CopyFromRenderToTexture;
	if (prevPassJ->renderTargetFormats().at(0) == DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		mode = ResolveMode_FullScreenQuad;
		prevPassRTT = GetPrevPassRenderToTexture();
	}

	if (mode == ResolveMode_CopyFromRenderToTexture)
	{
		CreateFsQuadResources("FullScreenQuad", GetRenderPassTemplate(cam->renderPasses().at(rpI)), [this](std::string name, ShaderConstantsBufferVariable& var)
			{
				for (unsigned int n = 0; n < renderer->numFrames; n++)
				{
					if (name == "alpha")
					{
						float data = 1.0f;
						fsQuadConstantsBuffer->push(data, n, var.offset);
					}
				}
			}
		);
	}
}

void ResolvePass::Pass()
{
	auto& swapChain = renderPassInstance->swapChainPass;
	swapChain->BeginRenderPass(swapChain->depthStencilViewDescriptorHeap);
	if (mode == ResolveMode_FullScreenQuad)
	{
		swapChain->CopyFromRenderToTexture(prevPassRTT);
	}
	else
	{
		Render();
	}
	swapChain->EndRenderPass();
}

void ResolvePass::Render()
{
	auto& commandList = renderer->commandList;
#if defined(_DEVELOPMENT)
	PIXBeginEvent(commandList.p, 0, "ResolvePassQuad");
#endif

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);

	commandList->SetGraphicsRootDescriptorTable(0, fsQuadConstantsBuffer->gpu_xhandle.at(renderer->backBufferIndex));
	commandList->SetGraphicsRootDescriptorTable(1, prevPassRTT->gpuTextureHandle);

	commandList->IASetVertexBuffers(0, 1, &fsQuad->vbvData.vertexBufferView);
	commandList->IASetIndexBuffer(&fsQuad->ibvData.indexBufferView);
	commandList->DrawIndexedInstanced(fsQuad->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);

#if defined(_DEVELOPMENT)
	PIXEndEvent(commandList.p);
#endif
}
