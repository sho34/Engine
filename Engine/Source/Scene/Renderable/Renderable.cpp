#include "pch.h"
#include <ppltasks.h>
#include <vector>
#include <nlohmann/json.hpp>
#include "Renderable.h"
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
#endif

extern std::mutex rendererMutex;
extern std::shared_ptr<Renderer> renderer;
namespace Scene {

	//NOTIFICATIONS
	static void OnMaterialChangeStart(void* renderablePtr) {
		//auto renderable = static_cast<Renderable*>(renderablePtr);
		//renderable->loading = true;
	}

	static void OnMaterialChangeComplete(void* renderablePtr, void* materialPtr) {
		//auto renderable = static_cast<Renderable*>(renderablePtr);
		//renderable->Initialize(renderer);
		//renderable->loading = false;
	}

	static void OnMaterialDestroy(void* renderablePtr, void* materialPtr) { }

	//NAMESPACE VARIABLES
	std::map<std::string, std::shared_ptr<Renderable>> renderables;
	std::map<std::string, std::shared_ptr<Renderable>> animables;

	void TranformJsonToMeshMaterialMap(MeshMaterialMap& map, nlohmann::json j, bool uniqueMaterialInstance)
	{
		std::transform(j.begin(), j.end(), std::inserter(map, map.end()), [uniqueMaterialInstance](const nlohmann::json& value)
			{
				std::map<TextureType, MaterialTexture> textures;
				TransformJsonToMaterialTextures(textures, value, "textures");

				std::shared_ptr<MeshInstance> mesh = GetMeshInstance(value["mesh"]);
				std::shared_ptr<MaterialInstance> material = GetMaterialInstance(value["material"], textures, mesh, uniqueMaterialInstance);
				return MeshMaterialPair(mesh, material);
			}
		);
	}

	void TransformJsonToRenderTargetBlendDesc(D3D12_RENDER_TARGET_BLEND_DESC& RenderTarget, nlohmann::json j)
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

	void TransformJsonToBlendState(D3D12_BLEND_DESC& BlendState, nlohmann::json j, std::string key)
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

	void TransformJsonToRasterizerState(D3D12_RASTERIZER_DESC& RasterizerState, nlohmann::json j, std::string key)
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

	void TransformJsonToPipelineState(RenderablePipelineState& pipelineState, nlohmann::json j, std::string key)
	{
		if (!j.contains(key) || j[key].empty()) return;
		nlohmann::json pstate = j[key];

		nostd::InsertFromJsonUsingMap(pipelineState.renderTargetsFormats, stringToDxgiFormat, pstate, "renderTargetsFormats");
		nostd::ReplaceFromJsonUsingMap(pipelineState.depthStencilFormat, stringToDxgiFormat, pstate, "depthStencilFormat");
		nostd::ReplaceFromJsonUsingMap(pipelineState.PrimitiveTopologyType, stringToPrimitiveTopologyType, pstate, "PrimitiveTopologyType");
		TransformJsonToRasterizerState(pipelineState.RasterizerState, pstate, "RasterizerState");
		TransformJsonToBlendState(pipelineState.BlendState, pstate, "BlendState");
	}

	void BuildPipelineStateFromJsonChain(RenderablePipelineState& pipelineState, std::vector<nlohmann::json> jsons)
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
		assert(renderablej["name"] != "");

		std::shared_ptr<Renderable> renderable = std::make_shared<Renderable>();

		renderable->json = renderablej;
		renderable->this_ptr = renderable;

		ReplaceFromJson(renderable->name, renderablej, "name");
		ReplaceFromJson(renderable->visible, renderablej, "visible");
		ReplaceFromJson(renderable->hidden, renderablej, "hidden");
		ReplaceFromJson(renderable->uniqueMaterialInstance, renderablej, "uniqueMaterialInstance");
		JsonToFloat3(renderable->position, renderablej, "position");
		JsonToFloat3(renderable->scale, renderablej, "scale");
		JsonToFloat3(renderable->rotation, renderablej, "rotation");
		TransformJsonArrayToSet(renderable->skipMeshes, renderablej, "skipMeshes");

