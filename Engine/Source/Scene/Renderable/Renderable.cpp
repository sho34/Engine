#include "pch.h"
#include <ppltasks.h>
#include <vector>
#include <nlohmann/json.hpp>
#include "Renderable.h"
#include "../Scene.h"
#include "../Camera/Camera.h"
#include "../Lights/Lights.h"
#include "../../Common/d3dx12.h"
#include "../../Common/DirectXHelper.h"
#include "../../Templates/Model3D/Model3D.h"
#include "../../Templates/Mesh/Mesh.h"
#include "../../Templates/Material/Material.h"
#include "../../Animation/Animated.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/DeviceUtils/D3D12Device/Builder.h"
#include "../../Renderer/DeviceUtils/D3D12Device/Interop.h"
#include "../../Renderer/DeviceUtils/RootSignature/RootSignature.h"
#include "../../Renderer/DeviceUtils/PipelineState/PipelineState.h"
#if defined(_EDITOR)
#include "../Level.h"
#include "../../Effects/Effects.h"
#include <imgui.h>
#endif
#include "../../Templates/Templates.h"
#include <Application.h>
#include <NoStd.h>
#include <Editor.h>

extern std::mutex rendererMutex;
extern std::shared_ptr<Renderer> renderer;
#if defined(_EDITOR)
namespace Editor {
	extern _Templates tempTab;
	extern std::string selTemp;
}
#endif

namespace Scene {
	//NAMESPACE VARIABLES
	std::map<std::string, std::shared_ptr<Renderable>> renderables;
	std::map<std::string, std::shared_ptr<Renderable>> animables;

	void Renderable::TransformJsonToMeshMaterialMap(MeshMaterialMap& map, nlohmann::json j, nlohmann::json shaderAttributes)
	{
		std::transform(j.begin(), j.end(), std::inserter(map, map.end()), [shaderAttributes](const nlohmann::json& value)
			{
				std::map<TextureType, MaterialTexture> textures;
				TransformJsonToMaterialTextures(textures, value, "textures");

				std::shared_ptr<MeshInstance> mesh = GetMeshInstance(value["mesh"]);
				std::shared_ptr<MaterialInstance> material = GetMaterialInstance(value["material"], textures, mesh, shaderAttributes);
				return MeshMaterialPair(mesh, material);
			}
		);
	}

	void Renderable::TransformJsonToRenderTargetBlendDesc(D3D12_RENDER_TARGET_BLEND_DESC& RenderTarget, nlohmann::json j)
	{
		ReplaceFromJson(RenderTarget.BlendEnable, j, "BlendEnable");
		ReplaceFromJson(RenderTarget.LogicOpEnable, j, "LogicOpEnable");
		nostd::ReplaceFromJsonUsingMap(RenderTarget.SrcBlend, stringToBlend, j, "SrcBlend");
		nostd::ReplaceFromJsonUsingMap(RenderTarget.DestBlend, stringToBlend, j, "DestBlend");
		nostd::ReplaceFromJsonUsingMap(RenderTarget.BlendOp, stringToBlendOp, j, "BlendOp");
		nostd::ReplaceFromJsonUsingMap(RenderTarget.SrcBlendAlpha, stringToBlend, j, "SrcBlendAlpha");
		nostd::ReplaceFromJsonUsingMap(RenderTarget.DestBlendAlpha, stringToBlend, j, "DestBlendAlpha");
		nostd::ReplaceFromJsonUsingMap(RenderTarget.BlendOpAlpha, stringToBlendOp, j, "BlendOpAlpha");
		nostd::ReplaceFromJsonUsingMap(RenderTarget.BlendOpAlpha, stringToBlendOp, j, "BlendOpAlpha");
		nostd::ReplaceFromJsonUsingMap(RenderTarget.LogicOp, stringToLogicOp, j, "LogicOp");
		ReplaceFromJson(RenderTarget.RenderTargetWriteMask, j, "RenderTargetWriteMask");
	}

	void Renderable::TransformJsonToBlendState(D3D12_BLEND_DESC& BlendState, nlohmann::json j, std::string key)
	{
		if (!j.contains(key) || j[key].empty()) return;
		nlohmann::json blstate = j[key];

		ReplaceFromJson(BlendState.AlphaToCoverageEnable, blstate, "AlphaToCoverageEnable");
		ReplaceFromJson(BlendState.IndependentBlendEnable, blstate, "IndependentBlendEnable");
		if (blstate.contains("RenderTarget") && !blstate["RenderTarget"].empty())
		{
			for (unsigned int i = 0U; i < blstate.count("RenderTarget"); i++)
			{
				TransformJsonToRenderTargetBlendDesc(BlendState.RenderTarget[i], blstate["RenderTarget"][i]);
			}
		}
	}

	void Renderable::TransformJsonToRasterizerState(D3D12_RASTERIZER_DESC& RasterizerState, nlohmann::json j, std::string key)
	{
		if (!j.contains(key) || j[key].empty()) return;
		nlohmann::json rzstate = j[key];

		nostd::ReplaceFromJsonUsingMap(RasterizerState.FillMode, stringToFillMode, rzstate, "FillMode");
		nostd::ReplaceFromJsonUsingMap(RasterizerState.CullMode, stringToCullMode, rzstate, "CullMode");
		ReplaceFromJson(RasterizerState.FrontCounterClockwise, rzstate, "FrontCounterClockwise");
		ReplaceFromJson(RasterizerState.DepthBias, rzstate, "DepthBias");
		ReplaceFromJson(RasterizerState.DepthBiasClamp, rzstate, "DepthBiasClamp");
		ReplaceFromJson(RasterizerState.SlopeScaledDepthBias, rzstate, "SlopeScaledDepthBias");
		ReplaceFromJson(RasterizerState.DepthClipEnable, rzstate, "DepthClipEnable");
		ReplaceFromJson(RasterizerState.MultisampleEnable, rzstate, "MultisampleEnable");
		ReplaceFromJson(RasterizerState.AntialiasedLineEnable, rzstate, "AntialiasedLineEnable");
		ReplaceFromJson(RasterizerState.ForcedSampleCount, rzstate, "ForcedSampleCount");
		nostd::ReplaceFromJsonUsingMap(RasterizerState.ConservativeRaster, stringToConservativeRasterizationMode, rzstate, "ConservativeRaster");
	}

	void Renderable::TransformJsonToPipelineState(RenderablePipelineState& pipelineState, nlohmann::json j, std::string key)
	{
		if (!j.contains(key) || j[key].empty()) return;
		nlohmann::json pstate = j[key];

		nostd::InsertFromJsonUsingMap(pipelineState.renderTargetsFormats, stringToDxgiFormat, pstate, "renderTargetsFormats");
		nostd::ReplaceFromJsonUsingMap(pipelineState.depthStencilFormat, stringToDxgiFormat, pstate, "depthStencilFormat");
		nostd::ReplaceFromJsonUsingMap(pipelineState.PrimitiveTopologyType, stringToPrimitiveTopologyType, pstate, "PrimitiveTopologyType");
		TransformJsonToRasterizerState(pipelineState.RasterizerState, pstate, "RasterizerState");
		TransformJsonToBlendState(pipelineState.BlendState, pstate, "BlendState");
	}

