#pragma once
#include <atlbase.h>
#include <nlohmann/json.hpp>
#include <set>
#include <DirectXCollision.h>
#include <Renderable/RenderableBoundingBox.h>
#include <Model3D/Model3D.h>
#include <Mesh/Mesh.h>
#include <Material/Material.h>
#include <Material/MeshMaterial.h>
#include <RenderPass/RenderPass.h>
#include <Animated.h>
#include <DeviceUtils/PipelineState/PipelineState.h>
#include <Json.h>
#include <SceneObjectDecl.h>
#include <NoMath.h>
#include <SceneObject.h>
#include <JExposeTypes.h>

namespace Scene { struct Camera; struct Light; };
namespace ComputeShader { struct RenderableBoundingBox; };
using namespace Templates;
using namespace DeviceUtils;
using namespace ComputeShader;

typedef std::vector<std::shared_ptr<MeshInstance>> RenderableMeshes;
typedef std::map<std::shared_ptr<RenderPassInstance>, std::vector<std::shared_ptr<MaterialInstance>>> RenderableMaterials;
typedef std::map<std::shared_ptr<RenderPassInstance>, std::vector<std::vector<std::shared_ptr<ConstantsBuffer>>>> RenderableConstantsBuffer;
typedef std::map<std::shared_ptr<RenderPassInstance>, std::vector<CComPtr<ID3D12RootSignature>>> RenderableRootSignatures;
typedef std::map<std::shared_ptr<RenderPassInstance>, std::vector<CComPtr<ID3D12PipelineState>>> RenderablePipelineStates;

static nlohmann::json defaultShadowMapShaderAttributes = { { "uniqueMaterialInstance", false }, { "castShadows",false }, { "ibl", false} };
#if defined(_EDITOR)
static nlohmann::json defaultPickingShaderAttributes = { { "uniqueMaterialInstance", true}, {"castShadows", false}, {"ibl" , false } };
#endif

namespace Scene
{
#include <JExposeTrackUUIDDecl.h>
#include <RenderableAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttOrder.h>
#include <RenderableAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttDrawersDecl.h>
#include <RenderableAtt.h>
#include <JExposeEnd.h>

	//UPDATE
	void RenderablesStep();
	void CreateRenderablesCameraBinding();
	void DestroyRenderablesCameraBinding();
	void RunBoundingBoxComputeShaders();
	void RunBoundingBoxComputeShadersSolution();

	//DELETE
	void DestroyRenderable(std::shared_ptr<Renderable>& renderable);
	void DestroyRenderables();

	//EDITOR
#if defined(_EDITOR)
	void WriteRenderablesJson(nlohmann::json& json);
#endif

	struct Renderable : SceneObject
	{
		SCENEOBJECT_DECL(Renderable);

#include <JExposeAttFlags.h>
#include <RenderableAtt.h>
#include <JExposeEnd.h>

#include <JExposeDecl.h>
#include <RenderableAtt.h>
#include <JExposeEnd.h>

		XMVECTOR rotationQ();
		XMMATRIX world();

		//Model3D
		std::shared_ptr<Model3DInstance> model3D = nullptr;

		//meshes
		//std::set<unsigned int> skipMeshes;
		RenderableMeshes meshes;
		RenderableMaterials materials;
		RenderableConstantsBuffer constantsBuffers;
		RenderableRootSignatures rootSignatures;
		RenderablePipelineStates pipelineStates;

