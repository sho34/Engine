#pragma once

#include "../Shader/Shader.h"
#include "../../Renderer/VertexFormats.h"
#include "Variables.h"
#include "SamplerDesc.h"
#include "Texture.h"
#include <d3d12.h>
#include <wrl/client.h>
#if defined(_EDITOR)
#include <imgui.h>
#endif

namespace Templates { struct MeshInstance; }

namespace Templates {

	namespace Material
	{
		inline static const std::string templateName = "materials.json";
		inline static const std::string fallbackShader = "BaseLighting";
	};

	struct MaterialInstance {
		~MaterialInstance() { Destroy(); }
		std::string material;
		std::map<TextureType, MaterialTexture> tupleTextures;
		VertexClass vertexClass;
		MaterialVariablesMapping variablesMapping;
		std::vector<size_t> variablesBufferSize;
		std::vector<std::vector<byte>> variablesBuffer;
		std::string shaderName;
		std::vector<std::string> defines;
		std::shared_ptr<ShaderInstance> vertexShader;
		std::shared_ptr<ShaderInstance> pixelShader;
		std::vector<MaterialSamplerDesc> samplers;
		std::map<TextureType, std::shared_ptr<MaterialTextureInstance>> textures;
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
		void LoadVariablesMapping(nlohmann::json material);
		void SetRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot);
		void UpdateMappedValues(nlohmann::json mappedValues);
	};

	//CREATE
	void CreateMaterial(std::string name, nlohmann::json json);
	std::shared_ptr<MaterialInstance> GetMaterialInstance(std::string name, const std::map<TextureType, MaterialTexture>& textures, const std::shared_ptr<MeshInstance>& mesh, nlohmann::json shaderAttributes);
	void LoadMaterialInstance(std::string name, const std::shared_ptr<MeshInstance>& mesh, const std::shared_ptr<MaterialInstance>& material, const std::map<TextureType, MaterialTexture>& textures, bool castShadows);

	//READ&GET
	//std::shared_ptr<Material> GetMaterialTemplate(std::string name);
	nlohmann::json GetMaterialTemplate(std::string name);
	std::vector<std::string> GetMaterialsNames();

	//UPDATE

	//DESTROY
	void DestroyMaterial(std::string name);
	void ReleaseMaterialTemplates();
	void DestroyMaterialInstance(std::shared_ptr<MaterialInstance>& material, const std::shared_ptr<MeshInstance>& mesh, nlohmann::json shaderAttributes);

	//EDITOR
#if defined(_EDITOR)
	void DrawMaterialPanel(std::string& material, ImVec2 pos, ImVec2 size, bool pop);
	void DeleteMaterial(std::string name);
	void DrawMaterialsPopups();
	void DetachShader(std::string material);
	//nlohmann::json json();
#endif
};