	void Renderable::BuildPipelineStateFromJsonChain(RenderablePipelineState& pipelineState, std::vector<nlohmann::json> jsons)
	{
		nlohmann::json pipelineStateJson = { {"pipelineState", {}} };

		for (auto& pj : jsons)
		{
			if (pj.contains("pipelineState") && !pj["pipelineState"].empty())
			{
				if (pj["pipelineState"].contains("renderTargetsFormats") && !pj["pipelineState"]["renderTargetsFormats"].empty())
				{
					pipelineState.renderTargetsFormats.clear();
				}
				pipelineStateJson["pipelineState"].merge_patch(pj["pipelineState"]);
			}
		}

		TransformJsonToPipelineState(pipelineState, pipelineStateJson, "pipelineState");
	}

	//CREATE
	std::shared_ptr<Renderable> CreateRenderable(nlohmann::json renderablej)
	{
		using namespace Animation;
		using namespace Templates;

		assert((renderablej["meshMaterials"].empty() and renderablej.count("meshMaterials") > 0) xor renderablej["model"].empty());
		assert(renderablej["name"] != "" && renderablej["uuid"] != "");

		std::shared_ptr<Renderable> renderable = std::make_shared<Renderable>();

		renderable->json = renderablej;
		if (!renderable->json.contains("pipelineState")) renderable->json["pipelineState"] = nlohmann::json::object();
		renderable->this_ptr = renderable;

		SetIfMissingJson(renderable->json, "visible", true);
		SetIfMissingJson(renderable->json, "hidden", false);
		SetIfMissingJson(renderable->json, "uniqueMaterialInstance", false);
		SetIfMissingJson(renderable->json, "castShadows", true);
		SetIfMissingJson(renderable->json, "position", XMFLOAT3({ 0.0f,0.0f,0.0f }));
		SetIfMissingJson(renderable->json, "rotation", XMFLOAT3({ 0.0f,0.0f,0.0f }));
		SetIfMissingJson(renderable->json, "scale", XMFLOAT3({ 1.0f,1.0f,1.0f }));
		SetIfMissingJson(renderable->json, "topology", primitiveTopologyToString.at(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		TransformJsonArrayToSet(renderable->skipMeshes, renderablej, "skipMeshes");

		std::string model3DName = "";
		if (!renderablej["meshMaterials"].empty())
		{
			renderable->TransformJsonToMeshMaterialMap(renderable->meshMaterials, renderablej["meshMaterials"], renderable->json);
			renderable->meshes.push_back(renderable->meshMaterials.begin()->first);

			renderable->TransformJsonToMeshMaterialMap(renderable->meshShadowMapMaterials, renderablej["meshMaterialsShadowMap"], defaultShadowMapShaderAttributes);
			if (renderable->meshShadowMapMaterials.size() > 0)
			{
				renderable->meshesShadowMap.push_back(renderable->meshShadowMapMaterials.begin()->first);
			}
		}
		else
		{
			renderable->CreateFromModel3D(renderablej["model"]);
		}

		renderable->CreateMeshesComponents();

		renderables.insert_or_assign(renderable->uuid(), renderable);

		return renderable;
	}

	void Renderable::CreateFromModel3D(std::string model3DUUID)
	{
		std::shared_ptr<Model3DInstance> model = GetModel3DInstance(model3DUUID, json);

		model3D = model;

		for (unsigned int i = 0U; i < model->meshes.size(); i++)
		{
			meshes.push_back(model->meshes[i]);
			meshesShadowMap.push_back(model->meshes[i]);
			SetMeshMaterial(model->meshes[i], model->materials[i]);
		}

		if (model->animations)
		{
			animable = model;
			AttachAnimation(this_ptr, model->animations);
			animables.insert_or_assign(uuid(), this_ptr);
			StepAnimation(0.0f);
		}

#if defined(_EDITOR)
		BindNotifications(model3DUUID, this_ptr);
#endif
	}

	std::string Renderable::uuid()
	{
		return json.at("uuid");
	}

	void Renderable::uuid(std::string uuid)
	{
		json.at("uuid") = uuid;
	}

	std::string Renderable::name()
	{
		return json.at("name");
	}

	void Renderable::name(std::string name)
	{
		json.at("name") = name;
	}

	XMFLOAT3 Renderable::position()
	{
		return XMFLOAT3(json.at("position").at(0), json.at("position").at(1), json.at("position").at(2));
	}

	void Renderable::position(XMFLOAT3 f3)
	{
		nlohmann::json& j = json.at("position");
		j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
	}

	void Renderable::position(nlohmann::json f3)
	{
		json.at("position") = f3;
	}

	XMFLOAT3 Renderable::rotation()
	{
		return XMFLOAT3(json.at("rotation").at(0), json.at("rotation").at(1), json.at("rotation").at(2));
	}

	void Renderable::rotation(XMFLOAT3 f3)
	{
		nlohmann::json& j = json.at("rotation");
		j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
	}

	void Renderable::rotation(nlohmann::json f3)
	{
		json.at("rotation") = f3;
	}

	XMFLOAT3 Renderable::scale()
	{
		return XMFLOAT3(json.at("scale").at(0), json.at("scale").at(1), json.at("scale").at(2));
	}

	void Renderable::scale(XMFLOAT3 f3)
	{
		nlohmann::json& j = json.at("scale");
		j.at(0) = f3.x; j.at(1) = f3.y; j.at(2) = f3.z;
	}

	void Renderable::scale(nlohmann::json f3)
	{
		json.at("scale") = f3;
	}

	bool Renderable::visible()
	{
		return json.at("visible");
	}

	void Renderable::visible(bool visible)
	{
		json.at("visible") = visible;
	}

	bool Renderable::hidden()
	{
		return json.at("hidden");
	}

	void Renderable::hidden(bool hidden)
	{
		json.at("hidden") = hidden;
	}

	D3D_PRIMITIVE_TOPOLOGY Renderable::topology()
	{
		return stringToPrimitiveTopology.at(json.at("topology"));
	}

	void Renderable::topology(std::string topology)
	{
		json.at("topology") = topology;
	}

	bool Renderable::uniqueMaterialInstance()
	{
		return json.at("uniqueMaterialInstance");
	}

	void Renderable::uniqueMaterialInstance(bool uniqueMaterialInstance)
	{
		json.at("uniqueMaterialInstance") = uniqueMaterialInstance;
	}

	bool Renderable::castShadows()
	{
		return json.at("castShadows");
	}

	void Renderable::castShadows(bool castShadows)
	{
		json.at("castShadows") = castShadows;
	}

	void Renderable::SetMeshMaterial(std::shared_ptr<MeshInstance> mesh, std::shared_ptr<MaterialInstance> material)
	{
		//if material is nullptr let's ensure this both meshMaterials and shadowMapMeshMaterials assign null
		if (!material)
		{
			meshMaterials.insert_or_assign(mesh, nullptr);
			meshShadowMapMaterials.insert_or_assign(mesh, nullptr);
		}

		//map the mesh to a material
		meshMaterials.insert_or_assign(mesh, material);

		//pick tuple textures if available
		std::map<TextureType, MaterialTexture> textures;
		if (material->tupleTextures.size() > 0U)
		{
			textures = material->tupleTextures;
		}
		else
		{
			//otherwise pick from the material template's textures
			nlohmann::json materialTemplate = GetMaterialTemplate(material->material);
			TransformJsonToMaterialTextures(textures, materialTemplate, "textures");
		}

		//truncate to first texture
		std::map<TextureType, MaterialTexture> shadowMapBaseTexture;
		if (textures.contains(TextureType_Base))
		{
			shadowMapBaseTexture.insert_or_assign(TextureType_Base, textures.at(TextureType_Base));
		}

		std::shared_ptr<MaterialInstance> shadowMapMaterial = GetMaterialInstance(FindMaterialUUIDByName("ShadowMap"), shadowMapBaseTexture, mesh, defaultShadowMapShaderAttributes);
		meshShadowMapMaterials.insert_or_assign(mesh, shadowMapMaterial);
	}

#if defined(_EDITOR)
	void Renderable::BindChangesToMaterial(unsigned int meshIndex)
	{
		std::string materialName = meshMaterials.at(meshes.at(meshIndex))->material;

		//rebuild material instance
		meshMaterials.at(meshes.at(meshIndex))->BindRebuildChange([this, meshIndex, materialName]
			{
				materialToChangeMeshIndex.push_back(meshIndex);
				materialToRebuild.push_back(materialName);
				renderableUpdateFlags |= RenderableFlags_RebuildMaterials;
			}
		);

		//rebuild shadow map material instance
		if (meshesShadowMap.size() > 0ULL)
		{
			meshShadowMapMaterials.at(meshesShadowMap.at(meshIndex))->BindRebuildChange([this, meshIndex, materialName]
				{
					materialToChangeMeshIndex.push_back(meshIndex);
					materialToRebuild.push_back(materialName);
					renderableUpdateFlags |= RenderableFlags_RebuildMaterials;
				}
			);
		}

		//update constants buffer values
		meshMaterials.at(meshes.at(meshIndex))->BindMappedValueChange([this, meshIndex]
			{
				std::shared_ptr<MeshInstance> mesh = meshes[meshIndex];
				std::shared_ptr<MaterialInstance>& material = meshMaterials.at(mesh);
				for (std::shared_ptr<ConstantsBuffer> cbvData : meshConstantsBuffer[mesh])
				{
					for (unsigned int n = 0; n < renderer->numFrames; n++)
					{
						WriteMaterialVariablesToConstantsBufferSpace(material, cbvData, n);
					}
				}
			}
		);

	}
#endif

	void Renderable::CreateMeshesComponents()
	{
#if defined(_EDITOR)
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			BindChangesToMaterial(i);
		}

#endif

		for (auto& mesh : meshes)
		{
			CreateMeshConstantsBuffers(mesh);
			CreateMeshRootSignatures(mesh);
			CreateMeshPipelineState(mesh);
			CreateMeshShadowMapConstantsBuffers(mesh);
			CreateMeshShadowMapRootSignatures(mesh);
			CreateMeshShadowMapPipelineState(mesh);
		}

		CreateBoundingBox();
	}

	void Renderable::CreateMeshConstantsBuffers(std::shared_ptr<MeshInstance> mesh)
	{
		if (meshConstantsBuffer.contains(mesh)) meshConstantsBuffer.at(mesh).clear();

		std::shared_ptr<MaterialInstance> material = meshMaterials.at(mesh);
		if (!material) return;

		for (size_t size : material->variablesBufferSize)
		{
			std::shared_ptr<ConstantsBuffer> cbvData = CreateConstantsBuffer(size, name() + "." + mesh->uuid);
			for (unsigned int n = 0; n < renderer->numFrames; n++)
			{
				WriteMaterialVariablesToConstantsBufferSpace(material, cbvData, n);
			}
			meshConstantsBuffer[mesh].push_back(cbvData);
		}
	}

	void Renderable::CreateMeshShadowMapConstantsBuffers(std::shared_ptr<MeshInstance> mesh)
	{
		if (!meshShadowMapMaterials.contains(mesh)) return;
		if (meshShadowMapConstantsBuffer.contains(mesh)) meshShadowMapConstantsBuffer.at(mesh).clear();

		std::shared_ptr<MaterialInstance> material = meshShadowMapMaterials.at(mesh);
		if (!material) return;

		for (size_t size : material->variablesBufferSize)
		{
			std::shared_ptr<ConstantsBuffer> cbvData = CreateConstantsBuffer(size, name() + ".sm." + mesh->uuid);
			for (unsigned int n = 0; n < renderer->numFrames; n++)
			{
				WriteMaterialVariablesToConstantsBufferSpace(material, cbvData, n);
			}
			meshShadowMapConstantsBuffer[mesh].push_back(cbvData);
		}
	}

	void Renderable::CreateMeshRootSignatures(std::shared_ptr<MeshInstance> mesh)
	{
		std::shared_ptr<MaterialInstance> material = meshMaterials.at(mesh);
		if (!material)
		{
			meshRootSignatures.insert_or_assign(mesh, nullptr);
			return;
		}

		auto& vsCBparams = material->vertexShader->constantsBuffersParameters;
		auto& psCBparams = material->pixelShader->constantsBuffersParameters;
		auto& psSRVparams = material->pixelShader->texturesParameters;
		auto& psSamplersParams = material->pixelShader->samplersParameters;
		auto& samplers = material->samplers;

		meshRootSignatures.insert_or_assign(mesh, CreateRootSignature(mesh->uuid, vsCBparams, psCBparams, psSRVparams, psSamplersParams, samplers));
	}

	void Renderable::CreateMeshShadowMapRootSignatures(std::shared_ptr<MeshInstance> mesh)
	{
		if (!meshShadowMapMaterials.contains(mesh)) return;
		std::shared_ptr<MaterialInstance> material = meshShadowMapMaterials.at(mesh);
		if (!material)
		{
			meshShadowMapRootSignatures.insert_or_assign(mesh, nullptr);
			return;
		}

		auto& vsCBparams = material->vertexShader->constantsBuffersParameters;
		auto& psCBparams = material->pixelShader->constantsBuffersParameters;
		auto& psSRVparams = material->pixelShader->texturesParameters;
		auto& psSamplersParams = material->pixelShader->samplersParameters;
		auto& samplers = material->samplers;

		meshShadowMapRootSignatures.insert_or_assign(mesh, CreateRootSignature(mesh->uuid, vsCBparams, psCBparams, psSRVparams, psSamplersParams, samplers));
	}

	void Renderable::CreateMeshPipelineState(std::shared_ptr<MeshInstance> mesh)
	{
		static std::map<D3D12_PRIMITIVE_TOPOLOGY_TYPE, D3D_PRIMITIVE_TOPOLOGY> topologyMap = {
			{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE , D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST },
			{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, D3D_PRIMITIVE_TOPOLOGY_LINELIST }
		};

		std::shared_ptr<MaterialInstance> material = meshMaterials.at(mesh);
		if (!material)
		{
			meshPipelineStates.insert_or_assign(mesh, nullptr);
			return;
		}

		auto& vsLayout = vertexInputLayoutsMap[mesh->vertexClass];
		auto& rootSignature = meshRootSignatures[mesh];
		auto& vsByteCode = material->vertexShader->byteCode;
		auto& psByteCode = material->pixelShader->byteCode;

		RenderablePipelineState pipelineState;
		BuildPipelineStateFromJsonChain(pipelineState, { GetMaterialTemplate(material->material), json });
		topology(primitiveTopologyToString.at(topologyMap.at(pipelineState.PrimitiveTopologyType)));
		meshPipelineStates.insert_or_assign(mesh, CreatePipelineState(mesh->uuid, vsLayout, vsByteCode, psByteCode, rootSignature, pipelineState));
	}

	void Renderable::CreateMeshShadowMapPipelineState(std::shared_ptr<MeshInstance> mesh)
	{
		if (!meshShadowMapMaterials.contains(mesh)) return;
		std::shared_ptr<MaterialInstance> material = meshShadowMapMaterials.at(mesh);
		if (!material)
		{
			meshShadowMapPipelineStates.insert_or_assign(mesh, nullptr);
			return;
		}

		auto& vsLayout = vertexInputLayoutsMap[mesh->vertexClass];
		auto& rootSignature = meshShadowMapRootSignatures[mesh];
		auto& vsByteCode = material->vertexShader->byteCode;
		auto& psByteCode = material->pixelShader->byteCode;

		RenderablePipelineState pipelineState;
		BuildPipelineStateFromJsonChain(pipelineState, { GetMaterialTemplate(material->material), json });
		meshShadowMapPipelineStates.insert_or_assign(mesh, CreatePipelineState(mesh->uuid, vsLayout, vsByteCode, psByteCode, rootSignature, pipelineState));
	}

	void Renderable::CreateBoundingBox()
	{
		if (meshes.size() == 0ULL)
		{
			boundingBox = BoundingBox(
				{ 0.0f, 0.0f, 0.0f },
				{ 0.5f, 0.5f, 0.5f }
			);
			return;
		}
		auto it = meshes.begin();
		boundingBox = (*it)->boundingBox;
		it++;
		while (it != meshes.end())
		{
			BoundingBox out;
			BoundingBox::CreateMerged(out, boundingBox, (*it)->boundingBox);
			boundingBox = out;
			it++;
		}
	}

	std::shared_ptr<Renderable> GetRenderable(std::string uuid)
	{
		return renderables.at(uuid);
	}

	//READ
	std::map<std::string, std::shared_ptr<Renderable>>& GetRenderables() { return renderables; }

	std::map<std::string, std::shared_ptr<Renderable>>& GetAnimables() { return animables; }

#if defined(_EDITOR)
	std::vector<std::string> GetRenderablesNames()
	{
		std::map<std::string, std::shared_ptr<Renderable>> r;
		std::copy_if(renderables.begin(), renderables.end(), std::inserter(r, r.end()), [](const auto& pair)
			{
				return !pair.second->hidden();
			}
		);

		return nostd::GetKeysFromMap(r);
	}

	std::vector<UUIDName> GetRenderablesUUIDNames()
	{
		return GetSceneObjectsUUIDsNames(renderables);
	}
#endif

	void Renderable::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox) {
		BoundingBox bb;

		//if (!animable)
		{
			bb = boundingBox;
		}
		/*
		else
		{
			using namespace Animation;
			auto bonesCbv = GetAnimatedConstantsBuffer(this_ptr);
			XMMATRIX* bones = reinterpret_cast<XMMATRIX*>(bonesCbv->mappedConstantBuffer + bonesCbv->alignedConstantBufferSize * renderer->backBufferIndex);
			bb = animable->GetAnimatedBoundingBox(bones);
		}
		*/

		XMVECTOR pv = { bb.Center.x, bb.Center.y, bb.Center.z };
		XMFLOAT3 rotV = rotation();
		XMVECTOR rot = XMQuaternionRotationRollPitchYaw(rotV.x, rotV.y, rotV.z);
		pv = XMVector3Rotate(pv, rot);
		XMFLOAT3 boxP = { pv.m128_f32[0],pv.m128_f32[1],pv.m128_f32[2] };
		bbox->position(boxP * scale() + position());
		bbox->scale(bb.Extents * scale());
		bbox->rotation(rotV);
	}

