#pragma once
#include "Renderer.h"
#include "DeviceUtils.h"
#include "../Templates/Model3D.h"
#include "../Animation/Animated.h"

namespace Scene::Lights { struct Light; };
namespace Scene::Camera { struct Camera; };
namespace Renderable 
{
	typedef std::pair<MeshPtr, MaterialPtr> MeshMaterialPair;
	typedef std::map<MeshPtr, MaterialPtr> MeshMaterialMap;

	typedef std::pair<Model3DPtr, std::vector<MaterialPtr>> ModelMaterialPair;

	typedef std::pair<MeshPtr, std::vector<ConstantsBufferViewDataPtr>> MeshConstantBufferPair;
	typedef std::map<MeshPtr, std::vector<ConstantsBufferViewDataPtr>> MeshConstantsBufferMap;

	typedef std::pair<MeshPtr, ComPtr<ID3D12RootSignature>> MeshRootSignaturePair;
	typedef std::map<MeshPtr, ComPtr<ID3D12RootSignature>> MeshRootSignatureMap;

	typedef std::pair<MeshPtr, ComPtr<ID3D12PipelineState>> MeshPipelineStatePair;
	typedef std::map<MeshPtr, ComPtr<ID3D12PipelineState>> MeshPipelineStateMap;

	struct RenderableDefinition
	{
		std::wstring name = L"";
		MeshMaterialMap meshMaterials;
		MeshMaterialMap meshMaterialsShadowMap;
		ModelMaterialPair modelMaterials;
		ModelMaterialPair modelMaterialsShadowMap;

		XMVECTOR position = { 0.0f, 0.0f, 0.0f, 1.0f };
		XMVECTOR scale = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMVECTOR rotation = { 0.0f, 0.0f, 0.0f, 1.0f };

		std::set<UINT> skipMeshes;
	};

	struct Renderable
	{
		~Renderable() { this_ptr = nullptr; } //will this work?
		std::shared_ptr<Renderable> this_ptr = nullptr; //dumb but efective

		bool loading = true;

		std::wstring name = L"";
		MeshMaterialMap meshMaterials;
		MeshMaterialMap meshMaterialsShadowMap;

		MeshConstantsBufferMap meshConstantsBuffer;
		MeshRootSignatureMap meshRootSignatures;
		MeshPipelineStateMap meshPipelineStates;

		XMVECTOR position = { 0.0f, 0.0f, 0.0f, 1.0f };
		XMVECTOR scale = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMVECTOR rotation = { 0.0f, 0.0f, 0.0f, 1.0f };

		std::set<MeshPtr> skipMeshes;

		std::shared_ptr<Animation::Animated> animations = nullptr;
		Animation::BonesTransformations bonesTransformation;
		std::wstring currentAnimation = L"";
		float currentAnimationTime = 0.0f;

		//initialization
		Concurrency::task<void> Initialize(const std::shared_ptr<Renderer>& renderer);
		
		//constants buffer
		template<typename T>
		void WriteToConstantsBufferSpace(std::wstring constantName, T& data, UINT index, CBufferVariablesDefinitionPtr& cbufferDef, auto& cbvData, UINT slot = 0U, size_t offset=0ULL) {
			
			if (loading) return;

			auto constantDef = cbufferDef->find(constantName);
			if (constantDef != cbufferDef->end()) cbvData[constantDef->second.bufferIndex]->push<T>(data, index, constantDef->second.offset + constantDef->second.size * slot + offset);

		}
		template<typename T>
		void WriteConstantsBuffer(std::wstring constantName, T& data, UINT backbufferIndex, UINT slot = 0U , size_t offset = 0ULL) {
			
			if (loading) return;

			for (auto& [mesh, cbv] : meshConstantsBuffer) {
				MaterialPtr material = meshMaterials[mesh];
				if (material->loading) continue;

				auto& cbufferDefVS = material->shader->vertexShader->cbufferVariablesDef;
				WriteToConstantsBufferSpace(constantName, data, backbufferIndex, cbufferDefVS, cbv, slot, offset);

				auto& cbufferDefPS = material->shader->pixelShader->cbufferVariablesDef;
				WriteToConstantsBufferSpace(constantName, data, backbufferIndex, cbufferDefPS, cbv, slot, offset);
			}

		};
		void WriteConstantsBuffer(UINT backbufferIndex);

		//rendering
		void Render(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<Scene::Camera::Camera>& camera);
		void RenderShadowMap(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<Scene::Lights::Light>& light, UINT cameraIndex);

		//animations
		void SetCurrentAnimation(std::wstring animation);
		void StepAnimation(double elapsedSeconds);

	};

	typedef void LoadRenderableFn(std::shared_ptr<Renderable> renderable);

	std::shared_ptr<Renderable> CreateRenderable(const std::shared_ptr<Renderer>& renderer, const RenderableDefinition& renderableParams, LoadRenderableFn loadFn = nullptr);
	std::map<std::wstring, std::shared_ptr<Renderable>>& GetRenderables();
	std::map<std::wstring, std::shared_ptr<Renderable>>& GetAnimables();
}
typedef Renderable::Renderable RenderableT;
typedef std::shared_ptr<RenderableT> RenderablePtr;
typedef Renderable::MeshMaterialMap RenderableMapT;
