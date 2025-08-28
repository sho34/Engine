#include "pch.h"
#include "MinMaxChainResultPass.h"
#include <Renderer.h>
#include <Material/Material.h>
#include <Shader/Shader.h>

extern std::shared_ptr<Renderer> renderer;

MinMaxChainResultPass::MinMaxChainResultPass(std::shared_ptr<Camera> cam, unsigned int rpI, std::shared_ptr<RenderPassInstance> rp) : OverridePass(cam, rpI, rp)
{
}

void MinMaxChainResultPass::CreateFSQuad(std::string materialName)
{
	CreateFsQuadResources(materialName, GetRenderPassTemplate(renderPassInstance->renderPassUUID));
}

void MinMaxChainResultPass::Pass()
{
	renderPassInstance->rendererToTexturePass->BeginRenderPass();
	Render();
	renderPassInstance->rendererToTexturePass->EndRenderPass();
}

void MinMaxChainResultPass::Render()
{
	auto& commandList = renderer->commandList;
#if defined(_DEVELOPMENT)
	PIXBeginEvent(commandList.p, 0, "MinMaxChainResultPassQuad");
#endif

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);

	commandList->SetGraphicsRootDescriptorTable(2, depthGpuHandle);
	commandList->SetGraphicsRootDescriptorTable(0, shadowMapChainGpuHandle1);
	commandList->SetGraphicsRootDescriptorTable(1, shadowMapChainGpuHandle2);


	commandList->IASetVertexBuffers(0, 1, &fsQuad->vbvData.vertexBufferView);
	commandList->IASetIndexBuffer(&fsQuad->ibvData.indexBufferView);
	commandList->DrawIndexedInstanced(fsQuad->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);

#if defined(_DEVELOPMENT)
	PIXEndEvent(commandList.p);
#endif
}
