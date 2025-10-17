#include "pch.h"
#include <ppltasks.h>
#include <vector>
#include <nlohmann/json.hpp>
#include "Renderable.h"
#include <Scene.h>
#include <Camera/Camera.h>
#include <Light/Light.h>
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

#if defined(_EDITOR)
namespace Editor
{
	extern void SelectRenderable(std::shared_ptr<Renderable> renderable);
	extern void BindRenderableToPickingPass(std::shared_ptr<Renderable> r);
	extern void UnbindRenderableFromPickingPass(std::shared_ptr<Renderable> r);
};
#endif

namespace Scene
{

#include <TrackUUID/JDef.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)

#include <Editor/JDrawersDef.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDef.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDef.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDef.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDef.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#endif

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
				if (r->dirty(Renderable::Update_cameras))
				{
					if (r->UpdatePrevValues.contains("cameras"))
					{
						nlohmann::json prevCams = r->UpdatePrevValues.at("cameras");
						std::set<std::string> prevCamUUIDs;
						for (auto& cam : prevCams) {
							if (cam != "") prevCamUUIDs.insert(cam);
						}
						std::vector<std::string> currCams = r->cameras();
						std::set<std::string> currCamUUIDs;
						for (auto& cam : currCams) {
							if (cam != "") currCamUUIDs.insert(cam);
						}
						bool isDifferent = prevCamUUIDs != currCamUUIDs;
						if (!isDifferent)
							r->clean(Renderable::Update_cameras);
						return isDifferent;
					}
					return true;
				}
				return false;
			}
		);

		std::set<std::shared_ptr<Renderable>> todelete;
		std::copy_if(r.begin(), r.end(), std::inserter(todelete, todelete.begin()), [](auto& r)
			{
				return r->markedForDelete;
			}
		);

		bool criticalFrame = meshes.size() > 0ULL || models.size() > 0ULL || bindToCam.size() > 0ULL || todelete.size() > 0ULL;

		if (criticalFrame)
		{
			renderer->Flush();
			renderer->RenderCriticalFrame([&meshes, &models, &bindToCam, &todelete]
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
						EraseRenderableFromAnimables(r);
						nlohmann::json patch = { {"meshMaterials", nlohmann::json::array({})} };
						r->merge_patch(patch);
						r->RebuildMeshMaterials();
						if (r->animable)
						{
							AttachAnimation(r->this_ptr, r->model3D->animations);
							r->StepAnimation(0.0f);
							r->boundingBoxCompute = std::make_shared<RenderableBoundingBox>(r->this_ptr);
						}
						r->BindToScene();
						r->clean(Renderable::Update_model);
					}
					for (auto& r : bindToCam)
					{
						std::set<std::string> currCamsUUIDs;
						std::vector<std::string> cameras = r->cameras();
						for (auto& cam : cameras) {
							if (cam != "") currCamsUUIDs.insert(cam);
						}

						std::set<std::string> prevCamsUUIDs;
						if (r->UpdatePrevValues.contains("cameras"))
						{
							nlohmann::json prevCams = r->UpdatePrevValues.at("cameras");
							for (auto& cam : prevCams) {
								if (cam != "") prevCamsUUIDs.insert(cam);
							}
						}

						//get cams present in the current set, but not present in the last set(this means adding cams)
						std::set<std::string> addCams;
						std::set_difference(
							currCamsUUIDs.begin(), currCamsUUIDs.end(),
							prevCamsUUIDs.begin(), prevCamsUUIDs.end(),
							std::inserter(addCams, addCams.begin())
						);

						//get cams present in the previous set, but not present in the current set(this means deleting cams)
						std::set<std::string> delCams;
						std::set_difference(
							prevCamsUUIDs.begin(), prevCamsUUIDs.end(),
							currCamsUUIDs.begin(), currCamsUUIDs.end(),
							std::inserter(delCams, delCams.begin())
						);

						//remove the camera from the renderable's binded camera set
						for (auto& uuid : delCams)
						{
							std::shared_ptr<Camera> cam = FindInCameras(uuid);
							if (cam)
								Scene::UnbindFromScene(r, cam);
						}
						//add the camera to the renderable's binded camera set
						for (auto& uuid : addCams)
						{
							std::shared_ptr<Camera> cam = FindInCameras(uuid);
							Scene::BindToScene(r, cam);
						}

						r->clean(Renderable::Update_cameras);
					}
					for (auto& r : todelete)
					{
						EraseRenderableFromRenderables(r);
						EraseRenderableFromAnimables(r);
						EraseRenderableFromShadowCasts(r);
						std::shared_ptr<Renderable> renderable = r;
						SafeDeleteSceneObject(renderable);
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
	void DestroyRenderables()
	{
		auto tmp = Renderables;
		for (auto& [_, r] : tmp)
		{
			SafeDeleteSceneObject(r);
		}
#include <TrackUUID/JClear.h>
#include <RenderableAtt.h>
#include <JEnd.h>
	}

	void DeleteRenderable(std::string uuid)
	{
		std::shared_ptr<Renderable> r = FindInRenderables(uuid);
		r->markedForDelete = true;
	}

	//EDITOR
#if defined(_EDITOR)
	void WriteRenderablesJson(nlohmann::json& json)
	{
#include <Editor/JSaveFile.h>
#include <RenderableAtt.h>
#include <JEnd.h>
	}
#endif

	Renderable::Renderable(nlohmann::json json) :SceneObject(json)
	{
#include <Attributes/JInit.h>
#include <RenderableAtt.h>
#include <JEnd.h>

#include <Attributes/JUpdate.h>
#include <RenderableAtt.h>
#include <JEnd.h>

	}

	void Renderable::Initialize()
	{
		CreateMeshInstances(); //why here, this is a special case, as Animables depends of animables which is created in this function

#include <TrackUUID/JInsert.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		if (animable)
		{
			AttachAnimation(this_ptr, model3D->animations);
			if (!animation().empty() && animationPlay())
			{
				SetCurrentAnimation(animation(), 0.0f, animationTimeFactor(), animationPlay(), animationLoop());
			}
			else
			{
				StepAnimation(0.0f); //take an empty T-Pose step so the skinning can be performed
			}
			boundingBoxCompute = std::make_shared<RenderableBoundingBox>(this_ptr);
		}
#if defined(_EDITOR)
		OnPick = [this] { Editor::SelectRenderable(this_ptr); };
#endif
	}

	void Renderable::Bind(std::shared_ptr<SceneObject> sceneObject)
	{
		switch (sceneObject->JType())
		{
		case SO_Cameras:
		{
			std::shared_ptr<Camera> c = std::dynamic_pointer_cast<Camera>(sceneObject);
			bindedCameras.insert(c);
		}
		break;
		}
	}

	void Renderable::Unbind(std::shared_ptr<SceneObject> sceneObject)
	{
		switch (sceneObject->JType())
		{
		case SO_Cameras:
		{
			std::shared_ptr<Camera> c = std::dynamic_pointer_cast<Camera>(sceneObject);
			bindedCameras.erase(c);
		}
		break;
		}
	}

	void Renderable::BindToScene()
	{
#include <TrackUUID/JInsert.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		BindCameras();
	}

	void Renderable::BindCameras()
	{
		auto cams = cameras();
		for (auto& uuid : cams) {
			std::shared_ptr<Camera> cam = FindInCameras(uuid);
			if (cam == nullptr) continue;
			BindCamera(cam);
		}
	}

	void Renderable::BindCamera(std::shared_ptr<Camera>& cam)
	{
		Scene::BindToScene(this_ptr, cam);
	}

	void Renderable::UnbindFromScene()
	{
#include <TrackUUID/JErase.h>
#include <RenderableAtt.h>
#include <JEnd.h>

		UnbindMaterialsChangesCallback();
		UnbindModelChangesCallback();
		Scene::UnbindFromScene(this_ptr);
		boundingBoxCompute = nullptr;
	}

	void Renderable::UnbindCameras()
	{
	}

	void Renderable::UnbindCamera(std::shared_ptr<Camera>& cam)
	{

	}

	void Renderable::BindShadowMapCameras()
	{
		if (!castShadows()) return;
		auto lights = GetShadowMapLights();
		for (auto& light : lights) {
			for (auto& cam : light->shadowMapCameras)
			{
				BindCamera(cam);
			}
		}
	}

	void Renderable::UnbindMaterialsChangesCallback()
	{
		for (auto& [rp, vec0] : materials)
		{
			for (auto& mat : vec0)
			{
				std::shared_ptr<MaterialJson> matJ = GetMaterialTemplate(mat->materialUUID);
				matJ->UnbindChangeCallback(uuid());
			}
		}
	}

	void Renderable::UnbindModelChangesCallback()
	{
		if (!model().empty())
		{
			std::shared_ptr<Model3DJson> mdl = GetModel3DTemplate(model());
			mdl->UnbindChangeCallback(uuid());
		}
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
		else if (model() != "" && GetModel3DTemplate(model()) != nullptr)
		{
			model3D = GetModel3DInstance(model(), [this]
				{
					return std::make_shared<Model3DInstance>(model(), uuid(), [this](std::shared_ptr<JObject> model)
						{
							if (model->dirty(Model3DJson::Update_animationSequences))
							{
								RebuildAnimationSequences();
							}
						}
					);
				}
			);
			meshes = model3D->meshes;
			animable = (model3D->animations) ? model3D : nullptr;
			if (!animable)
				CreateBoundingBox();
			else
				CreateAnimationSequences();
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
		auto onPostMaterialChange = [this](unsigned int index, unsigned int total)
			{
				if (index == 0U)
				{
					renderer->Flush();
					renderer->ResetCommands();
					renderer->SetCSUDescriptorHeap();
				}

				RebuildMeshMaterials();

				if (index >= (total - 1))
				{
					renderer->CloseCommandsAndFlush();
				}
			};

		std::vector<PassMaterialOverride> pmo = passMaterialOverrides();

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
				std::vector<PassMaterialOverride> mpmo;
				std::copy_if(pmo.begin(), pmo.end(), std::back_inserter(mpmo), [i](PassMaterialOverride& o)
					{
						return o.meshIndex == i;
					}
				);
				auto& mesh = meshes.at(i);
				std::string matUUID = mm.at(i).materialUUID;
				if (GetMaterialTemplate(matUUID) == nullptr)
				{
					matUUID = FindMaterialUUIDByName("BaseLighting"); //fallback
				}
				std::shared_ptr<MaterialInstance> mi = rp->GetRenderPassMaterialInstance(
					matUUID, mesh, shadowed(),
					mpmo, uuid(), nullptr, onPostMaterialChange);
				materials[rp].push_back(mi);
			}
		}
		else if (model() != "" && model3D)
		{
			for (unsigned int i = 0; i < meshes.size(); i++)
			{
				std::vector<PassMaterialOverride> mpmo;
				std::copy_if(pmo.begin(), pmo.end(), std::back_inserter(mpmo), [i](PassMaterialOverride& o)
					{
						return o.meshIndex == i;
					}
				);
				auto& mesh = meshes.at(i);
				std::string matUUID = model3D->materialUUIDs.at(i);
				std::shared_ptr<MaterialInstance> mi = rp->GetRenderPassMaterialInstance(matUUID, mesh, shadowed(),
					mpmo, uuid(), nullptr, onPostMaterialChange);
				materials[rp].push_back(mi);
			}
		}
	}

	void Renderable::DestroyRenderPassMaterialsInstances(std::shared_ptr<Templates::RenderPassInstance>& rp)
	{
		if (!materials.contains(rp)) return;
		for (auto& m : materials.at(rp))
		{
			RemoveMaterialInstance(m->instanceUUID, m);
			m = nullptr;
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
		auto rtFormats = rp->GetRenderTargetsFormats();
		auto depthFormat = rp->GetDepthStencilFormat();

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
#if defined(_EDITOR)
		Editor::UnbindRenderableFromPickingPass(this_ptr);
#endif
		Destroy();
		try
		{
			CreateMeshInstances();
			for (auto& cam : bindedCameras)
			{
				CreateMaterialsInstances(cam);
				CreateConstantsBuffersInstances(cam);
				CreateRootSignatures(cam);
				CreatePipelineStates(cam);
			}
#if defined(_EDITOR)
			Editor::BindRenderableToPickingPass(this_ptr);
#endif
		}
		catch (...)
		{
			renderException = true;
		}
	}

	void Renderable::CreateAnimationSequences()
	{
		std::shared_ptr<Model3DJson> mdl = GetModel3DTemplate(model());
		animationsSequences = mdl->animationSequences();
		for (auto& [name, _] : animable->animations->animationsLength)
		{
			int totalFrames = static_cast<int>(animable->animations->animationsLength.at(name) * 60);
			totalFrames /= 1000;
			animationsSequences.sequences.insert_or_assign(
				name,
				Sequence(
					name,
					totalFrames
				)
			);
		}
	}

	void Renderable::RebuildAnimationSequences()
	{
		if (currentSequence == nullptr)
			return CreateAnimationSequences();

		std::string currentSequenceStr;
		for (auto it = animationsSequences.sequences.begin(); it != animationsSequences.sequences.end(); it++)
		{
			if (currentSequence == &(it->second))
			{
				currentSequenceStr = it->first;
				break;
			}
		}

		CreateAnimationSequences();
		if (currentSequenceStr.empty()) return;

		for (auto it = animationsSequences.sequences.begin(); it != animationsSequences.sequences.end(); it++)
		{
			if (currentSequenceStr != it->first) continue;
			currentSequence = &(it->second);
			return;
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

	BoundingBox Renderable::GetBoundingBox()
	{
		BoundingBox& bb = animable ? boundingBoxCompute->boundingBox : boundingBox;
		BoundingBox bbw;
		bb.Transform(bbw, world());
		return bbw;
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

	void Renderable::SetCurrentAnimation(Sequence* sequence, float startTime, float timeFactor, bool play, bool loop)
	{
		currentSequence = sequence;
		animationTime(startTime);
		animationTimeFactor(timeFactor);
		animationPlay(play);
		animationLoop(loop);
		animationFrame(0);
		animation("");
	}

	void Renderable::SetCurrentAnimation(std::string anim, float startTime, float timeFactor, bool play, bool loop)
	{
		auto it = animationsSequences.sequences.find(anim);
		if (it == animationsSequences.sequences.end()) return;
		SetCurrentAnimation(&(it->second), startTime, timeFactor, play, loop);
	}

	void Renderable::StepAnimation(double elapsedSeconds)
	{
		using namespace Animation;
		//no animation? no problem. just go T pose
		if (!currentSequence)
		{
			TraverseMultiplycationQueue(0.0f, "", animable->animations, bonesTransformation);
			animation("");
			animationFrame(0);
			return;
		}

		float totalTime = 1000.0f * static_cast<float>(currentSequence->totalFrames) / static_cast<float>(currentSequence->framesPerSecond);
		float currentAnimationTime = animationTime();
		currentAnimationTime += animationPlay() ? animationTimeFactor() * static_cast<float>(elapsedSeconds) * 1000.0f : 0.0f;
		int currentFrame = static_cast<int>(static_cast<float>(currentSequence->framesPerSecond) * currentAnimationTime / 1000.0f);

		//handle end of animation
		if (currentFrame >= currentSequence->totalFrames)
		{
			//are we looping?
			if (animationLoop())
			{
				currentAnimationTime = fmodf(currentAnimationTime, totalTime);
				currentFrame = static_cast<int>(static_cast<float>(currentSequence->framesPerSecond) * currentAnimationTime / 1000.0f);
			}
			else
			{
				currentAnimationTime = totalTime;
				currentFrame = currentSequence->totalFrames - 1;
			}
		}
		animationTime(currentAnimationTime);

		std::vector<SequenceChannel> channels;
		std::copy_if(currentSequence->channels.begin(), currentSequence->channels.end(), std::back_inserter(channels), [currentFrame](SequenceChannel& ch)
			{
				return currentFrame >= ch.frameStart && currentFrame <= ch.frameEnd;
			}
		);

		//no animations? fallback to T-Pose, yes!
		if (channels.size() == 0ULL)
		{
			TraverseMultiplycationQueue(0.0f, "", animable->animations, bonesTransformation);
			animation("");
			animationFrame(0);
			return;
		}

		//now here is the tricky part, pick the last animation in the hierarchy
		SequenceChannel& last = channels.back();
		float animationLength = animable->animations->animationsLength[last.animation];
		float time = animationLength * static_cast<float>(currentFrame - last.frameStart) / static_cast<float>(last.frameEnd - last.frameStart - 1);
		animation(last.animation);
		animationFrame(currentFrame);
		TraverseMultiplycationQueue(time, last.animation, animable->animations, bonesTransformation);
	}

	int Renderable::GetCurrentAnimationFrame()
	{
		return static_cast<int>(static_cast<float>(currentSequence->framesPerSecond) * animationTime() / 1000.0f);
	}

	int Renderable::GetCurrentAnimationNumFrames() const
	{
		return currentSequence->totalFrames;
	}

	void Renderable::SetCurrentAnimationFrame(int frame)
	{
		float time = 1000.0f * static_cast<float>(frame) / static_cast<float>(currentSequence->framesPerSecond);
		animationTime(time);
	}

	bool Renderable::AnimationEnded()
	{
		if (!animationPlay()) return false;
		if (animationLoop()) return false;
		if (!currentSequence) return false;
		return (GetCurrentAnimationNumFrames() - GetCurrentAnimationFrame()) <= 1;
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

		if (model3D == nullptr)
		{
			destroyMeshInstance(meshes);
		}
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

		if (!visible() || !materials.contains(renderPass) || renderException) return;

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
