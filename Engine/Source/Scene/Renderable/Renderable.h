#pragma once
#include "../../Renderer/Renderer.h"
#include "../../Templates/Model3D/Model3DImpl.h"
#include "../../Animation/Animated.h"

namespace Scene::Lights { struct Light; };
namespace Scene::Camera { struct Camera; };
namespace Scene::Renderable 
{
	typedef std::pair<MeshPtr, MaterialPtr> MeshMaterialPair;
	typedef std::map<MeshPtr, MaterialPtr> MeshMaterialMap;

	typedef std::pair<Model3DPtr, std::vector<MaterialPtr>> ModelMaterialPair;

	typedef std::pair<MeshPtr, std::vector<ConstantsBufferViewDataPtr>> MeshConstantBufferPair;
	typedef std::map<MeshPtr, std::vector<ConstantsBufferViewDataPtr>> MeshConstantsBufferMap;

	typedef std::pair<MeshPtr, CComPtr<ID3D12RootSignature>> MeshRootSignaturePair;
	typedef std::map<MeshPtr, CComPtr<ID3D12RootSignature>> MeshRootSignatureMap;

	typedef std::pair<MeshPtr, CComPtr<ID3D12PipelineState>> MeshPipelineStatePair;
	typedef std::map<MeshPtr, CComPtr<ID3D12PipelineState>> MeshPipelineStateMap;

	struct Renderable
	{
		~Renderable() { this_ptr = nullptr; } //will this work?
		std::shared_ptr<Renderable> this_ptr = nullptr; //dumb but efective

		bool loading = true;

		std::string name = "";
		std::string model3DName = "";
		MeshMaterialMap meshMaterials;
		MeshMaterialMap meshMaterialsShadowMap;

		MeshConstantsBufferMap meshConstantsBuffer;
		MeshRootSignatureMap meshRootSignatures;
		MeshPipelineStateMap meshPipelineStates;

		XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
		XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };

		std::set<MeshPtr> skipMeshes;
#if defined(_EDITOR)
		std::vector<MeshPtr> meshesVector;
#endif

		std::shared_ptr<Animation::Animated> animations = nullptr;
		Animation::BonesTransformations bonesTransformation;
		std::string currentAnimation = "";
		float currentAnimationTime = 0.0f;
		float animationTimeFactor = 1.0f;
		bool playingAnimation = false;

		//initialization
		void Initialize(const std::shared_ptr<Renderer>& renderer);
		
		//constants buffer
		template<typename T>
		void WriteToConstantsBufferSpace(std::string constantName, T& data, UINT index, CBufferVariablesDefinitionPtr& cbufferDef, auto& cbvData, UINT slot = 0U, size_t offset=0ULL) {
			
			if (loading) return;

			auto constantDef = cbufferDef->find(constantName);
			if (constantDef != cbufferDef->end()) cbvData[constantDef->second.bufferIndex]->push<T>(data, index, constantDef->second.offset + constantDef->second.size * slot + offset);

		}
		template<typename T>
		void WriteConstantsBuffer(std::string constantName, T& data, UINT backbufferIndex, UINT slot = 0U , size_t offset = 0ULL) {
			
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
		void SetCurrentAnimation(std::string animation, float animationTime = 0.0f, float timeFactor = 1.0f, bool autoPlay = true);
		void StepAnimation(double elapsedSeconds);

#if defined(_EDITOR)
		nlohmann::json json();
		void DrawEditorInformationAttributes();
		void DrawEditorWorldAttributes();
		void DrawEditorAnimationAttributes();
		void DrawEditorMeshesAttributes();
#endif

	};

	//create
	std::shared_ptr<Renderable> CreateRenderable(nlohmann::json renderablej);
	
	//access
	std::map<std::string, std::shared_ptr<Renderable>>& GetRenderables();
	std::map<std::string, std::shared_ptr<Renderable>>& GetAnimables();
	std::vector<std::string> GetRenderablesNames();
#if defined(_EDITOR)
	void SelectRenderable(std::string renderableName, void*& ptr);
	void DrawRenderablePanel(void*& ptr, ImVec2 pos, ImVec2 size);
	std::string GetRenderableName(void* ptr);
#endif

	//destroy
	void DestroyRenderables();
}
typedef Scene::Renderable::Renderable RenderableT;
typedef std::shared_ptr<RenderableT> RenderablePtr;
typedef Scene::Renderable::MeshMaterialMap RenderableMapT;