	//UPDATE
	void RenderablesStep()
	{
#if defined(_EDITOR)
		std::map<std::string, std::shared_ptr<Renderable>> renderablesRebuildMaterials;
		std::copy_if(renderables.begin(), renderables.end(), std::inserter(renderablesRebuildMaterials, renderablesRebuildMaterials.end()), [](const auto& pair)
			{
				return pair.second->renderableUpdateFlags & RenderableFlags_RebuildMaterials;
			}
		);

		std::map<std::string, std::shared_ptr<Renderable>> renderablesToSwaps;
		std::copy_if(renderables.begin(), renderables.end(), std::inserter(renderablesToSwaps, renderablesToSwaps.end()), [](const auto& pair)
			{
				return !pair.second->materialSwaps.empty() && pair.second->renderableUpdateFlags & RenderableFlags_SwapMaterialsFromMesh;
			}
		);

		std::map<std::string, std::shared_ptr<Renderable>> renderablesToDestroyMeshes;
		std::copy_if(renderables.begin(), renderables.end(), std::inserter(renderablesToDestroyMeshes, renderablesToDestroyMeshes.end()), [](const auto& pair)
			{
				return pair.second->renderableUpdateFlags & RenderableFlags_DestroyMeshes;
			}
		);

		std::map<std::string, std::shared_ptr<Renderable>> renderablesToCreateMeshes;
		std::copy_if(renderables.begin(), renderables.end(), std::inserter(renderablesToCreateMeshes, renderablesToCreateMeshes.end()), [](const auto& pair)
			{
				return pair.second->renderableUpdateFlags & RenderableFlags_CreateMeshes;
			}
		);

		std::map<std::string, std::shared_ptr<Renderable>> renderablesToCreateModels3D;
		std::copy_if(renderables.begin(), renderables.end(), std::inserter(renderablesToCreateModels3D, renderablesToCreateModels3D.end()), [](const auto& pair)
			{
				return pair.second->renderableUpdateFlags & RenderableFlags_CreateMeshesFromModel3D;
			}
		);

		std::map<std::string, std::shared_ptr<Renderable>> renderablesToDestroy;
		std::copy_if(renderables.begin(), renderables.end(), std::inserter(renderablesToDestroy, renderablesToDestroy.end()), [](const auto& pair)
			{
				return pair.second->renderableUpdateFlags & RenderableFlags_Destroy;
			}
		);

		if (renderablesRebuildMaterials.size() > 0ULL || renderablesToSwaps.size() > 0ULL || renderablesToDestroyMeshes.size() > 0ULL ||
			renderablesToCreateMeshes.size() > 0ULL || renderablesToCreateModels3D.size() > 0ULL || renderablesToDestroy.size() > 0ULL)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([&renderablesRebuildMaterials, &renderablesToSwaps, &renderablesToDestroyMeshes, &renderablesToCreateMeshes, &renderablesToCreateModels3D, &renderablesToDestroy]
				{
					for (auto& [name, renderable] : renderablesRebuildMaterials)
					{
						renderable->DestroyMaterialsToRebuild();
					}

					for (auto& [name, renderable] : renderablesRebuildMaterials)
					{
						renderable->RebuildMaterials();
					}

					for (auto& [name, renderable] : renderablesToDestroyMeshes)
					{
						renderable->CleanMeshes();
					}

					for (auto& [name, renderable] : renderablesToSwaps)
					{
						renderable->SwapMaterials();
					}

					for (auto& [name, renderable] : renderablesToCreateMeshes)
					{
						renderable->SwapMeshes();
					}

					for (auto& [name, renderable] : renderablesToCreateModels3D)
					{
						renderable->SwapModel3D();
					}

					for (auto& [name, renderable] : renderablesToDestroy)
					{
						DestroyRenderable(renderable);
					}
				}
			);
		}
#endif
	}