		std::set<std::string> bindedCameras;
		void UnbindCameras();
		void CreateMeshInstances();
		std::vector<std::shared_ptr<RenderPassInstance>> GetCameraRenderPasses(std::shared_ptr<Camera> cam);
		void CreateMaterialsInstances(std::shared_ptr<Camera> cam);
		void DestroyMaterialsInstances(std::shared_ptr<Camera> cam);
		void CreateRenderPassMaterialsInstances(std::shared_ptr<Templates::RenderPassInstance>& rp);
		void DestroyRenderPassMaterialsInstances(std::shared_ptr<Templates::RenderPassInstance>& rp);
		void CreateConstantsBuffersInstances(std::shared_ptr<Camera> cam);
		void DestroyConstantsBuffersInstances(std::shared_ptr<Camera> cam);
		void CreateRenderPassConstantsBuffersInstances(std::shared_ptr<Templates::RenderPassInstance>& rp);
		void DestroyRenderPassConstantsBuffersInstances(std::shared_ptr<Templates::RenderPassInstance>& rp);
		void CreateRootSignatures(std::shared_ptr<Camera> cam);
		void DestroyRootSignatures(std::shared_ptr<Camera> cam);
		void CreateRenderPassRootSignatures(std::shared_ptr<Templates::RenderPassInstance>& rp);
		void DestroyRenderPassRootSignatures(std::shared_ptr<Templates::RenderPassInstance>& rp);
		void CreatePipelineStates(std::shared_ptr<Camera> cam);
		void DestroyPipelineStates(std::shared_ptr<Camera> cam);
		void CreateRenderPassPipelineStates(std::shared_ptr<Templates::RenderPassInstance>& rp);
		void DestroyRenderPassPipelineStates(std::shared_ptr<Templates::RenderPassInstance>& rp);

		void RebuildMeshMaterials();

		//ANIMATION
		std::shared_ptr<Model3DInstance> animable = nullptr;
		Animation::BonesTransformations bonesTransformation;
		//float currentAnimationTime = 0.0f;
		//float animationTimeFactor = 1.0f;
		//bool playingAnimation = false;

		BoundingBox boundingBox;
		std::shared_ptr<RenderableBoundingBox> boundingBoxCompute; //used for animables

		void CreateFromModel3D(std::string model3DUUID);
		void CreateBoundingBox();
		BoundingBox GetBoundingBox();

		//READ&GET
		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);

		//UPDATE
#if defined(_EDITOR)
		void ReloadModel3D();
#endif
		void WriteMaterialVariablesToConstantsBufferSpace(std::shared_ptr<MaterialInstance>& material, std::shared_ptr<ConstantsBuffer>& cbvData, unsigned int cbvFrameIndex);
		template<typename T>
		void WriteConstantsBuffer(std::string constantName, T& data, unsigned int backbufferIndex, unsigned int slot = 0U, size_t offset = 0ULL)
		{
			for (auto& [rp, meshMaterials] : materials)
			{
				for (unsigned int mesh = 0; mesh < meshMaterials.size(); mesh++)
				{
					auto& vsVars = meshMaterials.at(mesh)->vertexShader->constantsBuffersVariables;
					auto& psVars = meshMaterials.at(mesh)->pixelShader->constantsBuffersVariables;
					auto& cbuffers = constantsBuffers.at(rp).at(mesh);

					if (vsVars.contains(constantName)) {
						auto& vsVar = vsVars.at(constantName);
						if (cbuffers.size() > vsVar.bufferIndex)
						{
							cbuffers[vsVar.bufferIndex]->push<T>(data, backbufferIndex, vsVar.offset + vsVar.size * slot + offset);
						}
					}
					if (psVars.contains(constantName)) {
						auto& psVar = psVars.at(constantName);
						if (cbuffers.size() > psVar.bufferIndex)
						{
							cbuffers[psVar.bufferIndex]->push<T>(data, backbufferIndex, psVar.offset + psVar.size * slot + offset);
						}
					}
				}
			}
		};
		void WriteAnimationConstantsBuffer(unsigned int backbufferIndex);
		void WriteConstantsBuffer(unsigned int backbufferIndex);
		void SetCurrentAnimation(std::string anim, float startTime = 0.0f, float timeFactor = 1.0f, bool play = true, bool loop = false);
		void StepAnimation(double elapsedSeconds);

		//DESTROY
		void Destroy();

		//RENDER
		bool renderException = false;
		void Render(std::shared_ptr<RenderPassInstance> renderPass, std::shared_ptr<Camera> camera = nullptr);
	};
}
