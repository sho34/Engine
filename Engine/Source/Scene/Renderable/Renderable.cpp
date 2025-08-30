#include "pch.h"
#include <ppltasks.h>
#include <vector>
#include <nlohmann/json.hpp>
#include "Renderable.h"
#include <Scene.h>
#include <Camera/Camera.h>
#include <Lights/Lights.h>
#include <d3dx12.h>
#include <DirectXHelper.h>
#include <Model3D/Model3D.h>
#include <Mesh/Mesh.h>
#include <Material/Material.h>
#include <Shader/Shader.h>
#include <Animated.h>
#include <Renderer.h>
#include <DeviceUtils/D3D12Device/Builder.h>
#include <DeviceUtils/D3D12Device/Interop.h>
#include <DeviceUtils/RootSignature/RootSignature.h>
#include <DeviceUtils/PipelineState/PipelineState.h>
#include <DeviceUtils/Resources/Resources.h>
#if defined(_EDITOR)
#include <Level.h>
#include <Effects.h>
#include <imgui.h>
#endif
#include <Templates.h>
#include <NoStd.h>
#include <Json.h>

extern std::shared_ptr<Renderer> renderer;

namespace Scene {
#include <JExposeAttDrawersDef.h>
#include <RenderableAtt.h>
#include <JExposeEnd.h>

#include <JExposeTrackUUIDDef.h>
#include <RenderableAtt.h>
#include <JExposeEnd.h>

	//UPDATE
	void RenderablesStep()
	{
#if defined(_EDITOR)
		std::set<std::shared_ptr<Renderable>> r;
		std::transform(Renderables.begin(), Renderables.end(), std::inserter(r, r.begin()), [](const auto& pair) { return pair.second; });

		std::set<std::shared_ptr<Renderable>> meshes;
		std::copy_if(r.begin(), r.end(), std::inserter(meshes, meshes.begin()), [](auto& r)
			{
				return (r->dirty(Renderable::Update_meshMaterials));
			}
		);

		std::set<std::shared_ptr<Renderable>> models;
		std::copy_if(r.begin(), r.end(), std::inserter(models, models.begin()), [](auto& r)
			{
				return (r->dirty(Renderable::Update_model));
			}
		);

		std::set<std::shared_ptr<Renderable>> bindToCam;
		std::copy_if(r.begin(), r.end(), std::inserter(bindToCam, bindToCam.end()), [](auto& r)
			{
				return r->dirty(Renderable::Update_cameras);
			}
		);

		bool criticalFrame = meshes.size() > 0ULL || models.size() > 0ULL || bindToCam.size() > 0ULL;

		if (criticalFrame)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([&meshes, &models, &bindToCam]
				{
					for (auto& r : meshes)
					{
						nlohmann::json patch = { {"model",""} };
						r->merge_patch(patch);
						r->RebuildMeshMaterials();
						r->BindToScene();
						r->clean(Renderable::Update_meshMaterials);
					}
					for (auto& r : models)
					{
						nlohmann::json patch = { {"meshMaterials", nlohmann::json::array({})} };
						r->merge_patch(patch);
						r->RebuildMeshMaterials();
						r->BindToScene();
						r->clean(Renderable::Update_model);
					}
					for (auto& r : bindToCam)
					{
						for (auto uuid : r->bindedCameras)
						{
							std::shared_ptr<Camera> cam = FindInCameras(uuid);
							if (cam)
								cam->UnbindRenderable(r);
						}
						auto camsV = r->cameras();
						r->bindedCameras = std::set<std::string>(camsV.begin(), camsV.end());
						for (auto uuid : r->bindedCameras)
						{
							std::shared_ptr<Camera> cam = FindInCameras(uuid);
							if (cam)
								cam->BindRenderable(r);
						}
						r->clean(Renderable::Update_cameras);
					}
				}
			);
		}

#endif
	}

	void DestroyRenderablesCameraBinding()
	{
	}

	void RunBoundingBoxComputeShaders()
	{
		for (auto& [name, renderable] : Renderables)
		{
			if (renderable->boundingBoxCompute)
			{
				renderable->boundingBoxCompute->Compute();
			}
		}
	}

	void RunBoundingBoxComputeShadersSolution()
	{
		for (auto& [name, renderable] : Renderables)
		{
			if (renderable->boundingBoxCompute)
			{
				renderable->boundingBoxCompute->Solution();
			}
		}
	}

	//DESTROY
	void DestroyRenderable(std::shared_ptr<Renderable>& renderable)
	{
		if (renderable == nullptr) return;
		DEBUG_PTR_COUNT_JSON(renderable);

		renderable->UnbindFromScene();
		renderable->this_ptr = nullptr;
		renderable = nullptr;
	}

	void DestroyRenderables()
	{
		auto tmp = Renderables;
		for (auto& [_, r] : tmp)
		{
			DestroyRenderable(r);
		}
		Animables.clear();
		Renderables.clear();
	}

	//EDITOR