#if defined(_EDITOR)
	void Renderable::DestroyMaterialsToRebuild()
	{
		for (unsigned int meshIndex : materialToChangeMeshIndex)
		{
			std::shared_ptr<MeshInstance> mesh = meshes.at(meshIndex);

			{
				//destroy material
				if (!model3D)
				{
					std::shared_ptr<MaterialInstance> mat = meshMaterials.at(mesh);
					DestroyMaterialInstance(mat, mesh, json);
				}
				else
				{
					DestroyMaterialInstance(model3D->materials[meshIndex], model3D->meshes[meshIndex], model3D->shaderAttributes);
				}
				meshMaterials.at(mesh) = nullptr;

				//destroy constants buffer
				for (std::shared_ptr<ConstantsBuffer>& cbuffer : meshConstantsBuffer.at(mesh))
				{
					DestroyConstantsBuffer(cbuffer);
				}
				meshConstantsBuffer.at(mesh).clear();

				//destroy root signature
				meshRootSignatures.at(mesh).Release();

				//destroy shader pipeline state
				meshPipelineStates.at(mesh).Release();
			}

			if (meshShadowMapMaterials.contains(mesh))
			{
				//destroy material
				std::shared_ptr<MaterialInstance> mat = meshShadowMapMaterials.at(mesh);
				DestroyMaterialInstance(mat, mesh, defaultShadowMapShaderAttributes);
				meshShadowMapMaterials.at(mesh) = nullptr;

				//destroy constants buffer
				for (std::shared_ptr<ConstantsBuffer>& cbuffer : meshShadowMapConstantsBuffer.at(mesh))
				{
					DestroyConstantsBuffer(cbuffer);
				}
				meshShadowMapConstantsBuffer.at(mesh).clear();

				//destroy root signature
				meshShadowMapRootSignatures.at(mesh).Release();

				//destroy shader pipeline state
				meshShadowMapPipelineStates.at(mesh).Release();
			}
		}
	}

	void Renderable::RebuildMaterials()
	{
		for (unsigned int meshIndex : materialToChangeMeshIndex)
		{
			std::shared_ptr<MeshInstance> mesh = meshes.at(meshIndex);
			std::shared_ptr<MaterialInstance> materialInstance = nullptr;

			if (!model3D)
			{
				materialInstance = GetMaterialInstance(materialToRebuild.at(meshIndex), std::map<TextureType, MaterialTexture>(), mesh, json);
			}
			else
			{
				materialInstance = model3D->GetModel3DMaterialInstance(meshIndex);
				model3D->materials[meshIndex] = materialInstance;
			}

			SetMeshMaterial(mesh, materialInstance);
			BindChangesToMaterial(meshIndex);
			CreateMeshConstantsBuffers(mesh);
			CreateMeshRootSignatures(mesh);
			CreateMeshPipelineState(mesh);
			CreateMeshShadowMapConstantsBuffers(mesh);
			CreateMeshShadowMapRootSignatures(mesh);
			CreateMeshShadowMapPipelineState(mesh);
		}

		materialToChangeMeshIndex.clear();

		renderableUpdateFlags &= ~RenderableFlags_RebuildMaterials;
	}

	void Renderable::CleanMeshes()
	{
		//model3D = nullptr;
		skipMeshes.clear();

		auto destroyMeshInstance = [](auto& vec)
			{
				for (auto& mesh : vec)
				{
					DestroyMeshInstance(mesh);
				}
			};

		destroyMeshInstance(meshes);
		if (!model3D)
		{
			destroyMeshInstance(meshesShadowMap);
		}

		if (!model3D)
		{
			std::shared_ptr<MaterialInstance> mat = meshMaterials.at(meshes.at(0));
			DestroyMaterialInstance(mat, meshes.at(0), json);
		}
		else
		{
			for (unsigned int i = 0U; i < meshes.size(); i++)
			{
				DestroyMaterialInstance(model3D->materials[i], model3D->meshes[i], model3D->shaderAttributes);
			}
		}

		meshes.clear();
		meshesShadowMap.clear();

		//meshes.clear();
		//meshesShadowMap.clear();

		meshMaterials.clear();
		meshShadowMapMaterials.clear();

		meshConstantsBuffer.clear();
		meshShadowMapConstantsBuffer.clear();
		meshRootSignatures.clear();
		meshShadowMapRootSignatures.clear();
		meshPipelineStates.clear();
		meshShadowMapPipelineStates.clear();
		if (animables.contains(name()))
		{
			animables.erase(name());
		}
		animable = nullptr;
		bonesTransformation.clear();
		if (model3D != nullptr)
		{
			DestroyModel3DInstance(model3D);
		}
		renderableUpdateFlags &= ~RenderableFlags_DestroyMeshes;
		CreateBoundingBox();
	}

	void Renderable::SwapMaterials()
	{
		for (auto& [mesh, materialUUID] : materialSwaps)
		{
			std::shared_ptr<MaterialInstance> materialInstance = GetMaterialInstance(materialUUID, std::map<TextureType, MaterialTexture>(), mesh, json);
			SetMeshMaterial(mesh, materialInstance);
			CreateMeshConstantsBuffers(mesh);
			CreateMeshRootSignatures(mesh);
			CreateMeshPipelineState(mesh);
			CreateMeshShadowMapConstantsBuffers(mesh);
			CreateMeshShadowMapRootSignatures(mesh);
			CreateMeshShadowMapPipelineState(mesh);
		}
		materialSwaps.clear();
		renderableUpdateFlags &= ~RenderableFlags_SwapMaterialsFromMesh;
	}

	void Renderable::SwapMeshes()
	{
		std::shared_ptr<MeshInstance> meshInstance = GetMeshInstance(meshSwap);
		meshes.push_back(meshInstance);
		meshSwap.clear();
		CreateBoundingBox();
		renderableUpdateFlags &= ~RenderableFlags_CreateMeshes;
	}

	void Renderable::SwapModel3D()
	{
		CreateFromModel3D(model3DSwap);
		CreateMeshesComponents();
		model3DSwap.clear();
		renderableUpdateFlags &= ~RenderableFlags_CreateMeshesFromModel3D;
	}

	void Renderable::ReloadModel3D()
	{
		renderableUpdateFlags |= RenderableFlags_RebuildMeshesFromModel3D;
		model3DSwap = json.at("model");
	}

