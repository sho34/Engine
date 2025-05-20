#pragma once

#include "../Shader/Shader.h"
#include "../../Renderer/VertexFormats.h"
#include "Variables.h"
#include "SamplerDesc.h"
#include <tuple>
#include <d3d12.h>
#include <wrl/client.h>
#include <atlbase.h>
#if defined(_EDITOR)
#include <imgui.h>
#endif

namespace Templates { struct MeshInstance; struct TextureInstance; }
namespace Scene { struct Renderable; }

using namespace Scene;

typedef std::tuple<
	std::string, //name
	nlohmann::json //data
#if defined(_EDITOR)
	,
	std::vector<std::tuple<std::shared_ptr<Renderable>, unsigned int>>
#endif
> MaterialTemplate;

namespace Templates {

#if defined(_EDITOR)
	enum MaterialPopupModal
	{
		MaterialPopupModal_CannotDelete = 1,
		MaterialPopupModal_CreateNew = 2
	};
#endif

	namespace Material
	{
		inline static const std::string templateName = "materials.json";
#if defined(_EDITOR)
		inline static const std::string fallbackShader_vs = "BaseLighting_vs";
		inline static const std::string fallbackShader_ps = "BaseLighting_ps";
#endif
	};

	struct MaterialInstance {
		~MaterialInstance() { Destroy(); }
		std::string material;
		std::string instanceName;
		std::map<TextureType, std::string> tupleTextures;
		VertexClass vertexClass;
		MaterialVariablesMapping variablesMapping;
		std::vector<size_t> variablesBufferSize;
		std::vector<std::vector<byte>> variablesBuffer;
		std::string vertexShaderUUID;
		std::string pixelShaderUUID;
		std::vector<std::string> defines;
		std::shared_ptr<ShaderInstance> vertexShader;
		std::shared_ptr<ShaderInstance> pixelShader;
		std::vector<MaterialSamplerDesc> samplers;
		std::map<TextureType, std::shared_ptr<TextureInstance>> textures;
		std::map<unsigned int, ::CD3DX12_GPU_DESCRIPTOR_HANDLE> uav;
		unsigned int changesCounter = 0U;
		std::vector<std::function<void()>> rebuildCallbacks;
		std::vector<std::function<void()>> propagateMappedValueChanges;

		void GetShaderInstances();
		void BindRebuildChange(std::function<void()> changeListener);
		void NotifyRebuild();
		void BindMappedValueChange(std::function<void()> changeListener);
		void NotifyMappedValueChange();
		void Destroy();
		bool ShaderInstanceHasRegister(std::function<int(std::shared_ptr<ShaderInstance>&)> getRegister);
		bool ConstantsBufferContains(std::string varName);
		ShaderConstantsBufferVariable& GetConstantsBufferVariable(std::string varName);
		void LoadVariablesMapping(nlohmann::json material);
		void SetRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot);
		void UpdateMappedValues(nlohmann::json mappedValues);
		void CallRebuildCallbacks() { for (auto& cb : rebuildCallbacks) { cb(); } }
		void CallMappedValueChangesPropagation() { for (auto& cb : propagateMappedValueChanges) { cb(); } }
		bool CanReleaseGPUUploadIntermediateResources();
		void ReleaseGPUUploadIntermediateResources();
	};

	//CREATE
	void CreateMaterial(nlohmann::json json);
	std::shared_ptr<MaterialInstance> GetMaterialInstance(std::string uuid, const std::map<TextureType, std::string>& textures, const std::shared_ptr<MeshInstance>& mesh, nlohmann::json shaderAttributes);
	void LoadMaterialInstance(std::string uuid, const std::shared_ptr<MeshInstance>& mesh, std::string instanceName, const std::shared_ptr<MaterialInstance>& material, const std::map<TextureType, std::string>& textures, bool castShadows);

	//READ&GET
	nlohmann::json GetMaterialTemplate(std::string uuid);
	std::string GetMaterialName(std::string uuid);
	std::vector<std::string> GetMaterialsNames();
	std::vector<UUIDName> GetMaterialsUUIDsNames();
	std::string FindMaterialUUIDByName(std::string name);

	//UPDATE
	void TransformJsonToMaterialTextures(std::map<TextureType, std::string>& textures, nlohmann::json object, const std::string& key);

	//DESTROY
	void DestroyMaterial(std::string uuid);
	void ReleaseMaterialTemplates();
	void DestroyMaterialInstance(std::shared_ptr<MaterialInstance>& material, const std::shared_ptr<MeshInstance>& mesh, nlohmann::json shaderAttributes);
	void FreeGPUTexturesUploadIntermediateResources();

	//EDITOR
#if defined(_EDITOR)
	void DrawMaterialPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop);
	bool DrawTextureParameters(nlohmann::json& mat, std::set<TextureType> texturesInShader);
	bool DrawSamplerParameters(nlohmann::json& mat, unsigned int totalSamplers, std::function<nlohmann::json()> getSamplerJson);
	void CreateNewMaterial();
	void DeleteMaterial(std::string uuid);
	void DrawMaterialsPopups();
	void DetachShader(std::string uuid);
	void WriteMaterialsJson(nlohmann::json& json);
#endif
};
