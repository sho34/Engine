#pragma once

#include <string>
#include "Shader.h"
#include "../Types.h"

using namespace Microsoft::WRL;
struct Renderer;

namespace Templates::Material {

	static const std::wstring templateName = L"materials.json";

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
			j["Filter"] = WStringToString(filterToString[Filter]);
			j["AddressU"] = WStringToString(textureAddressModeToString[AddressU]);
			j["AddressV"] = WStringToString(textureAddressModeToString[AddressV]);
			j["AddressW"] = WStringToString(textureAddressModeToString[AddressW]);
			j["MipLODBias"] = MipLODBias;
			j["MaxAnisotropy"] = MaxAnisotropy;
			j["ComparisonFunc"] = WStringToString(comparisonFuncToString[ComparisonFunc]);
			j["BorderColor"] = WStringToString(borderColorToString[BorderColor]);
			j["MinLOD"] = MinLOD;
			j["MaxLOD"] = MaxLOD;
			j["ShaderRegister"] = ShaderRegister;
			j["RegisterSpace"] = RegisterSpace;
			j["ShaderVisibility"] = WStringToString(shaderVisibilityToString[ShaderVisibility]);
			return j;
		}
#endif
	};

	struct TextureDefinition {
		std::wstring texturePath;
		DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		UINT numFrames = 0U;
	};

	struct MaterialDefinition {
		//vertex & pixel shader 
		std::wstring shaderTemplate = L"";

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
		std::wstring texturePath;
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
		std::wstring materialName;

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

	std::shared_ptr<Material>* CreateNewMaterial(std::wstring materialName, MaterialDefinition materialDefinition);
	void ReleaseMaterialTemplates();
	void BuildMaterialProperties(std::shared_ptr<Material>& material);

	Concurrency::task<void> CreateMaterialTemplate(std::wstring materialName, MaterialDefinition materialDefinition, LoadMaterialFn loadFn = nullptr);
	Concurrency::task<void> BindToMaterialTemplate(const std::wstring& materialName, void* target, NotificationCallbacks callbacks);
	std::shared_ptr<Material> GetMaterialTemplate(std::wstring materialName);
	std::shared_ptr<Material>* GetMaterialTemplatePtr(std::wstring materialName);
	void BuildMaterialTextures(const std::shared_ptr<Renderer>& renderer, std::shared_ptr<Material>& material);
	
#if defined(_EDITOR)
	nlohmann::json json();
#endif
	Concurrency::task<void> json(std::wstring, nlohmann::json);

};
typedef Templates::Material::Material MaterialT;
typedef std::shared_ptr<MaterialT> MaterialPtr;
