#pragma once
#include <atlbase.h>
#include <nlohmann/json.hpp>
#include <set>
#include <DirectXCollision.h>
#include "../../Templates/Model3D/Model3D.h"
#include "../../Templates/Mesh/Mesh.h"
#include "../../Templates/Material/Material.h"
#include "../../Animation/Animated.h"
#include "../../Renderer/DeviceUtils/PipelineState/PipelineState.h"

namespace Scene { struct Camera; struct Light; };
namespace Scene
{
	using namespace Templates;
	using namespace DeviceUtils;

#if defined(_EDITOR)
	enum RenderableFlags
	{
		RenderableFlags_CreateMeshes = 0x1,
		RenderableFlags_CreateMeshesFromModel3D = 0x2,
		RenderableFlags_DestroyMeshes = 0x4,
		RenderableFlags_RebuildMeshes = RenderableFlags_CreateMeshes | RenderableFlags_DestroyMeshes,
		RenderableFlags_RebuildMeshesFromModel3D = RenderableFlags_CreateMeshesFromModel3D | RenderableFlags_DestroyMeshes,
		RenderableFlags_SwapMaterialsFromMesh = 0x8,
	};
#endif

	//Mesh Instance to Material Instance
	typedef std::pair<std::shared_ptr<MeshInstance>, std::shared_ptr<MaterialInstance>> MeshMaterialPair;
	typedef std::map<std::shared_ptr<MeshInstance>, std::shared_ptr<MaterialInstance>> MeshMaterialMap;
	//MeshInstance to ConstantsBuffer
	typedef std::pair<std::shared_ptr<MeshInstance>, std::vector<std::shared_ptr<ConstantsBuffer>>> MeshConstantBufferPair;
	typedef std::map<std::shared_ptr<MeshInstance>, std::vector<std::shared_ptr<ConstantsBuffer>>> MeshConstantsBufferMap;
	//MeshInstance to RootSignature
	typedef std::pair<std::shared_ptr<MeshInstance>, CComPtr<ID3D12RootSignature>> MeshRootSignaturePair;
	typedef std::map<std::shared_ptr<MeshInstance>, CComPtr<ID3D12RootSignature>> MeshRootSignatureMap;
	//MeshInstance to PipelineState
	typedef std::pair<std::shared_ptr<MeshInstance>, CComPtr<ID3D12PipelineState>> MeshPipelineStatePair;
	typedef std::map<std::shared_ptr<MeshInstance>, CComPtr<ID3D12PipelineState>> MeshPipelineStateMap;

	inline void TranformJsonToMeshMaterialMap(MeshMaterialMap& map, nlohmann::json j, bool uniqueMaterialInstance = false);

	inline void TransformJsonToPipelineState(RenderablePipelineState& pipelineState, nlohmann::json j, std::string key);

	struct Renderable
	{
		~Renderable() { Destroy(); } //will this work?
		std::shared_ptr<Renderable> this_ptr = nullptr; //dumb but efective

		nlohmann::json json;

		std::string name = "";

		XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
		union {
			XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
			struct { FLOAT roll, pitch, yaw; };
			FLOAT rot[3];
		};

		//draw
		bool visible = true;
		bool hidden = false;
		D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		//Model3D
		std::shared_ptr<Model3DInstance> model3D = nullptr;

		//meshes
		std::set<unsigned int> skipMeshes;
		std::vector<std::shared_ptr<MeshInstance>> meshes;
		std::vector<std::shared_ptr<MeshInstance>> meshesShadowMap;

		//Materials
		MeshMaterialMap meshMaterials;
		MeshMaterialMap meshShadowMapMaterials;
		bool uniqueMaterialInstance = false;

		//CONSTANTS BUFFER
		MeshConstantsBufferMap meshConstantsBuffer;
		MeshConstantsBufferMap meshShadowMapConstantsBuffer;

		//ROOT SIGNATURE
		MeshRootSignatureMap meshRootSignatures;
		MeshRootSignatureMap meshShadowMapRootSignatures;

		//PIPELINE STATE
		MeshPipelineStateMap meshPipelineStates;
		MeshPipelineStateMap meshShadowMapPipelineStates;

		//ANIMATION
		std::shared_ptr<Model3DInstance> animable = nullptr;
		Animation::BonesTransformations bonesTransformation;
		std::string currentAnimation = "";
		float currentAnimationTime = 0.0f;
		float animationTimeFactor = 1.0f;
		bool playingAnimation = false;

		BoundingBox boundingBox;

#if defined(_EDITOR)
		unsigned int renderableUpdateFlags = 0U;
		std::map<std::shared_ptr<MeshInstance>, std::string> materialSwaps;
		std::string model3DSwap;
		std::string meshSwap;
#endif

		//CREATE
		void SetMeshMaterial(std::shared_ptr<MeshInstance> mesh, std::shared_ptr<MaterialInstance> material);
		void CreateFromModel3D(std::string model3DName);
		void CreateMeshesComponents();
		void CreateMeshConstantsBuffers(std::shared_ptr<MeshInstance> mesh);
		void CreateMeshShadowMapConstantsBuffers(std::shared_ptr<MeshInstance> mesh);
		void CreateMeshRootSignatures(std::shared_ptr<MeshInstance> mesh);
		void CreateMeshShadowMapRootSignatures(std::shared_ptr<MeshInstance> mesh);
		void CreateMeshPipelineState(std::shared_ptr<MeshInstance> mesh);
		void CreateMeshShadowMapPipelineState(std::shared_ptr<MeshInstance> mesh);
		void CreateBoundingBox();

