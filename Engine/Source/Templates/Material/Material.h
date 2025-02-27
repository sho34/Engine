#pragma once

#include "../Shader/Shader.h"
#include "../../Renderer/VertexFormats.h"
#include "Variables.h"
#include "SamplerDesc.h"
#include "Texture.h"

namespace Templates { struct MeshInstance; }

namespace Templates {

	namespace Material
	{
		inline static const  std::string templateName = "materials.json";
	};

	struct MaterialInstance {
		~MaterialInstance() { Destroy(); }
		std::string material;
		std::map<TextureType, MaterialTexture> tupleTextures;
		VertexClass vertexClass;
		MaterialVariablesMapping variablesMapping;
		std::vector<size_t> variablesBufferSize;
		std::vector<std::vector<byte>> variablesBuffer;
		std::shared_ptr<ShaderBinary> vertexShader;
		std::shared_ptr<ShaderBinary> pixelShader;
		std::vector<MaterialSamplerDesc> samplers;
		std::map<TextureType, std::shared_ptr<MaterialTextureInstance>> textures;

		void Destroy();
		bool ShaderBinaryHasRegister(std::function<int(std::shared_ptr<ShaderBinary>&)> getRegister);
		void LoadVariablesMapping(nlohmann::json material);
		void SetRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& cbvSlot);
	};

	//CREATE
	void CreateMaterial(std::string name, nlohmann::json json);
	std::shared_ptr<MaterialInstance> GetMaterialInstance(std::string name, const std::map<TextureType, MaterialTexture>& textures, const std::shared_ptr<MeshInstance>& mesh, bool uniqueMaterialInstance = false);
	void LoadMaterialInstance(std::string name, const std::shared_ptr<MeshInstance>& mesh, const std::shared_ptr<MaterialInstance>& material, const std::map<TextureType, MaterialTexture>& textures);

	//READ&GET
	//std::shared_ptr<Material> GetMaterialTemplate(std::string name);
	nlohmann::json GetMaterialTemplate(std::string name);
	std::vector<std::string> GetMaterialsNames();

	//UPDATE

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
};