#endif

	void Renderable::WriteMaterialVariablesToConstantsBufferSpace(std::shared_ptr<MaterialInstance>& material, std::shared_ptr<ConstantsBuffer>& cbvData, unsigned int cbvFrameIndex)
	{
		for (auto& [varName, varMapping] : material->variablesMapping)
		{
			UINT8* source = material->variablesBuffer[varMapping.mapping.bufferIndex].data() + varMapping.mapping.offset;
			UINT8* destination = cbvData->mappedConstantBuffer + (cbvFrameIndex * cbvData->alignedConstantBufferSize) + varMapping.mapping.offset;
			memcpy(destination, source, varMapping.mapping.size);
		}
	}

	void Renderable::WirteAnimationConstantsBuffer(unsigned int backbufferIndex)
	{
		if (!animable) return;

		using namespace Animation;
		WriteBoneTransformationsToConstantsBuffer(this_ptr, bonesTransformation, backbufferIndex);
	}

	void Renderable::WriteConstantsBuffer(unsigned int backbufferIndex)
	{
		XMFLOAT3 posV = position();
		XMFLOAT3 rotV = rotation();
		XMFLOAT3 scaleV = scale();
		XMMATRIX rotationM = XMMatrixRotationRollPitchYawFromVector({ rotV.x, rotV.y, rotV.z, 0.0f });
		XMMATRIX scaleM = XMMatrixScalingFromVector({ scaleV.x, scaleV.y, scaleV.z });
		XMMATRIX positionM = XMMatrixTranslationFromVector({ posV.x, posV.y, posV.z });
		XMMATRIX world = XMMatrixMultiply(XMMatrixMultiply(scaleM, rotationM), positionM);

		WriteConstantsBuffer("world", world, backbufferIndex);
	}

	void Renderable::WriteShadowMapConstantsBuffer(unsigned int backbufferIndex)
	{
		XMFLOAT3 posV = position();
		XMFLOAT3 rotV = rotation();
		XMFLOAT3 scaleV = scale();
		XMMATRIX rotationM = XMMatrixRotationRollPitchYawFromVector({ rotV.x, rotV.y, rotV.z, 0.0f });
		XMMATRIX scaleM = XMMatrixScalingFromVector({ scaleV.x, scaleV.y, scaleV.z });
		XMMATRIX positionM = XMMatrixTranslationFromVector({ posV.x, posV.y, posV.z });
		XMMATRIX world = XMMatrixMultiply(XMMatrixMultiply(scaleM, rotationM), positionM);

		WriteShadowMapConstantsBuffer("world", world, backbufferIndex);
	}

	void Renderable::SetCurrentAnimation(std::string animation, float animationTime, float timeFactor, bool autoPlay)
	{
		currentAnimation = animation;
		currentAnimationTime = animationTime;
		animationTimeFactor = timeFactor;
		playingAnimation = autoPlay;
	}

	void Renderable::StepAnimation(double elapsedSeconds)
	{
		currentAnimationTime += playingAnimation ? animationTimeFactor * static_cast<float>(elapsedSeconds) * 1000.0f : 0.0f;
		float animationLength = animable->animations->animationsLength[currentAnimation];

		if (animationTimeFactor > 0.0f)
		{
			if (currentAnimationTime >= animationLength)
			{
				currentAnimationTime = fmodf(currentAnimationTime, animationLength);
			}
		}
		else if (animationTimeFactor < 0.0f)
		{
			if (currentAnimationTime < 0.0f)
			{
				currentAnimationTime = animationLength - fmodf(currentAnimationTime, animationLength);
			}
		}

		using namespace Animation;
		TraverseMultiplycationQueue(currentAnimationTime, currentAnimation, animable->animations, bonesTransformation);
	}

	//DESTROY
	void Renderable::Destroy()
	{
		for (auto& [mesh, mat] : meshMaterials) { DestroyMaterialInstance(mat, mesh, json); } meshMaterials.clear();
		for (auto& [mesh, mat] : meshShadowMapMaterials) { DestroyMaterialInstance(mat, mesh, defaultShadowMapShaderAttributes); } meshShadowMapMaterials.clear();

		for (auto& [mesh, cbuffers] : meshConstantsBuffer) { for (auto& cbuffer : cbuffers) DestroyConstantsBuffer(cbuffer); } meshConstantsBuffer.clear();
		for (auto& [mesh, cbuffers] : meshShadowMapConstantsBuffer) { for (auto& cbuffer : cbuffers) DestroyConstantsBuffer(cbuffer); } meshShadowMapConstantsBuffer.clear();

		auto releaseSecond = [](auto& map) { for (auto& [first, second] : map) { second.Release(); second = nullptr; } };

		releaseSecond(meshRootSignatures);
		releaseSecond(meshShadowMapRootSignatures);
		releaseSecond(meshPipelineStates);
		releaseSecond(meshShadowMapPipelineStates);

		meshRootSignatures.clear();
		meshShadowMapRootSignatures.clear();
		meshPipelineStates.clear();
		meshShadowMapPipelineStates.clear();

		auto destroyMeshInstance = [](auto& vec) { for (auto& mesh : vec) { DestroyMeshInstance(mesh); } };

		destroyMeshInstance(meshes);
		if (!model3D) { destroyMeshInstance(meshesShadowMap); }

		meshes.clear();
		meshesShadowMap.clear();

		if (model3D)
		{
			DestroyModel3DInstance(model3D);
		}
	}

	//RENDER
	void Renderable::Render(std::shared_ptr<Camera> camera)
	{
		using namespace Animation;

		if (!visible()) return;

		auto& commandList = renderer->commandList;

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, name().c_str());
#endif

		commandList->IASetPrimitiveTopology(topology());

		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			if (skipMeshes.contains(i) || !meshMaterials.contains(meshes[i])) continue;

			auto& mesh = meshes.at(i);
			auto& material = meshMaterials.at(mesh);
			auto& rootSignature = meshRootSignatures.at(mesh);
			auto& pipelineState = meshPipelineStates.at(mesh);

			commandList->SetGraphicsRootSignature(rootSignature);
			commandList->SetPipelineState(pipelineState);

			unsigned int cbvSlot = 0U;

			if (meshConstantsBuffer.contains(mesh))
			{
				auto& constantsBuffer = meshConstantsBuffer.at(mesh);
				for (unsigned int i = 0U; i < constantsBuffer.size(); i++) {
					constantsBuffer[i]->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
				}
			}

			if (camera && material->ShaderInstanceHasRegister([](auto& binary) { return binary->cameraCBVRegister; })) {
				camera->cameraCbv->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
			}

			if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->lightCBVRegister; })) {
				GetLightsConstantsBuffer()->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
			}

			if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->lightsShadowMapCBVRegister; })) {
				if (SceneHasShadowMaps()) {
					GetShadowMapConstantsBuffer()->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
				}
				else
				{
					cbvSlot++;
				}
			}

			if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->animationCBVRegister; })) {
				GetAnimatedConstantsBuffer(this_ptr)->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
			}

			material->SetRootDescriptorTable(commandList, cbvSlot);

			if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->lightsShadowMapSRVRegister; })) {
				if (SceneHasShadowMaps()) {
					commandList->SetGraphicsRootDescriptorTable(cbvSlot, GetShadowMapGpuDescriptorHandleStart());
				}
				cbvSlot++;
			}

			commandList->IASetVertexBuffers(0, 1, &mesh->vbvData.vertexBufferView);
			commandList->IASetIndexBuffer(&mesh->ibvData.indexBufferView);
			commandList->DrawIndexedInstanced(mesh->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);
		}

