#include "pch.h"
#include "OverridePass.h"
#include <Renderer.h>
#include <Material/Material.h>
#include <Shader/Shader.h>

extern std::shared_ptr<Renderer> renderer;

OverridePass::~OverridePass()
{
	DestroyMeshInstance(fsQuad);
	DestroyMaterialInstance(fsQuadMaterial);
	DestroyConstantsBuffer(fsQuadConstantsBuffer);
	camera = nullptr;
	renderPassInstance = nullptr;
	rootSignature = nullptr;
	pipelineState = nullptr;
	prevPassRTT = nullptr;
}

void OverridePass::CreateFsQuadResources(std::string materialName, std::shared_ptr<RenderPassJson> renderPass, std::function<void(std::string, ShaderConstantsBufferVariable&)> constantsBufferPusher)
{
	fsQuad = GetMeshInstance(FindMeshUUIDByName("decal"));
	std::string fsQuadMaterialUUID = FindMaterialUUIDByName(materialName);
	VertexClass vertexClass = fsQuad->vertexClass;
	fsQuadMaterial = GetMaterialInstance(fsQuadMaterialUUID, [fsQuadMaterialUUID, vertexClass]()
		{
			return std::make_shared<MaterialInstance>(fsQuadMaterialUUID, fsQuadMaterialUUID, vertexClass, false);
		}
	);

	if (fsQuadMaterial->variablesBufferSize.size() > 0ULL)
	{
		size_t size = fsQuadMaterial->variablesBufferSize.at(0);
		fsQuadConstantsBuffer = CreateConstantsBuffer(size, materialName + ":cbv");

		auto& vsVars = fsQuadMaterial->vertexShader->constantsBuffersVariables;
		auto& psVars = fsQuadMaterial->pixelShader->constantsBuffersVariables;

		for (auto& [name, var] : vsVars) { constantsBufferPusher(name, var); }
		for (auto& [name, var] : psVars) { constantsBufferPusher(name, var); }
	}

	auto& mi = fsQuadMaterial;

	auto& vsCBparams = mi->vertexShader->constantsBuffersParameters;
	auto& psCBparams = mi->pixelShader->constantsBuffersParameters;
	auto& uavParams = mi->pixelShader->uavParameters;
	auto& psSRVCSparams = mi->pixelShader->srvCSParameters;
	auto& psSRVTexparams = mi->pixelShader->srvTexParameters;
	auto& psSamplersParams = mi->pixelShader->samplersParameters;
	auto& samplers = mi->samplers;

	std::string rsName = "rootSignature:" + materialName;
	rootSignature = CreateRootSignature(rsName, vsCBparams, psCBparams, uavParams, psSRVCSparams, psSRVTexparams, psSamplersParams, samplers);

	auto& vsLayout = vertexInputLayoutsMap[vertexClass];
	auto& vsByteCode = mi->vertexShader->byteCode;
	auto& psByteCode = mi->pixelShader->byteCode;

	std::shared_ptr<MaterialJson> material = GetMaterialTemplate(mi->materialUUID);
	BlendDesc blendDesc = material->blendState();
	RasterizerDesc rasterizerDesc = material->rasterizerState();

	D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	std::string plName = "pipelineState:" + materialName;
	auto rtf = renderPass->renderTargetFormats();
	auto df = renderPass->depthStencilFormat();
	pipelineState = CreateGraphicsPipelineState(plName, vsLayout, vsByteCode, psByteCode, rootSignature, blendDesc, rasterizerDesc, primitiveTopologyType, rtf, df);

	if (camera)
		prevPassRTT = GetPrevPassRenderToTexture();
}

std::shared_ptr<RenderToTexture> OverridePass::GetPrevPassRenderToTexture(unsigned int index)
{
	std::shared_ptr<RenderPassInstance> prevPass = camera->cameraRenderPasses.at(renderPassIndex - 1);
	return prevPass->rendererToTexturePass->renderToTexture.at(index);
}