		std::string model3DName = "";
		if (!renderablej["meshMaterials"].empty())
		{
			TranformJsonToMeshMaterialMap(renderable->meshMaterials, renderablej["meshMaterials"], renderable->uniqueMaterialInstance);
			renderable->meshes.push_back(renderable->meshMaterials.begin()->first);

			TranformJsonToMeshMaterialMap(renderable->meshShadowMapMaterials, renderablej["meshMaterialsShadowMap"]);
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

		renderables.insert_or_assign(renderable->name, renderable);

		return renderable;
	}

	void Renderable::CreateFromModel3D(std::string model3DName)
	{
		std::shared_ptr<Model3DInstance> model = GetModel3DInstance(model3DName);

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
			animables.insert_or_assign(name, this_ptr);
			StepAnimation(0.0f);
		}
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

		std::shared_ptr<MaterialInstance> shadowMapMaterial = GetMaterialInstance("ShadowMap", shadowMapBaseTexture, mesh);
		meshShadowMapMaterials.insert_or_assign(mesh, shadowMapMaterial);
	}

	void Renderable::CreateMeshesComponents()
	{
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
			std::shared_ptr<ConstantsBuffer> cbvData = CreateConstantsBuffer(size, name + "." + mesh->name);
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
			std::shared_ptr<ConstantsBuffer> cbvData = CreateConstantsBuffer(size, name + ".sm." + mesh->name);
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

		meshRootSignatures.insert_or_assign(mesh, CreateRootSignature(mesh->name, vsCBparams, psCBparams, psSRVparams, psSamplersParams, samplers));
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

		meshShadowMapRootSignatures.insert_or_assign(mesh, CreateRootSignature(mesh->name, vsCBparams, psCBparams, psSRVparams, psSamplersParams, samplers));
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
		topology = topologyMap.at(pipelineState.PrimitiveTopologyType);
		meshPipelineStates.insert_or_assign(mesh, CreatePipelineState(mesh->name, vsLayout, vsByteCode, psByteCode, rootSignature, pipelineState));
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
		meshShadowMapPipelineStates.insert_or_assign(mesh, CreatePipelineState(mesh->name, vsLayout, vsByteCode, psByteCode, rootSignature, pipelineState));
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

	//READ
	std::map<std::string, std::shared_ptr<Renderable>>& GetRenderables() { return renderables; }

	std::map<std::string, std::shared_ptr<Renderable>>& GetAnimables() { return animables; }

#if defined(_EDITOR)
	std::vector<std::string> GetRenderablesNames()
	{
		std::map<std::string, std::shared_ptr<Renderable>> r;
		std::copy_if(renderables.begin(), renderables.end(), std::inserter(r, r.end()), [](const auto& pair)
			{
				return !pair.second->hidden;
			}
		);

		return nostd::GetKeysFromMap(r);
	}
#endif

	void Renderable::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox) {
		BoundingBox bb;

		if (!animable)
		{
			bb = boundingBox;
		}
		else
		{
			using namespace Animation;
			auto bonesCbv = GetAnimatedConstantsBuffer(this_ptr);
			XMMATRIX* bones = reinterpret_cast<XMMATRIX*>(bonesCbv->mappedConstantBuffer + bonesCbv->alignedConstantBufferSize * renderer->backBufferIndex);
			bb = animable->GetAnimatedBoundingBox(bones);
		}

		XMVECTOR pv = { bb.Center.x, bb.Center.y, bb.Center.z };
		XMVECTOR rot = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
		pv = XMVector3Rotate(pv, rot);
		XMFLOAT3 boxP = { pv.m128_f32[0],pv.m128_f32[1],pv.m128_f32[2] };
		bbox->position = boxP * scale + position;
		bbox->scale = bb.Extents * scale;
		bbox->rotation = rotation;
	}

	//UPDATE
	void RenderablesStep()
	{
#if defined(_EDITOR)
		std::map<std::string, std::shared_ptr<Renderable>> renderablesToSwaps;
		std::copy_if(renderables.begin(), renderables.end(), std::inserter(renderablesToSwaps, renderablesToSwaps.end()), [](const auto& pair)
			{
				return !pair.second->materialSwaps.empty() && pair.second->renderableUpdateFlags & RenderableFlags_SwapMaterialsFromMesh;
			}
		);

		std::map<std::string, std::shared_ptr<Renderable>> renderablesToDestroy;
		std::copy_if(renderables.begin(), renderables.end(), std::inserter(renderablesToDestroy, renderablesToDestroy.end()), [](const auto& pair)
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

		if (renderablesToSwaps.size() > 0ULL || renderablesToDestroy.size() > 0ULL ||
			renderablesToCreateMeshes.size() > 0ULL || renderablesToCreateModels3D.size() > 0ULL)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([&renderablesToSwaps, &renderablesToDestroy, &renderablesToCreateMeshes, &renderablesToCreateModels3D]
				{
					for (auto& [name, renderable] : renderablesToDestroy)
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
				}
			);
		}
#endif
	}