#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
	}

	void Renderable::RenderShadowMap(const std::shared_ptr<Light>& light, unsigned int cameraIndex)
	{
		using namespace Animation;

		if (!visible() || !meshShadowMapMaterials.size()) return;

		auto& commandList = renderer->commandList;
		auto& shadowMapCameras = light->shadowMapCameras;

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, name().c_str());
#endif

		commandList->IASetPrimitiveTopology(topology());

		for (unsigned int i = 0; i < meshesShadowMap.size(); i++)
		{
			if (skipMeshes.contains(i)) continue;

			auto& mesh = meshesShadowMap.at(i);
			auto& material = meshShadowMapMaterials.at(mesh);
			auto& rootSignature = meshShadowMapRootSignatures.at(mesh);
			auto& pipelineState = meshShadowMapPipelineStates.at(mesh);

			//don't draw things without a camera, bad shader(sorry)
			if (!material->ShaderInstanceHasRegister([](auto& binary) { return binary->cameraCBVRegister; })) continue;

			commandList->SetGraphicsRootSignature(rootSignature);
			commandList->SetPipelineState(pipelineState);

			unsigned int cbvSlot = 0U;

			if (meshShadowMapConstantsBuffer.contains(mesh))
			{
				auto& constantsBuffer = meshShadowMapConstantsBuffer.at(mesh);
				for (unsigned int i = 0U; i < constantsBuffer.size(); i++) {
					constantsBuffer[i]->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
				}
			}

			if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->cameraCBVRegister; })) {
				shadowMapCameras[cameraIndex]->cameraCbv->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
			}

			if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->animationCBVRegister; })) {
				GetAnimatedConstantsBuffer(this_ptr)->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
			}

			material->SetRootDescriptorTable(commandList, cbvSlot);

			commandList->IASetVertexBuffers(0, 1, &mesh->vbvData.vertexBufferView);
			commandList->IASetIndexBuffer(&mesh->ibvData.indexBufferView);
			commandList->DrawIndexedInstanced(mesh->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);
		}