#if defined(_EDITOR)
	void WriteRenderablesJson(nlohmann::json& json)
	{
		//std::map<std::string, std::shared_ptr<Renderable>> filtered;
		//std::copy_if(renderables.begin(), renderables.end(), std::inserter(filtered, filtered.end()), [](const auto& pair)
		//	{
		//		auto& [uuid, renderable] = pair;
		//		return !renderable->hidden();
		//	}
		//);
		//std::transform(filtered.begin(), filtered.end(), std::back_inserter(json), [](const auto& pair)
		//	{
		//		auto& [uuid, renderable] = pair;
		//		nlohmann::json ret = renderable->json;
		//		ret["uuid"] = uuid;
		//		ret["skipMeshes"] = nlohmann::json::array();
		//		for (auto skip : renderable->skipMeshes())
		//		{
		//			ret["skipMeshes"].push_back(skip);
		//		}
		//		return ret;
		//	}
		//);
	}
#endif

	void Renderable::CreateFromModel3D(std::string model3DUUID)
	{
		/*
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
			boundingBoxCompute = std::make_shared<RenderableBoundingBox>(this_ptr);
			using namespace ComputeShader;
		}

#if defined(_EDITOR)
		BindNotifications(model3DUUID, this_ptr);
#endif
*/
	}

	Renderable::Renderable(nlohmann::json json) :SceneObject(json)
	{
		using namespace Animation;
		using namespace Templates;

		assert(!name().empty() && !uuid().empty());

#include <JExposeInit.h>
#include <RenderableAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttUpdate.h>
#include <RenderableAtt.h>
#include <JExposeEnd.h>

		//create_castShadows(true);
		//create_shadowed(true);
		//create_hidden(false);
		//create_ibl(false);
		//create_meshMaterials({});
		//create_model("");
		//create_name("846455$NAME");
		//create_position({ 0.0f,0.0f,0.0f });
		//create_rotation({ 0.0f,0.0f,0.0f });
		//create_scale({ 1.0f,1.0f,1.0f });
		//create_skipMeshes({});
		//create_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//create_uniqueMaterialInstance(false);
		//create_uuid("846455$UUID");
		//create_visible(true);
		//create_cameras({});
		//r->create_pipelineState(nlohmann::json::object());

		CreateMeshInstances();
	}

	void Renderable::BindToScene()
	{
		Renderables.insert_or_assign(uuid(), this_ptr);
		if (animable)
		{
			AttachAnimation(this_ptr, model3D->animations);
			Animables.insert_or_assign(uuid(), this_ptr);
			StepAnimation(0.0f);
			boundingBoxCompute = std::make_shared<RenderableBoundingBox>(this_ptr);
		}
	}

	void Renderable::UnbindFromScene()
	{
		Renderables.erase(uuid());
		Animables.erase(uuid());
		boundingBoxCompute = nullptr;
		UnbindCameras();
	}

	XMVECTOR Renderable::rotationQ()
	{
		XMFLOAT3 rotV = rotation();
		float roll, pitch, yaw;
		pitch = rotV.x; yaw = rotV.y; roll = rotV.z;
		XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
		return rotQ;
	}

	XMMATRIX Renderable::world()
	{
		XMFLOAT3 posV = position();
		XMFLOAT3 scaleV = scale();
		XMMATRIX rotationM = XMMatrixRotationQuaternion(rotationQ());
		XMMATRIX scaleM = XMMatrixScalingFromVector({ scaleV.x, scaleV.y, scaleV.z });
		XMMATRIX positionM = XMMatrixTranslationFromVector({ posV.x, posV.y, posV.z });
		return XMMatrixMultiply(XMMatrixMultiply(scaleM, rotationM), positionM);
	}

	void Renderable::UnbindCameras()
	{
		for (auto uuid : bindedCameras)
		{
			std::shared_ptr<Camera> cam = FindInCameras(uuid);
			if (!cam) continue;
			cam->UnbindRenderable(this_ptr);
		}
	}

	void Renderable::CreateMeshInstances()
	{
		if (!meshMaterials().empty())
		{
			std::vector<MeshMaterial> rmm = meshMaterials();
			std::vector<MeshMaterial> mm;
			std::copy_if(rmm.begin(), rmm.end(), std::back_inserter(mm), [](const MeshMaterial& mm)
				{
					return mm.mesh != "" && mm.materialUUID != "";
				}
			);

			std::transform(mm.begin(), mm.end(), std::back_inserter(meshes), [](const MeshMaterial& m)
				{
					return GetMeshInstance(m.mesh);
				}
			);
			CreateBoundingBox();
		}
		else if (model() != "")
		{
			model3D = GetModel3DInstance(model(), [this]
				{
					return std::make_shared<Model3DInstance>(model());
				}
			);
			meshes = model3D->meshes;
			animable = (model3D->animations) ? model3D : nullptr;
		}
	}

	std::vector<std::shared_ptr<RenderPassInstance>> Renderable::GetCameraRenderPasses(std::shared_ptr<Camera> cam)
	{
		if (!cam->useSwapChain()) return cam->cameraRenderPasses;

		std::vector<std::shared_ptr<RenderPassInstance>> rpiv = cam->cameraRenderPasses;
		rpiv.push_back(renderer->swapChainPass);
		return rpiv;
	}

	void Renderable::CreateMaterialsInstances(std::shared_ptr<Camera> cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			CreateRenderPassMaterialsInstances(rp);
		}
	}

	void Renderable::DestroyMaterialsInstances(std::shared_ptr<Camera> cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassMaterialsInstances(rp);
		}
	}

	void Renderable::CreateRenderPassMaterialsInstances(std::shared_ptr<Templates::RenderPassInstance>& rp)
	{
		if (!meshMaterials().empty())
		{
			std::vector<MeshMaterial> rmm = meshMaterials();
			std::vector<MeshMaterial> mm;
			std::copy_if(rmm.begin(), rmm.end(), std::back_inserter(mm), [](const MeshMaterial& mm)
				{
					return mm.mesh != "" && mm.materialUUID != "";
				}
			);

			for (unsigned i = 0; i < mm.size(); i++)
			{
				auto& mesh = meshes.at(i);
				std::string matUUID = mm.at(i).materialUUID;
				std::shared_ptr<MaterialInstance> mi = rp->GetRenderPassMaterialInstance(matUUID, mesh, shadowed());
				materials[rp].push_back(mi);
			}
		}
		else if (model() != "" && model3D)
		{
			for (unsigned int i = 0; i < meshes.size(); i++)
			{
				auto& mesh = meshes.at(i);
				std::string matUUID = model3D->materialUUIDs.at(i);
				std::shared_ptr<MaterialInstance> mi = rp->GetRenderPassMaterialInstance(matUUID, mesh, shadowed());
				materials[rp].push_back(mi);
			}
		}
	}

	void Renderable::DestroyRenderPassMaterialsInstances(std::shared_ptr<Templates::RenderPassInstance>& rp)
	{
		if (!materials.contains(rp)) return;
		for (auto& m : materials.at(rp))
		{
			DestroyMaterialInstance(m);
		}
		materials.erase(rp);
	}

	void Renderable::CreateConstantsBuffersInstances(std::shared_ptr<Camera> cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			CreateRenderPassConstantsBuffersInstances(rp);
		}
	}

	void Renderable::DestroyConstantsBuffersInstances(std::shared_ptr<Camera> cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassConstantsBuffersInstances(rp);
		}
	}

	void Renderable::CreateRenderPassConstantsBuffersInstances(std::shared_ptr<Templates::RenderPassInstance>& rp)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			constantsBuffers[rp].push_back({});
			auto& mi = materials[rp].at(i);
			auto& mesh = meshes.at(i);
			for (unsigned int j = 0; j < mi->variablesBufferSize.size(); j++)
			{
				size_t size = mi->variablesBufferSize[j];
				std::shared_ptr<ConstantsBuffer> cbuffer = CreateConstantsBuffer(size, name() + "." + std::to_string(j) + "." + mesh->uuid);
				for (unsigned int n = 0; n < renderer->numFrames; n++)
				{
					WriteMaterialVariablesToConstantsBufferSpace(mi, cbuffer, n);
				}
				constantsBuffers[rp][i].push_back(cbuffer);
			}
		}
	}

	void Renderable::DestroyRenderPassConstantsBuffersInstances(std::shared_ptr<Templates::RenderPassInstance>& rp)
	{
		if (!constantsBuffers.contains(rp)) return;

		for (unsigned int i = 0; i < constantsBuffers.at(rp).size(); i++)
		{
			for (unsigned int c = 0; c < constantsBuffers.at(rp).at(i).size(); c++)
			{
				DestroyConstantsBuffer(constantsBuffers.at(rp).at(i)[c]);
			}
		}
		constantsBuffers.erase(rp);
	}

	void Renderable::CreateRootSignatures(std::shared_ptr<Camera> cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			if (rootSignatures.contains(rp)) rootSignatures.erase(rp);
			CreateRenderPassRootSignatures(rp);
		}
	}

	void Renderable::DestroyRootSignatures(std::shared_ptr<Camera> cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassRootSignatures(rp);
		}
	}

	void Renderable::CreateRenderPassRootSignatures(std::shared_ptr<Templates::RenderPassInstance>& rp)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			auto& mi = materials[rp].at(i);

			auto& vsCBparams = mi->vertexShader->constantsBuffersParameters;
			auto& psCBparams = mi->pixelShader->constantsBuffersParameters;
			auto& uavParams = mi->pixelShader->uavParameters;
			auto& psSRVCSparams = mi->pixelShader->srvCSParameters;
			auto& psSRVTexparams = mi->pixelShader->srvTexParameters;
			auto& psSamplersParams = mi->pixelShader->samplersParameters;
			auto& samplers = mi->samplers;

			std::string rsName = "rootSignature:" + name() + ":" + std::to_string(i);
			rootSignatures[rp].push_back(
				CreateRootSignature(rsName, vsCBparams, psCBparams, uavParams, psSRVCSparams, psSRVTexparams, psSamplersParams, samplers)
			);
		}
	}

	void Renderable::DestroyRenderPassRootSignatures(std::shared_ptr<Templates::RenderPassInstance>& rp)
	{
		rootSignatures.erase(rp);
	}

	void Renderable::CreatePipelineStates(std::shared_ptr<Camera> cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			if (pipelineStates.contains(rp)) pipelineStates.erase(rp);
			CreateRenderPassPipelineStates(rp);
		}
	}

	void Renderable::DestroyPipelineStates(std::shared_ptr<Camera> cam)
	{
		for (auto& rp : GetCameraRenderPasses(cam))
		{
			DestroyRenderPassPipelineStates(rp);
		}
	}

	void Renderable::CreateRenderPassPipelineStates(std::shared_ptr<Templates::RenderPassInstance>& rp)
	{
		std::shared_ptr<RenderPassJson> renderPass = GetRenderPassTemplate(rp->renderPassUUID);
		auto rtFormats = renderPass->renderTargetFormats();
		auto depthFormat = renderPass->depthStencilFormat();

		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			auto& mesh = meshes.at(i);
			auto& mi = materials[rp].at(i);
			auto& vsLayout = vertexInputLayoutsMap[mesh->vertexClass];
			auto& rootSignature = rootSignatures[rp].at(i);
			auto& vsByteCode = mi->vertexShader->byteCode;
			auto& psByteCode = mi->pixelShader->byteCode;

			std::shared_ptr<MaterialJson> material = GetMaterialTemplate(mi->materialUUID);
			BlendDesc blendDesc = material->blendState();
			RasterizerDesc rasterizerDesc = material->rasterizerState();

			D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType = D3D_PRIMITIVE_TOPOLOGYToD3D12_PRIMITIVE_TOPOLOGY_TYPE.at(topology());

			std::string plName = "pipelineState:" + name() + ":" + std::to_string(i);
			pipelineStates[rp].push_back(
				CreateGraphicsPipelineState(plName, vsLayout, vsByteCode, psByteCode, rootSignature, blendDesc, rasterizerDesc, primitiveTopologyType, rtFormats, depthFormat)
			);
		}
	}

	void Renderable::DestroyRenderPassPipelineStates(std::shared_ptr<Templates::RenderPassInstance>& rp)
	{
		pipelineStates.erase(rp);
	}

	void Renderable::RebuildMeshMaterials()
	{
		renderException = false;
		Destroy();
		try
		{
			CreateMeshInstances();
			for (std::string camUUID : bindedCameras)
			{
				std::shared_ptr<Camera> cam = FindInCameras(camUUID);
				CreateMaterialsInstances(cam);
				CreateConstantsBuffersInstances(cam);
				CreateRootSignatures(cam);
				CreatePipelineStates(cam);
			}
		}
		catch (...)
		{
			renderException = true;
		}
	}

	void Renderable::CreateBoundingBox()
	{
		bool extend = false;
		for (auto& mesh : meshes)
		{
			mesh->ExtendBoundingBox(boundingBox, extend);
			extend = true;
		}
	}

	void Renderable::FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox)
	{
		BoundingBox bb = animable ? boundingBoxCompute->boundingBox : boundingBox;
		XMVECTOR center = { bb.Center.x, bb.Center.y, bb.Center.z, 1.0f };
		XMVECTOR bbpos = XMVector3Transform(center, world());

		bbox->scale(bb.Extents * scale());
		bbox->rotation(rotation());
		bbox->position(*((XMFLOAT3*)bbpos.m128_f32));
	}

	BoundingBox Renderable::GetBoundingBox()
	{
		BoundingBox& bb = animable ? boundingBoxCompute->boundingBox : boundingBox;
		BoundingBox bbw;
		bb.Transform(bbw, world());
		return bbw;
	}

	void Renderable::ReloadModel3D()
	{
		//renderableUpdateFlags |= RenderableFlags_RebuildMeshesFromModel3D;
		//model3DSwap = json.at("model");
	}

	void Renderable::WriteMaterialVariablesToConstantsBufferSpace(std::shared_ptr<MaterialInstance>& material, std::shared_ptr<ConstantsBuffer>& cbvData, unsigned int cbvFrameIndex)
	{
		for (auto& [varName, varMapping] : material->variablesMapping)
		{
			UINT8* source = material->variablesBuffer[varMapping.mapping.bufferIndex].data() + varMapping.mapping.offset;
			UINT8* destination = cbvData->mappedConstantBuffer + (cbvFrameIndex * cbvData->alignedConstantBufferSize) + varMapping.mapping.offset;
			memcpy(destination, source, varMapping.mapping.size);
		}
	}

	void Renderable::WriteAnimationConstantsBuffer(unsigned int backbufferIndex)
	{
		if (!animable) return;

		using namespace Animation;
		WriteBoneTransformationsToConstantsBuffer(this_ptr, bonesTransformation, backbufferIndex);
	}

	void Renderable::WriteConstantsBuffer(unsigned int backbufferIndex)
	{
		XMMATRIX w = world();
		WriteConstantsBuffer("world", w, backbufferIndex);
	}

	void Renderable::SetCurrentAnimation(std::string anim, float startTime, float timeFactor, bool play, bool loop)
	{
		animation(anim);
		animationTime(startTime);
		animationTimeFactor(timeFactor);
		animationPlay(play);
		animationLoop(loop);
	}

	void Renderable::StepAnimation(double elapsedSeconds)
	{
		float animationLength = animable->animations->animationsLength[animation()];
		float currentAnimationTime = animationTime();
		if (animationLength > 0.0f)
		{
			currentAnimationTime += animationPlay() ? animationTimeFactor() * static_cast<float>(elapsedSeconds) * 1000.0f : 0.0f;

			if (animationTimeFactor() > 0.0f)
			{
				if (currentAnimationTime >= animationLength)
					currentAnimationTime = animationLoop() ? fmodf(currentAnimationTime, animationLength) : animationLength;
			}
			else if (animationTimeFactor() < 0.0f)
			{
				if (currentAnimationTime < 0.0f)
					currentAnimationTime = animationLoop() ? (animationLength - fmodf(currentAnimationTime, animationLength)) : 0.0f;
			}
			animationTime(currentAnimationTime);
		}

		using namespace Animation;
		TraverseMultiplycationQueue(currentAnimationTime, animation(), animable->animations, bonesTransformation);
	}

	//DESTROY
	void Renderable::Destroy()
	{
		for (auto& [rp, vec0] : materials)
		{
			for (auto& mat : vec0)
			{
				RemoveMaterialInstance(mat->instanceUUID, mat);
			}
		}
		materials.clear();

		for (auto& [rp, vec0] : constantsBuffers)
		{
			for (auto& vec1 : vec0)
			{
				for (auto& cbuffer : vec1)
				{
					DestroyConstantsBuffer(cbuffer);
				}
			}
		}
		constantsBuffers.clear();

		auto destroyMeshInstance = [](auto& vec) { for (auto& mesh : vec) { DestroyMeshInstance(mesh); } };

		destroyMeshInstance(meshes);
		meshes.clear();

		if (model3D)
		{
			DestroyModel3DInstance(model3D);
			model3D = nullptr;
			animable = nullptr;
			boundingBoxCompute = nullptr;
		}
	}

	//RENDER
	void Renderable::Render(std::shared_ptr<RenderPassInstance> renderPass, std::shared_ptr<Camera> camera)
	{
		using namespace Animation;

		if (!visible() || renderException) return;

		auto& commandList = renderer->commandList;

#if defined(_DEVELOPMENT)
		PIXBeginEvent(commandList.p, 0, name().c_str());
#endif
		auto& meshesMaterials = materials.at(renderPass);
		auto& meshesRootSignatures = rootSignatures.at(renderPass);
		auto& meshesPipelineStates = pipelineStates.at(renderPass);

		auto setConstantsBuffersDescriptorTables = [&commandList](auto& cbuffers, unsigned int& slot)
			{
				for (auto& cbuffer : cbuffers) {
					cbuffer->SetRootDescriptorTable(commandList, slot, renderer->backBufferIndex);
				}
			};
		auto setCameraConstantsBufferDescriptorTable = [&commandList, &camera](auto& material, unsigned int& slot)
			{
				if (camera && material->ShaderInstanceHasRegister([](auto& binary) { return binary->CBV.camera; })) {
					camera->cameraCbv->SetRootDescriptorTable(commandList, slot, renderer->backBufferIndex);
				}
			};
		auto setLightsConstantsBufferDescriptorTable = [&commandList](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->CBV.light; })) {
					GetLightsConstantsBuffer()->SetRootDescriptorTable(commandList, slot, renderer->backBufferIndex);
				}
			};
		auto setShadowMapsConstantsBufferDescriptorTable = [&commandList](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->CBV.lightsShadowMap; })) {
					if (SceneHasShadowMaps())
						return GetShadowMapConstantsBuffer()->SetRootDescriptorTable(commandList, slot, renderer->backBufferIndex);
					slot++;
				}
			};
		auto setSkinningConstantsBufferDescriptorTable = [&commandList, this](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([this](auto& binary) { return binary->CBV.animation; })) {
					if (animable)
						return GetAnimatedConstantsBuffer(this_ptr)->SetRootDescriptorTable(commandList, slot, renderer->backBufferIndex);
					slot++;
				}
			};
		auto setUAVRootDescriptorTable = [&commandList](auto& material, unsigned int& slot)
			{
				material->SetUAVRootDescriptorTable(commandList, slot);
			};
		auto setIBLRootDescriptorTable = [&commandList, &camera](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([](auto& binary) { return
					(binary->SRV.iblIrradiance == -1 || binary->SRV.iblPrefiteredEnv == -1 || binary->SRV.iblBRDFLUT == -1) ? -1 : 1; })
					)
				{
					camera->SetIBLRootDescriptorTables(commandList, slot);
				}
			};
		auto setSRVRootDescriptorTable = [&commandList](auto& material, unsigned int& slot)
			{
				material->SetSRVRootDescriptorTable(commandList, slot);
			};
		auto setShadowMapsSRVDescriptorTable = [&commandList](auto& material, unsigned int& slot)
			{
				if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->SRV.lightsShadowMap; })) {
					if (SceneHasShadowMaps())
						return commandList->SetGraphicsRootDescriptorTable(slot, GetShadowMapGpuDescriptorHandleStart());
					slot++;
				}
			};

		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			if (skipMeshes_contains(i)) continue;

			commandList->IASetPrimitiveTopology(topology());
			commandList->SetGraphicsRootSignature(meshesRootSignatures.at(i));
			commandList->SetPipelineState(meshesPipelineStates.at(i));

			auto& material = meshesMaterials.at(i);
			auto& cbuffers = constantsBuffers.at(renderPass).at(i);
			unsigned int slot = 0U;

			setConstantsBuffersDescriptorTables(cbuffers, slot);
			setCameraConstantsBufferDescriptorTable(material, slot);
			setLightsConstantsBufferDescriptorTable(material, slot);
			setShadowMapsConstantsBufferDescriptorTable(material, slot);
			setSkinningConstantsBufferDescriptorTable(material, slot);
			setUAVRootDescriptorTable(material, slot);
			setIBLRootDescriptorTable(material, slot);
			setSRVRootDescriptorTable(material, slot);
			setShadowMapsSRVDescriptorTable(material, slot);

			/*
			auto& mesh = meshes.at(i);
			auto& material = meshMaterials.at(mesh);
			auto& rootSignature = std::get<1>(meshHashedRootSignatures.at(mesh));
			if (!meshHashedPipelineStates.contains(passHash) || !meshHashedPipelineStates.at(passHash).contains(mesh))
			{
				CreateMeshPipelineState(passHash, mesh);
			}
			auto& pipelineState = std::get<1>(meshHashedPipelineStates.at(passHash).at(mesh));

			commandList->IASetPrimitiveTopology(topology());
			commandList->SetGraphicsRootSignature(rootSignature);
			commandList->SetPipelineState(pipelineState);
			*/

			/*
			unsigned int cbvSlot = 0U;

			if (meshConstantsBuffer.contains(mesh))
			{
				auto& constantsBuffer = meshConstantsBuffer.at(mesh);
				for (unsigned int i = 0U; i < constantsBuffer.size(); i++) {
					constantsBuffer[i]->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
				}
			}
			*/

			/*
			if (camera && material->ShaderInstanceHasRegister([](auto& binary) { return binary->cameraCBVRegister; })) {
				camera->cameraCbv->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
			}
			*/

			/*
			if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->lightCBVRegister; })) {
				GetLightsConstantsBuffer()->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
			}
			*/

			/*
			if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->lightsShadowMapCBVRegister; })) {
				if (SceneHasShadowMaps()) {
					GetShadowMapConstantsBuffer()->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
				}
				else
				{
					cbvSlot++;
				}
			}
			*/

			/*
			if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->animationCBVRegister; })) {
				GetAnimatedConstantsBuffer(this_ptr)->SetRootDescriptorTable(commandList, cbvSlot, renderer->backBufferIndex);
			}
			*/

			/*
			material->SetUAVRootDescriptorTable(commandList, cbvSlot);
			*/

			/*
			if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->iblIrradianceSRVRegister; })
				&& material->ShaderInstanceHasRegister([](auto& binary) { return binary->iblPrefiteredEnvSRVRegister; })
				&& material->ShaderInstanceHasRegister([](auto& binary) { return binary->iblBRDFLUTSRVRegister; })
				)
			{
				camera->SetIBLRootDescriptorTables(commandList, cbvSlot);
			}
			*/

			/*
			material->SetSRVRootDescriptorTable(commandList, cbvSlot);
			*/

			/*
			if (material->ShaderInstanceHasRegister([](auto& binary) { return binary->lightsShadowMapSRVRegister; })) {
				if (SceneHasShadowMaps()) {
					commandList->SetGraphicsRootDescriptorTable(cbvSlot, GetShadowMapGpuDescriptorHandleStart());
				}
				cbvSlot++;
			}
			*/

			auto& mesh = meshes.at(i);
			commandList->IASetVertexBuffers(0, 1, &mesh->vbvData.vertexBufferView);
			commandList->IASetIndexBuffer(&mesh->ibvData.indexBufferView);
			commandList->DrawIndexedInstanced(mesh->ibvData.indexBufferView.SizeInBytes / sizeof(unsigned int), 1, 0, 0, 0);
		}
#if defined(_DEVELOPMENT)
		PIXEndEvent(commandList.p);
#endif
	}
}
