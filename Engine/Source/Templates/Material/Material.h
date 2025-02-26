#pragma once

#include "../Shader/Shader.h"
#include "../../Renderer/VertexFormats.h"
#include "Variables.h"
#include "SamplerDesc.h"
#include "Texture.h"

//struct Renderer;

namespace Templates { struct MeshInstance; }

namespace Templates {

	namespace Material
	{
		inline static const  std::string templateName = "materials.json";
	};

	/*
	struct Material
	{
		inline static const  std::string templateName = "materials.json";

		//name
		std::string name;

		nlohmann::json json;

		//constants buffer
		MaterialInitialValueMap mappedValues = MaterialInitialValueMap();

		//two sided FRONT_CULL OR CULL_NONE
		//bool twoSided = false;

		//textures
		std::vector<MaterialTexture> textures;

		//samplers
		std::vector<MaterialSamplerDesc> samplers;

		//vertex & pixel shader
		//std::shared_ptr<Shader> shader;
		std::string shader;

		//flags
		//unsigned int flags = TemplateFlags::None;
		//loading state
		//bool loading = false;
	};
	*/

	struct MaterialInstance {
		~MaterialInstance() { Destroy(); }
		//std::shared_ptr<Material> material;
		std::string material;
		std::vector<MaterialTexture> tupleTextures;
		VertexClass vertexClass;
		MaterialVariablesMapping variablesMapping;
		std::vector<size_t> variablesBufferSize;
		std::vector<std::vector<byte>> variablesBuffer;
		std::shared_ptr<ShaderBinary> vertexShader;
		std::shared_ptr<ShaderBinary> pixelShader;
		std::vector<MaterialSamplerDesc> samplers;
		std::vector<std::shared_ptr<MaterialTextureInstance>> textures;

		void Destroy();
		bool ShaderBinaryHasRegister(std::function<int(std::shared_ptr<ShaderBinary>&)> getRegister);
		//void LoadVariablesMapping(const std::shared_ptr<Material>& material);
		void LoadVariablesMapping(nlohmann::json material);
		void SetRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot);
	};

	//CREATE
	void CreateMaterial(std::string name, nlohmann::json json);
	std::shared_ptr<MaterialInstance> GetMaterialInstance(std::string name, const std::vector<MaterialTexture>& textures, const std::shared_ptr<MeshInstance>& mesh, bool uniqueMaterialInstance = false);
	void LoadMaterialInstance(std::string name, const std::shared_ptr<MeshInstance>& mesh, const std::shared_ptr<MaterialInstance>& material, const std::vector<MaterialTexture>& textures);

	/*
	Concurrency::task<void> CreateMaterialTemplate(std::string name, MaterialDefinition materialDefinition, LoadMaterialFn loadFn = nullptr);
	std::shared_ptr<Material>* CreateNewMaterial(std::string name, MaterialDefinition materialDefinition);
	void BuildMaterialProperties(std::shared_ptr<Material>& material);
	void BuildMaterialTextures(const std::shared_ptr<Renderer>& renderer, std::shared_ptr<Material>& material);
	*/

	//READ&GET
	//std::shared_ptr<Material> GetMaterialTemplate(std::string name);
	nlohmann::json GetMaterialTemplate(std::string name);

	std::vector<std::string> GetMaterialsNames();
	/*
	std::vector<std::string> GetMaterialsNamesMatchingClass(VertexClass vertexClass);
	std::vector<std::string> GetShadowMapMaterialsNamesMatchingClass(VertexClass vertexClass);

	//UPDATE
	Concurrency::task<void> BindToMaterialTemplate(const std::string& name, void* target, NotificationCallbacks callbacks);
	*/
	//DESTROY
	void ReleaseMaterialTemplates();
	void DestroyMaterialInstance(std::shared_ptr<MaterialInstance>& material, const std::shared_ptr<MeshInstance>& mesh);

	//EDITOR
#if defined(_EDITOR)
	//void SelectMaterial(std::string materialName, void*& ptr);
	void DrawMaterialPanel(std::string& material, ImVec2 pos, ImVec2 size, bool pop);
	//std::string GetMaterialName(void* ptr);
	std::string GetMaterialInstanceTemplateName(std::shared_ptr<MaterialInstance> material);
	//nlohmann::json json();
#endif
	/*
	int FindCBufferIndexInMaterial(const std::shared_ptr<Material>& material, std::string bufferName);
	int PickRegister(std::shared_ptr<Material>& mat, std::function<int(ShaderCompilerOutputPtr)> pick);
	*/
};
