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
		RenderableFlags_RebuildMaterials = 0x10,
		RenderableFlags_Destroy = 0x20
	};

	enum Renderable_PopupModal
	{
		RenderablePopupModal_CannotDelete = 1,
		RenderablePopupModal_CreateNew = 2
	};
#endif

	static nlohmann::json defaultShadowMapShaderAttributes = { { "uniqueMaterialInstance", false }, { "castShadows",false } };

	//Mesh Instance to Material Instance
	typedef std::pair<std::shared_ptr<MeshInstance>, std::shared_ptr<MaterialInstance>> MeshMaterialPair;
	typedef std::map<std::shared_ptr<MeshInstance>, std::shared_ptr<MaterialInstance>> MeshMaterialMap;
	//MeshInstance to ConstantsBuffer
	typedef std::pair<std::shared_ptr<MeshInstance>, std::vector<std::shared_ptr<ConstantsBuffer>>> MeshConstantBufferPair;
	typedef std::map<std::shared_ptr<MeshInstance>, std::vector<std::shared_ptr<ConstantsBuffer>>> MeshConstantsBufferMap;

	struct Renderable
	{
		~Renderable() { Destroy(); }
		std::shared_ptr<Renderable> this_ptr = nullptr; //dumb but efective

		nlohmann::json json;

		std::string uuid();
		void uuid(std::string uuid);

		std::string name();
		void name(std::string name);

		XMFLOAT3 position();
		void position(XMFLOAT3 f3);
		void position(nlohmann::json f3);

		XMFLOAT3 rotation();
		void rotation(XMFLOAT3 f3);
		void rotation(nlohmann::json f3);

		XMFLOAT3 scale();
		void scale(XMFLOAT3 f3);
		void scale(nlohmann::json f3);

		//draw
		bool visible();
		void visible(bool visible);

		bool hidden();
		void hidden(bool hidden);

		D3D_PRIMITIVE_TOPOLOGY topology();
		void topology(std::string topology);

		//shader attributes
		bool uniqueMaterialInstance();
		void uniqueMaterialInstance(bool uniqueMaterialInstance);

		bool castShadows();
		void castShadows(bool castShadows);

		//Model3D
		std::shared_ptr<Model3DInstance> model3D = nullptr;

		//meshes
		std::set<unsigned int> skipMeshes;
		std::vector<std::shared_ptr<MeshInstance>> meshes;
		std::vector<std::shared_ptr<MeshInstance>> meshesShadowMap;

		//Materials
		MeshMaterialMap meshMaterials;
		MeshMaterialMap meshShadowMapMaterials;

		//CONSTANTS BUFFER
		MeshConstantsBufferMap meshConstantsBuffer;
		MeshConstantsBufferMap meshShadowMapConstantsBuffer;

		//ROOT SIGNATURE
		MeshHashedRootSignatureMap meshHashedRootSignatures;
		MeshHashedRootSignatureMap meshHashedShadowMapRootSignatures;

		//PIPELINE STATE
		std::map<size_t, MeshHashedPipelineStateMap> meshHashedPipelineStates;
		std::map<size_t, MeshHashedPipelineStateMap> meshHashedShadowMapPipelineStates;

		//ANIMATION
		std::shared_ptr<Model3DInstance> animable = nullptr;
		Animation::BonesTransformations bonesTransformation;
		std::string currentAnimation = "";
		float currentAnimationTime = 0.0f;
		float animationTimeFactor = 1.0f;
		bool playingAnimation = false;

		BoundingBox boundingBox;

#if defined(_EDITOR)
		static nlohmann::json creationJson;
		static unsigned int popupModalId;
		unsigned int renderableUpdateFlags = 0U;
		std::map<std::shared_ptr<MeshInstance>, std::string> materialSwaps;
		std::string model3DSwap;
		std::string meshSwap;
		std::vector<unsigned int> materialToChangeMeshIndex;
		std::vector<std::string> materialToRebuild;
#endif

		//CREATE
		void TransformJsonToMeshMaterialMap(MeshMaterialMap& map, nlohmann::json j, nlohmann::json shaderAttributes, std::map<TextureType, MaterialTexture> baseTextures = {});
		void TransformJsonToRenderTargetBlendDesc(D3D12_RENDER_TARGET_BLEND_DESC& RenderTarget, nlohmann::json j);
		void TransformJsonToBlendState(D3D12_BLEND_DESC& BlendState, nlohmann::json j, std::string key);
		void TransformJsonToRasterizerState(D3D12_RASTERIZER_DESC& RasterizerState, nlohmann::json j, std::string key);
		void SetMeshMaterial(std::shared_ptr<MeshInstance> mesh, std::shared_ptr<MaterialInstance> material);
		void CreateFromModel3D(std::string model3DUUID);
		void CreateMeshesComponents();
		void CreateMeshConstantsBuffers(std::shared_ptr<MeshInstance> mesh);
		void CreateMeshShadowMapConstantsBuffers(std::shared_ptr<MeshInstance> mesh);
		void CreateMeshRootSignatures(std::shared_ptr<MeshInstance> mesh);
		void CreateMeshShadowMapRootSignatures(std::shared_ptr<MeshInstance> mesh);
		void CreateMeshPipelineState(std::shared_ptr<MeshInstance> mesh);
		void CreateMeshPipelineState(size_t passHash, std::shared_ptr<MeshInstance> mesh);
		void CreateMeshShadowMapPipelineState(std::shared_ptr<MeshInstance> mesh);
		void CreateMeshShadowMapPipelineState(size_t passHash, std::shared_ptr<MeshInstance> mesh);
		void CreateBoundingBox();

		//READ&GET
		void FillRenderableBoundingBox(std::shared_ptr<Renderable>& bbox);

		//UPDATE
#if defined(_EDITOR)
		void BindChangesToMaterial(unsigned int meshIndex);
		void DestroyMaterialsToRebuild();
		void RebuildMaterials();
		void CleanMeshes();
		void SwapMaterials();
		void SwapMeshes();
		void SwapModel3D();
		void ReloadModel3D();
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
		};
		void WirteAnimationConstantsBuffer(unsigned int backbufferIndex);
		void WriteConstantsBuffer(unsigned int backbufferIndex);
		void WriteShadowMapConstantsBuffer(unsigned int backbufferIndex);
		void SetCurrentAnimation(std::string animation, float animationTime = 0.0f, float timeFactor = 1.0f, bool autoPlay = true);
		void StepAnimation(double elapsedSeconds);

		//DESTROY
		void Destroy();

		//RENDER
		void Render(size_t passHash, std::shared_ptr<Camera> camera = nullptr);
		void RenderShadowMap(size_t passHash, const std::shared_ptr<Light>& light, unsigned int cameraIndex);

		//EDITOR
