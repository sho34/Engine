#include "pch.h"
#include "ResolvePass.h"
#include <Renderer.h>
#include <Material/Material.h>
#include <Shader/Shader.h>
#include <Camera/Camera.h>
#include <Mesh/Mesh.h>
#include <RenderPass/RenderPass.h>

extern std::unique_ptr<Renderer> renderer;

ResolvePass::ResolvePass(JUUID cam, unsigned int rpI, JUUID rp) : OverridePass(cam, rpI, rp)
{
	using namespace Scene;

	auto& camSO = GetCameraSceneObject(cam);
	auto& prevPassJ = GetRenderPassTemplate(camSO->renderPasses().at(rpI - 1));
	mode = ResolveMode_CopyFromRenderToTexture;

	if (prevPassJ->renderTargetFormats().at(0) == DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		mode = ResolveMode_FullScreenQuad;
	}

	if (mode == ResolveMode_CopyFromRenderToTexture)
	{
		CreateFsQuadResources("FullScreenQuad", camSO->renderPasses().at(rpI), [this](std::string name, ShaderConstantsBufferVariable& var)
			{
				auto& fsCB = fsQuadConstantsBuffer;

				for (unsigned int n = 0; n < renderer->numFrames; n++)
				{
					if (name == "alpha")
					{
						float data = 1.0f;
						fsCB->push(data, n, var.offset);
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
		swapChain->CopyFromRenderToTexture(GetPrevPassRenderToTexture());
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
	auto& fsCB = fsQuadConstantsBuffer;
	auto& fsQuadMesh = GetMeshInstance(fsQuad);
	auto& prevPassRTT = GetRenderToTexture(GetPrevPassRenderToTexture());

#if defined(_DEVELOPMENT)
	PIXBeginEvent(commandList.p, 0, "ResolvePassQuad");
#endif

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);

	commandList->SetGraphicsRootDescriptorTable(0, fsCB->gpu_xhandle.at(renderer->backBufferIndex));
	commandList->SetGraphicsRootDescriptorTable(1, prevPassRTT->gpuTextureHandle);

	commandList->IASetVertexBuffers(0, 1, &fsQuadMesh->vbvData.vertexBufferView);
	commandList->IASetIndexBuffer(&fsQuadMesh->ibvData.indexBufferView);
	commandList->DrawIndexedInstanced(fsQuadMesh->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);

#if defined(_DEVELOPMENT)
	PIXEndEvent(commandList.p);
#endif
}