		//READ&GET
		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);

		//UPDATE
#if defined(_EDITOR)
		void CleanMeshes();
		void SwapMaterials();
		void SwapMeshes();
		void SwapModel3D();
#endif
		void WriteMaterialVariablesToConstantsBufferSpace(std::shared_ptr<MaterialInstance>& material, std::shared_ptr<ConstantsBuffer>& cbvData, unsigned int cbvFrameIndex);
		template<typename T>
		void WriteConstantsBuffer(std::string constantName, T& data, unsigned int backbufferIndex, unsigned int slot = 0U, size_t offset = 0ULL) {

			for (auto& [mesh, cbv] : meshConstantsBuffer)
			{
				auto& mat = meshMaterials.at(mesh);
				auto& vsVars = mat->vertexShader->constantsBuffersVariables;
				auto& psVars = mat->pixelShader->constantsBuffersVariables;

				if (vsVars.contains(constantName)) {
					auto& vsVar = vsVars.at(constantName);
					cbv[vsVar.bufferIndex]->push<T>(data, backbufferIndex, vsVar.offset + vsVar.size * slot + offset);
				}
				if (psVars.contains(constantName)) {
					auto& psVar = psVars.at(constantName);
					cbv[psVar.bufferIndex]->push<T>(data, backbufferIndex, psVar.offset + psVar.size * slot + offset);
				}
			}
		}

		template<typename T>
		void WriteShadowMapConstantsBuffer(std::string constantName, T& data, unsigned int backbufferIndex, unsigned int slot = 0U, size_t offset = 0ULL) {

			for (auto& [mesh, cbv] : meshShadowMapConstantsBuffer)
			{
				auto& mat = meshShadowMapMaterials.at(mesh);
				auto& vsVars = mat->vertexShader->constantsBuffersVariables;
				auto& psVars = mat->pixelShader->constantsBuffersVariables;

				if (vsVars.contains(constantName)) {
					auto& vsVar = vsVars.at(constantName);
					cbv[vsVar.bufferIndex]->push<T>(data, backbufferIndex, vsVar.offset + vsVar.size * slot + offset);
				}
				if (psVars.contains(constantName)) {
					auto& psVar = psVars.at(constantName);
					cbv[psVar.bufferIndex]->push<T>(data, backbufferIndex, psVar.offset + psVar.size * slot + offset);
				}
			}

			//if (loading) return;
			//
			//for (auto& [mesh, cbv] : meshConstantsBuffer) {
			//	MaterialPtr material = meshMaterials[mesh];
			//	if (material->loading) continue;
			//
			//	auto& cbufferDefVS = material->shader->vertexShader->constantsBuffersVariables;
			//	WriteToConstantsBufferSpace(constantName, data, backbufferIndex, cbufferDefVS, cbv, slot, offset);
			//
			//	auto& cbufferDefPS = material->shader->pixelShader->constantsBuffersVariables;
			//	WriteToConstantsBufferSpace(constantName, data, backbufferIndex, cbufferDefPS, cbv, slot, offset);
			//}
		};

		void WirteAnimationConstantsBuffer(unsigned int backbufferIndex);
		void WriteConstantsBuffer(unsigned int backbufferIndex);
		void WriteShadowMapConstantsBuffer(unsigned int backbufferIndex);

		void SetCurrentAnimation(std::string animation, float animationTime = 0.0f, float timeFactor = 1.0f, bool autoPlay = true);
		void StepAnimation(double elapsedSeconds);

		//DESTROY
		void Destroy();

		//RENDER
		void Render(std::shared_ptr<Camera> camera = nullptr);
		void RenderShadowMap(const std::shared_ptr<Light>& light, unsigned int cameraIndex);

		//EDITOR
#if defined(_EDITOR)
		//nlohmann::json json();
		void DrawEditorInformationAttributes();
		void DrawEditorWorldAttributes();
		void DrawEditorAnimationAttributes();
		void DrawEditorMeshesAttributes();
#endif
	};

	//CREATE
	std::shared_ptr<Renderable> CreateRenderable(nlohmann::json renderablej);

	//READ
	std::map<std::string, std::shared_ptr<Renderable>>& GetRenderables();
	std::map<std::string, std::shared_ptr<Renderable>>& GetAnimables();
#if defined(_EDITOR)
	std::vector<std::string> GetRenderablesNames();
#endif

	//UPDATE
	void RenderablesStep();

	//EDITOR
#if defined(_EDITOR)
	void SelectRenderable(std::string renderableName, void*& ptr);
	void DeSelectRenderable(void*& ptr);
	void DrawRenderablePanel(void*& ptr, ImVec2 pos, ImVec2 size, bool pop);
	std::string GetRenderableName(void* ptr);
#endif

	//DELETE
	void DestroyRenderable(std::shared_ptr<Renderable>& renderable);
	void DestroyRenderables();
}