#if defined(_EDITOR)
		//nlohmann::json json();
		void DrawEditorInformationAttributes();
		void DrawEditorWorldAttributes();
		void DrawEditorAnimationAttributes();
		void DrawEditorShaderAttributes();
		void DrawEditorModelSelectionAttributes();
		void DrawEditorMeshesAttributes();
		void DrawEditorPipelineStateAttributes();
#endif
	};

	//CREATE
	std::shared_ptr<Renderable> CreateRenderable(nlohmann::json renderablej);

	//READ&GET
	std::shared_ptr<Renderable> GetRenderable(std::string uuid);
	std::map<std::string, std::shared_ptr<Renderable>>& GetRenderables();
	std::map<std::string, std::shared_ptr<Renderable>>& GetAnimables();
#if defined(_EDITOR)
	std::vector<std::string> GetRenderablesNames();
	std::vector<UUIDName> GetRenderablesUUIDNames();
#endif

	//UPDATE
	void RenderablesStep();

	//EDITOR
#if defined(_EDITOR)
	void SelectRenderable(std::string uuid, std::string& edSO);
	void DeSelectRenderable(std::string& edSO);
	void DrawRenderablePanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop);
	std::string GetRenderableName(std::string uuid);
	void CreateNewRenderable();
	void DeleteRenderable(std::string uuid);
	void DrawRenderablesPopups();
	void WriteRenderablesJson(nlohmann::json& json);
#endif

	//DELETE
	void DestroyRenderable(std::shared_ptr<Renderable>& renderable);
	void DestroyRenderables();
}
