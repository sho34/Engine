#include "pch.h"
#include "MinMaxChainPass.h"
#include <Renderer.h>
#include <Material/Material.h>
#include <Shader/Shader.h>

extern std::shared_ptr<Renderer> renderer;

MinMaxChainPass::MinMaxChainPass(std::shared_ptr<Camera> cam, unsigned int rpI, std::shared_ptr<RenderPassInstance> rp) : OverridePass(cam, rpI, rp)
{
	CreateFsQuadResources("DepthMinMax", GetRenderPassTemplate(rp->renderPassUUID), [this, rp](std::string name, ShaderConstantsBufferVariable& var)
		{
			XMFLOAT2 texelInvSize = {
				1.0f / rp->rendererToTexturePass->screenViewport.Width,
				1.0f / rp->rendererToTexturePass->screenViewport.Height
			};

			for (unsigned int n = 0; n < renderer->numFrames; n++)
			{
				if (name == "texelInvSize")
				{
					fsQuadConstantsBuffer->push(texelInvSize, n, var.offset);
				}
			}
		}
	);
}

void MinMaxChainPass::Pass()
{
	renderPassInstance->rendererToTexturePass->BeginRenderPass();
	Render();
	renderPassInstance->rendererToTexturePass->EndRenderPass();
}

void MinMaxChainPass::Render()
{
	auto& commandList = renderer->commandList;
#if defined(_DEVELOPMENT)
	PIXBeginEvent(commandList.p, 0, "MinMaxChainPassQuad");
#endif

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);

	commandList->SetGraphicsRootDescriptorTable(0, fsQuadConstantsBuffer->gpu_xhandle.at(renderer->backBufferIndex));
	commandList->SetGraphicsRootDescriptorTable(1, shadowMapChainGpuHandle1);
	commandList->SetGraphicsRootDescriptorTable(2, shadowMapChainGpuHandle2);

	commandList->IASetVertexBuffers(0, 1, &fsQuad->vbvData.vertexBufferView);
	commandList->IASetIndexBuffer(&fsQuad->ibvData.indexBufferView);
	commandList->DrawIndexedInstanced(fsQuad->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);

#if defined(_DEVELOPMENT)
	PIXEndEvent(commandList.p);
#endif
}
