#pragma once

#include "../Shader/ShaderImpl.h"
#include "../../Renderer/VertexFormats.h"

struct Renderer;

namespace Templates::Material {

	static const std::string templateName = "materials.json";

	struct MaterialSamplerDesc : D3D12_STATIC_SAMPLER_DESC {
		MaterialSamplerDesc() {
			Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			MipLODBias = 0;
			MaxAnisotropy = 0;
			ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
			MinLOD = 0.0f;
			MaxLOD = D3D12_FLOAT32_MAX;
			ShaderRegister = 0;
			RegisterSpace = 0;
			ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		}
		MaterialSamplerDesc(const MaterialSamplerDesc& other) {
			Filter = other.Filter;
			AddressU = other.AddressU;
			AddressV = other.AddressV;
			AddressW = other.AddressW;
			MipLODBias = other.MipLODBias;
			MaxAnisotropy = other.MaxAnisotropy;
			ComparisonFunc = other.ComparisonFunc;
			BorderColor = other.BorderColor;
			MinLOD = other.MinLOD;
			MaxLOD = other.MaxLOD;
			ShaderRegister = other.ShaderRegister;
			RegisterSpace = other.RegisterSpace;
			ShaderVisibility = other.ShaderVisibility;
		}
		bool operator<(const MaterialSamplerDesc& other) const {
			return std::tie(
				Filter, AddressU, AddressV, AddressW,
				MipLODBias, MaxAnisotropy, ComparisonFunc, BorderColor,
				MinLOD, MaxLOD, ShaderRegister, RegisterSpace, ShaderVisibility)
				< std::tie(
					other.Filter, other.AddressU, other.AddressV, other.AddressW,
					other.MipLODBias, other.MaxAnisotropy, other.ComparisonFunc, other.BorderColor,
					other.MinLOD, other.MaxLOD, other.ShaderRegister, other.RegisterSpace, other.ShaderVisibility);
		}
#if defined(_EDITOR)
		nlohmann::json json() {
			nlohmann::json j = nlohmann::json({});
			j["Filter"] = filterToString[Filter];
			j["AddressU"] = textureAddressModeToString[AddressU];
			j["AddressV"] = textureAddressModeToString[AddressV];
			j["AddressW"] = textureAddressModeToString[AddressW];
			j["MipLODBias"] = MipLODBias;
			j["MaxAnisotropy"] = MaxAnisotropy;
			j["ComparisonFunc"] = comparisonFuncToString[ComparisonFunc];
			j["BorderColor"] = borderColorToString[BorderColor];
			j["MinLOD"] = MinLOD;
			j["MaxLOD"] = MaxLOD;
			j["ShaderRegister"] = ShaderRegister;
			j["RegisterSpace"] = RegisterSpace;
			j["ShaderVisibility"] = shaderVisibilityToString[ShaderVisibility];
			return j;
		}
#endif
	};

	struct TextureDefinition {
		std::string texturePath;
		DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		UINT numFrames = 0U;
	};

	struct MaterialDefinition {
		//vertex & pixel shader 
		std::string shaderTemplate = "";

		bool systemCreated = false;

		//constants buffer
		MaterialInitialValueMap mappedValues = MaterialInitialValueMap();

		//textures
		std::vector<TextureDefinition> textures = {};

		//samplers
		std::vector<MaterialSamplerDesc> samplers = {};

		//two sided FRONT_CULL OR CULL_NONE
		bool twoSided = false;
	};

	struct MaterialTextures
	{
		std::string texturePath;
		DXGI_FORMAT textureFormat;
		UINT numFrames;
		D3D12_SHADER_RESOURCE_VIEW_DESC textureViewDesc;
		CD3DX12_CPU_DESCRIPTOR_HANDLE textureCpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE textureGpuHandle;
		CComPtr<ID3D12Resource> texture;
		CComPtr<ID3D12Resource> textureUpload;
	};

	struct Material
	{
		//definition
		MaterialDefinition materialDefinition;

		//name
		std::string name;

		//vertex & pixel shader
		ShaderPtr shader;

		//constants buffer
		std::vector<std::shared_ptr<UINT8*>> variablesBuffer;
		std::vector<size_t> variablesBufferSize;
		MaterialVariablesMapping variablesMapping;

		//textures
		std::vector<MaterialTextures> textures;

		//samplers
		std::vector<MaterialSamplerDesc> samplers;

		//loading state
		bool loading = false;
	};

	typedef void LoadMaterialFn(std::shared_ptr<Material> material);

	std::shared_ptr<Material>* CreateNewMaterial(std::string name, MaterialDefinition materialDefinition);
	void ReleaseMaterialTemplates();
	void BuildMaterialProperties(std::shared_ptr<Material>& material);

	Concurrency::task<void> CreateMaterialTemplate(std::string name, MaterialDefinition materialDefinition, LoadMaterialFn loadFn = nullptr);
	Concurrency::task<void> BindToMaterialTemplate(const std::string& name, void* target, NotificationCallbacks callbacks);
	std::shared_ptr<Material> GetMaterialTemplate(std::string name);
	std::shared_ptr<Material>* GetMaterialTemplatePtr(std::string name);
	std::vector<std::string> GetMaterialsNames();
	std::vector<std::string> GetMaterialsNamesMatchingClass(VertexClass vertexClass);
	std::vector<std::string> GetShadowMapMaterialsNamesMatchingClass(VertexClass vertexClass);
#if defined(_EDITOR)
	void SelectMaterial(std::string materialName, void* &ptr);
	void DrawMaterialPanel(void*& ptr, ImVec2 pos, ImVec2 size);
	std::string GetMaterialName(void* ptr);
#endif
	void BuildMaterialTextures(const std::shared_ptr<Renderer>& renderer, std::shared_ptr<Material>& material);
	
#if defined(_EDITOR)
	nlohmann::json json();
#endif
	Concurrency::task<void> json(std::string, nlohmann::json);

};
typedef Templates::Material::Material MaterialT;
typedef std::shared_ptr<MaterialT> MaterialPtr;