#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif

	}

	//EDITOR
#if defined(_EDITOR)

	void SelectRenderable(std::string uuid, std::string& edSO) {
		edSO = uuid;
	}

	void DeSelectRenderable(std::string& edSO)
	{
		edSO = "";
	}

	std::string GetRenderableName(std::string uuid)
	{
		return renderables.at(uuid)->json.at("name");
	}

	void DeleteRenderable(std::string uuid)
	{
		renderables.at(uuid)->renderableUpdateFlags |= RenderableFlags_Destroy;
	}

	void DrawRenderablesPopups()
	{
	}

	void DrawRenderablePanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop)
	{
		std::shared_ptr<Renderable> renderable = renderables.at(uuid);

		std::string tableName = "renderable-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			renderable->DrawEditorInformationAttributes();
			renderable->DrawEditorWorldAttributes();
			renderable->DrawEditorAnimationAttributes();
			renderable->DrawEditorShaderAttributes();
			renderable->DrawEditorModelSelectionAttributes();
			renderable->DrawEditorMeshesAttributes();
			renderable->DrawEditorPipelineStateAttributes();
			ImGui::EndTable();
		}
	}

	void Renderable::DrawEditorInformationAttributes()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "renderable-information-atts";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string currentName = name();
			if (ImGui::InputText("name", &currentName))
			{
				if (!renderables.contains(currentName))
				{
					renderables[currentName] = renderables[name()];
					renderables.erase(name());
				}
				if (!animables.contains(currentName) && animables.contains(name()))
				{
					animables[currentName] = animables[name()];
					animables.erase(name());
				}
				name(currentName);
			}
			ImGui::EndTable();
		}

	}

	void Renderable::DrawEditorWorldAttributes()
	{
		XMFLOAT3 posV = position();
		XMFLOAT3 rotV = rotation();
		XMFLOAT3 scaleV = scale();
		ImDrawFloatValues<XMFLOAT3>("renderable-world-position", { "x","y","z" }, posV, [this](XMFLOAT3 pos) {position(pos); });
		ImDrawDegreesValues<XMFLOAT3>("renderable-world-rotation", { "roll","pitch","yaw" }, rotV, [this](XMFLOAT3 rot) {rotation(rot); });
		ImDrawFloatValues<XMFLOAT3>("renderable-world-scale", { "x","y","z" }, scaleV, [this](XMFLOAT3 s) {scale(s); });
	}

	void Renderable::DrawEditorAnimationAttributes()
	{
		if (!animable) return;

		ImGui::Text("animations");
		std::vector<std::string> selectables = { " " };
		for (auto& [name, length] : animable->animations->animationsLength)
		{
			if (name == "") continue;
			selectables.push_back(name);
		}

		int current_item = 0;
		if (currentAnimation != "")
		{
			current_item = static_cast<int>(std::find(selectables.begin(), selectables.end(), currentAnimation) - selectables.begin());
		}

		DrawComboSelection(selectables[current_item], selectables, [this](std::string animName)
			{
				SetCurrentAnimation(animName != " " ? animName : "");
			}
		);

		if (ImGui::Button(ICON_FA_BACKWARD))
		{
			int animIdx = current_item - 1;
			if (animIdx == -1) animIdx = static_cast<int>(selectables.size()) - 1;
			std::string animName = selectables[animIdx];
			SetCurrentAnimation(animName != " " ? animName : "");
		}
		ImGui::SameLine();

		if (playingAnimation)
		{
			if (ImGui::Button(ICON_FA_PAUSE)) { playingAnimation = false; }
		}
		else
		{
			if (ImGui::Button(ICON_FA_PLAY)) { playingAnimation = true; }
		}
		ImGui::SameLine();

		if (ImGui::Button(ICON_FA_STOP))
		{
			playingAnimation = false;
			currentAnimationTime = 0.0f;
		}
		ImGui::SameLine();
		if (animationTimeFactor > 0.0f)
		{
			if (ImGui::Button(ICON_FA_UNDO))
			{
				animationTimeFactor = -animationTimeFactor;
			}
		}
		else if (animationTimeFactor < 0.0f)
		{
			if (ImGui::Button(ICON_FA_REDO))
			{
				animationTimeFactor = -animationTimeFactor;
			}
		}
		else
		{
			if (ImGui::Button(ICON_FA_SYNC))
			{
				animationTimeFactor = 1.0f;
			}
		}
		ImGui::SameLine();

		if (ImGui::Button(ICON_FA_FORWARD))
		{
			int animIdx = current_item + 1;
			if (animIdx == selectables.size()) animIdx = 0;
			std::string animName = selectables[animIdx];
			SetCurrentAnimation(animName != " " ? animName : "");
		}
		ImGui::NewLine();

		if (currentAnimation != "")
		{
			float sliderTime = currentAnimationTime / 1000.0f;
			ImGui::Text("time");
			ImGui::Text("0s");
			ImGui::SameLine();
			if (ImGui::SliderFloat((std::to_string(animable->animations->animationsLength[currentAnimation] / 1000.0f) + "s").c_str(), &sliderTime, 0.0f, animable->animations->animationsLength[currentAnimation] / 1000.0f))
			{
				playingAnimation = false;
				currentAnimationTime = 1000.0f * sliderTime;
			}
		}
	}

	void Renderable::DrawEditorShaderAttributes()
	{
		auto rebuildMaterials = [this]
			{
				renderableUpdateFlags |= RenderableFlags_SwapMaterialsFromMesh;
				for (auto& [mesh, mat] : meshMaterials)
				{
					materialSwaps.insert_or_assign(mesh, mat->material);
				}
			};

		if (ImGui::Checkbox("Unique Materials instances", json.at("uniqueMaterialInstance").get_ptr<bool*>())) { rebuildMaterials(); }
		if (ImGui::Checkbox("Cast Shadows", json.at("castShadows").get_ptr<bool*>())) { rebuildMaterials(); }
	}

	void Renderable::DrawEditorModelSelectionAttributes()
	{
		std::vector<UUIDName> modelsUUIDNames = GetModels3DUUIDsNames();
		std::vector<UUIDName> meshesUUIDNames = GetMeshesUUIDsNames();

		std::vector<UUIDName> selectables = { std::tie(" ", " ") };
		selectables.insert(selectables.end(), modelsUUIDNames.begin(), modelsUUIDNames.end());
		selectables.insert(selectables.end(), meshesUUIDNames.begin(), meshesUUIDNames.end());

		UUIDName model3dUUIDName;
		bool model3DSelected = false;
		if (model3D != nullptr)
		{
			model3DSelected = true;

			Model3DTemplate t = GetModel3DTemplate(model3D->uuid);

			std::string& mUUID = std::get<0>(model3dUUIDName);
			mUUID = model3D->uuid;

			std::string& mName = std::get<1>(model3dUUIDName);
			mName = std::get<0>(t);
		}

		int current_item = 0;
		int current_model = static_cast<int>(std::find(modelsUUIDNames.begin(), modelsUUIDNames.end(), model3dUUIDName) - modelsUUIDNames.begin());

		if (current_model != modelsUUIDNames.size())
		{
			current_item = 1 + current_model;
		}
		else if (meshes.size() > 0)
		{
			std::string meshUUID = meshes.at(0)->uuid;
			int current_mesh = static_cast<int>(std::find_if(meshesUUIDNames.begin(), meshesUUIDNames.end(), [meshUUID](UUIDName uuidName)
				{
					return meshUUID == std::get<0>(uuidName);
				}
			) - meshesUUIDNames.begin());
			if (current_mesh != meshesUUIDNames.size())
			{
				current_item = 1 + static_cast<int>(modelsUUIDNames.size()) + current_mesh;
			}
		}

		ImGui::PushID("renderable-mesh-model-selector");
		{
			if (model3DSelected)
			{
				if (ImGui::Button(ICON_FA_CUBE))
				{
					Editor::tempTab = T_Models3D;
					Editor::selTemp = model3D->uuid;
				}
				ImGui::SameLine();
			}

			DrawComboSelection(selectables[current_item], selectables, [this, modelsUUIDNames, meshesUUIDNames](UUIDName uuidName)
				{
					std::string uuid = std::get<0>(uuidName);
					if (uuid == " ")
					{
						renderableUpdateFlags |= RenderableFlags_DestroyMeshes;
					}
					else if (std::find(modelsUUIDNames.begin(), modelsUUIDNames.end(), uuidName) != modelsUUIDNames.end())
					{
						renderableUpdateFlags |= RenderableFlags_RebuildMeshesFromModel3D;
						model3DSwap = uuid;
					}
					else if (std::find(meshesUUIDNames.begin(), meshesUUIDNames.end(), uuidName) != meshesUUIDNames.end())
					{
						renderableUpdateFlags |= RenderableFlags_RebuildMeshes;
						meshSwap = uuid;
					}
				}, "model"
			);
		}
		ImGui::PopID();
	}

	void Renderable::DrawEditorMeshesAttributes()
	{
		std::string tableName = "renderable-meshes-atts";
		if (ImGui::BeginTable(tableName.c_str(), 3, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableSetupColumn("skip", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("mesh", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("material", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TableHeader("skip");
			ImGui::TableSetColumnIndex(1);
			ImGui::TableHeader("mesh");
			ImGui::TableSetColumnIndex(2);
			ImGui::TableHeader("material");

			std::vector<UUIDName> materialsUUIDNames = GetMaterialsUUIDsNames();
			std::vector<UUIDName> selectables = { std::tie(" ", " ") };
			nostd::AppendToVector(selectables, materialsUUIDNames);

			for (unsigned int i = 0; i < meshes.size(); i++)
			{
				std::shared_ptr<MeshInstance>& mesh = meshes[i];

				ImGui::TableNextRow();

				bool skip_v = skipMeshes.contains(i);
				ImGui::TableSetColumnIndex(0);
				ImGui::PushID(("mesh#" + mesh->uuid + "#skip").c_str());
				{
					if (ImGui::Checkbox("", &skip_v))
					{
						if (skip_v)
						{
							skipMeshes.insert(i);
						}
						else
						{
							skipMeshes.erase(i);
						}
					}
				}
				ImGui::PopID();

				ImGui::TableSetColumnIndex(1);
				if (model3D != nullptr)
				{
					ImGui::Text((std::get<0>(GetModel3DTemplate(model3D->uuid)) + "." + std::to_string(i)).c_str());
				}
				else
				{
					ImGui::Text(GetMeshName(mesh->uuid).c_str());
				}

				ImGui::TableSetColumnIndex(2);

				ImGui::PushID(("mesh#" + mesh->uuid + "#material").c_str());
				{
					bool selectedMaterial = false;
					UUIDName currentMaterial = std::tie(" ", " ");

					if (meshMaterials.contains(mesh))
					{
						std::shared_ptr<MaterialInstance> material = meshMaterials.at(mesh);
						nlohmann::json json = GetMaterialTemplate(material->material);

						std::string& uuid = std::get<0>(currentMaterial);
						std::string& name = std::get<1>(currentMaterial);

						uuid = material->material;
						name = GetMaterialName(uuid);
						selectedMaterial = true;
					}

					if (selectedMaterial)
					{
						if (ImGui::Button(ICON_FA_TSHIRT))
						{
							Editor::tempTab = T_Materials;
							Editor::selTemp = std::get<0>(currentMaterial);
						}
						ImGui::SameLine();
					}


					ImGui::SetNextItemWidth(-1);

					DrawComboSelection(currentMaterial, selectables, [this, mesh](UUIDName matUUIDName)
						{
							materialSwaps.insert_or_assign(mesh, std::get<0>(matUUIDName));
							renderableUpdateFlags |= RenderableFlags_SwapMaterialsFromMesh;
						}
					);

				}
				ImGui::PopID();
			}

			ImGui::EndTable();
		}
	}

	void Renderable::DrawEditorPipelineStateAttributes()
	{
		nlohmann::json currentJson = json;
		ImDrawPipelineState(json.at("pipelineState"));
		if (currentJson != json)
		{
			renderableUpdateFlags |= RenderableFlags_RebuildMaterials;
			for (unsigned int i = 0; i < meshes.size(); i++)
			{
				materialToChangeMeshIndex.push_back(i);
				materialToRebuild.push_back(meshMaterials.at(meshes.at(i))->material);
			}
		}
	}
#endif

	//DESTROY
	void DestroyRenderable(std::shared_ptr<Renderable>& renderable)
	{
		if (renderable == nullptr) return;
#if defined(_EDITOR)
		if (renderable->model3D != nullptr) UnbindNotifications(renderable->model3D->uuid, renderable);
#endif
		if (renderables.contains(renderable->uuid())) renderables.erase(renderable->uuid());
		if (animables.contains(renderable->uuid())) animables.erase(renderable->uuid());
		renderable->this_ptr = nullptr;
		renderable = nullptr;
	}

	void DestroyRenderables()
	{
		for (auto& [name, renderable] : renderables)
		{
			DEBUG_PTR_COUNT_JSON(renderable);
#if defined(_EDITOR)
			if (renderable->model3D != nullptr) UnbindNotifications(renderable->model3D->uuid, renderable);
#endif
			renderable->this_ptr = nullptr;
		}
		animables.clear();
		renderables.clear();
	}

}