#if defined(_EDITOR)
	void Renderable::CleanMeshes()
	{
		model3D = nullptr;
		skipMeshes.clear();
		meshes.clear();
		meshesShadowMap.clear();
		meshMaterials.clear();
		meshShadowMapMaterials.clear();
		meshConstantsBuffer.clear();
		meshShadowMapConstantsBuffer.clear();
		meshRootSignatures.clear();
		meshShadowMapRootSignatures.clear();
		meshPipelineStates.clear();
		meshShadowMapPipelineStates.clear();
		if (animables.contains(name))
		{
			animables.erase(name);
		}
		animable = nullptr;
		bonesTransformation.clear();
		renderableUpdateFlags &= ~RenderableFlags_DestroyMeshes;
		CreateBoundingBox();
	}

	void Renderable::SwapMaterials()
	{
		for (auto& [mesh, materialName] : materialSwaps)
		{
			std::shared_ptr<MaterialInstance> materialInstance = GetMaterialInstance(materialName, std::map<TextureType, MaterialTexture>(), mesh, uniqueMaterialInstance);
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
		XMMATRIX rotationM = XMMatrixRotationRollPitchYawFromVector({ rotation.x, rotation.y, rotation.z, 0.0f });
		XMMATRIX scaleM = XMMatrixScalingFromVector({ scale.x, scale.y, scale.z });
		XMMATRIX positionM = XMMatrixTranslationFromVector({ position.x, position.y, position.z });
		XMMATRIX world = XMMatrixMultiply(XMMatrixMultiply(scaleM, rotationM), positionM);

		WriteConstantsBuffer("world", world, backbufferIndex);
	}

	void Renderable::WriteShadowMapConstantsBuffer(unsigned int backbufferIndex)
	{
		XMMATRIX rotationM = XMMatrixRotationRollPitchYawFromVector({ rotation.x, rotation.y, rotation.z, 0.0f });
		XMMATRIX scaleM = XMMatrixScalingFromVector({ scale.x, scale.y, scale.z });
		XMMATRIX positionM = XMMatrixTranslationFromVector({ position.x, position.y, position.z });
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
		for (auto& [mesh, mat] : meshMaterials) { DestroyMaterialInstance(mat, mesh); } meshMaterials.clear();
		for (auto& [mesh, mat] : meshShadowMapMaterials) { DestroyMaterialInstance(mat, mesh); } meshShadowMapMaterials.clear();

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
		destroyMeshInstance(meshesShadowMap);

		meshes.clear();
		meshesShadowMap.clear();

		if (model3D) { DestroyModel3DInstance(model3D); }
	}

	//RENDER
	void Renderable::Render(std::shared_ptr<Camera> camera)
	{
		using namespace Animation;

		if (!visible) return;

		auto& commandList = renderer->commandList;

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, name.c_str());
#endif

		commandList->IASetPrimitiveTopology(topology);

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

			if (camera && material->ShaderBinaryHasRegister([](auto& binary) { return binary->cameraCBVRegister; })) {
				camera->cameraCbv->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
			}

			if (material->ShaderBinaryHasRegister([](auto& binary) { return binary->lightCBVRegister; })) {
				GetLightsConstantsBuffer()->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
			}

			if (material->ShaderBinaryHasRegister([](auto& binary) { return binary->lightsShadowMapCBVRegister; })) {
				if (SceneHasShadowMaps()) {
					GetShadowMapConstantsBuffer()->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
				}
				else
				{
					cbvSlot++;
				}
			}

			if (material->ShaderBinaryHasRegister([](auto& binary) { return binary->animationCBVRegister; })) {
				GetAnimatedConstantsBuffer(this_ptr)->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
			}

			material->SetRootDescriptorTable(commandList, cbvSlot);

			if (material->ShaderBinaryHasRegister([](auto& binary) { return binary->lightsShadowMapSRVRegister; })) {
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

		if (!visible || !meshShadowMapMaterials.size()) return;

		auto& commandList = renderer->commandList;
		auto& shadowMapCameras = light->shadowMapCameras;

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, name.c_str());
#endif

		commandList->IASetPrimitiveTopology(topology);

		for (unsigned int i = 0; i < meshesShadowMap.size(); i++)
		{
			if (skipMeshes.contains(i)) continue;

			auto& mesh = meshesShadowMap.at(i);
			auto& material = meshShadowMapMaterials.at(mesh);
			auto& rootSignature = meshShadowMapRootSignatures.at(mesh);
			auto& pipelineState = meshShadowMapPipelineStates.at(mesh);

			//don't draw things without a camera, bad shader(sorry)
			if (!material->ShaderBinaryHasRegister([](auto& binary) { return binary->cameraCBVRegister; })) continue;

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

			if (material->ShaderBinaryHasRegister([](auto& binary) { return binary->cameraCBVRegister; })) {
				shadowMapCameras[cameraIndex]->cameraCbv->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
			}

			if (material->ShaderBinaryHasRegister([](auto& binary) { return binary->animationCBVRegister; })) {
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

	void SelectRenderable(std::string renderableName, void*& ptr) {
		ptr = renderables.at(renderableName).get();
	}

	void DeSelectRenderable(void*& ptr)
	{
		ptr = nullptr;
	}

	void DrawRenderablePanel(void*& ptr, ImVec2 pos, ImVec2 size, bool pop)
	{
		Renderable* renderable = (Renderable*)ptr;

		std::string tableName = "renderable-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			renderable->DrawEditorInformationAttributes();
			renderable->DrawEditorWorldAttributes();
			renderable->DrawEditorAnimationAttributes();
			renderable->DrawEditorMeshesAttributes();
			ImGui::EndTable();
		}
	}

	std::string GetRenderableName(void* ptr)
	{
		Renderable* renderable = (Renderable*)ptr;
		return renderable->name;
	}

	/*
	nlohmann::json Renderable::json() {
		nlohmann::json j = nlohmann::json({});

		j["name"] = name;
		j["position"] = { position.x, position.y, position.z };
		j["scale"] = { scale.x, scale.y, scale.z };
		j["rotation"] = { rotation.x, rotation.y, rotation.z };

		auto buildJsonMeshMaterials = [](auto& j, std::string objectName, MeshMaterialMap& meshMatMap) {
			j[objectName] = nlohmann::json::array();
			for (auto& [mesh, material] : meshMatMap) {
				j[objectName].push_back({ {"mesh", mesh->name }, {"material", material->name } });
			}
			};

		if (model3DName == "") {
			buildJsonMeshMaterials(j, "meshMaterials", meshMaterials);
			buildJsonMeshMaterials(j, "meshShadowMapMaterials", meshShadowMapMaterials);
		}
		else {
			j["model"] = model3DName;
		}

		j["skipMeshes"] = nlohmann::json::array();
		for (MeshPtr mesh : skipMeshes) {
			j["skipMeshes"].push_back(mesh->name);
		}

		using namespace Animation::Effects;
		auto effects = GetRenderableEffects(this_ptr);
		if (!effects.empty()) {
			j["effects"] = effects;
		}
		return j;
	}
	*/

	void Renderable::DrawEditorInformationAttributes()
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "renderable-information-atts";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string currentName = name;
			if (ImGui::InputText("name", &currentName))
			{
				if (!renderables.contains(currentName))
				{
					renderables[currentName] = renderables[name];
					renderables.erase(name);
				}
				if (!animables.contains(currentName) && animables.contains(name))
				{
					animables[currentName] = animables[name];
					animables.erase(name);
				}
				name = currentName;
			}
			ImGui::EndTable();
		}

	}

	void Renderable::DrawEditorWorldAttributes()
	{
		ImDrawFloatValues<XMFLOAT3>("renderable-world-position", { "x","y","z" }, position, [this](XMFLOAT3 pos) {});
		ImDrawDegreesValues<XMFLOAT3>("renderable-world-rotation", { "roll","pitch","yar" }, rotation, [this](XMFLOAT3 rot) {});
		ImDrawFloatValues<XMFLOAT3>("renderable-world-scale", { "x","y","z" }, scale, [this](XMFLOAT3 scale) {});
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

	void Renderable::DrawEditorMeshesAttributes()
	{
		std::vector<std::string> modelsNames = GetModels3DNames();
		std::vector<std::string> meshesNames = GetMeshesNames();

		std::vector<std::string> selectables = { " " };
		selectables.insert(selectables.end(), modelsNames.begin(), modelsNames.end());
		selectables.insert(selectables.end(), meshesNames.begin(), meshesNames.end());

		std::string model3DName = "";

		if (model3D != nullptr)
		{
			model3DName = GetModel3DInstanceTemplateName(model3D);
		}

		int current_item = 0;
		int current_model = static_cast<int>(std::find(modelsNames.begin(), modelsNames.end(), model3DName) - modelsNames.begin());

		if (current_model != modelsNames.size())
		{
			current_item = 1 + current_model;
		}
		else if (meshes.size() > 0)
		{
			std::string meshName = meshes[0]->name;
			int current_mesh = static_cast<int>(std::find(meshesNames.begin(), meshesNames.end(), meshName) - meshesNames.begin());
			if (current_mesh != meshesNames.size())
			{
				current_item = 1 + static_cast<int>(modelsNames.size()) + current_mesh;
			}
		}

		DrawComboSelection(selectables[current_item], selectables, [this, modelsNames, meshesNames](std::string m)
			{
				if (m == " ")
				{
					renderableUpdateFlags |= RenderableFlags_DestroyMeshes;
				}
				else if (std::find(modelsNames.begin(), modelsNames.end(), m) != modelsNames.end())
				{
					renderableUpdateFlags |= RenderableFlags_RebuildMeshesFromModel3D;
					model3DSwap = m;
				}
				else if (std::find(meshesNames.begin(), meshesNames.end(), m) != meshesNames.end())
				{
					renderableUpdateFlags |= RenderableFlags_RebuildMeshes;
					meshSwap = m;
				}
			}, "model"
		);

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

			for (unsigned int i = 0; i < meshes.size(); i++)
			{
				std::shared_ptr<MeshInstance>& mesh = meshes[i];

				ImGui::TableNextRow();

				bool skip_v = skipMeshes.contains(i);
				ImGui::TableSetColumnIndex(0);
				std::string comboIdSkip = "mesh#" + mesh->name + "#skip";
				ImGui::PushID(comboIdSkip.c_str());
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
				ImGui::PopID();

				ImGui::TableSetColumnIndex(1);
				ImGui::Text(mesh->name.c_str());

				ImGui::TableSetColumnIndex(2);

				std::vector materialList = GetMaterialsNames();
				materialList.insert(materialList.begin(), " ");

				std::string comboId = "mesh#" + mesh->name + "#material";
				ImGui::PushID(comboId.c_str());
				ImGui::SetNextItemWidth(-1);

				std::string materialName = " ";
				if (meshMaterials.contains(mesh))
				{
					std::shared_ptr<MaterialInstance> material = meshMaterials.at(mesh);
					materialName = GetMaterialInstanceTemplateName(material);
				}
				DrawComboSelection(materialName, materialList, [this, mesh](std::string matName)
					{
						materialSwaps.insert_or_assign(mesh, matName);
						renderableUpdateFlags |= RenderableFlags_SwapMaterialsFromMesh;
					}
				);
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}
#endif

	void DestroyRenderable(std::shared_ptr<Renderable>& renderable)
	{
		if (renderable == nullptr) return;
		if (renderables.contains(renderable->name)) renderables.erase(renderable->name);
		if (animables.contains(renderable->name)) animables.erase(renderable->name);
		renderable->this_ptr = nullptr;
		renderable = nullptr;
	}

	//DESTROY
	void DestroyRenderables()
	{
		for (auto& [name, renderable] : renderables)
		{
			DEBUG_PTR_COUNT(renderable);
			renderable->this_ptr = nullptr;
		}
		animables.clear();
		renderables.clear();
	}
}