#pragma once
#include <d3d12.h>

struct MaterialSamplerDesc : D3D12_STATIC_SAMPLER_DESC {
	MaterialSamplerDesc() {
		Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		MipLODBias = 0;
		MaxAnisotropy = 0;
		ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;//D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
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

	MaterialSamplerDesc(const D3D12_STATIC_SAMPLER_DESC& other) {
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

	nlohmann::json json()
	{
		return nlohmann::json(
			{
				{ "Filter" , filterToString[Filter] },
				{ "AddressU" , textureAddressModeToString[AddressU] },
				{ "AddressV" , textureAddressModeToString[AddressV] },
				{ "AddressW" , textureAddressModeToString[AddressW] },
				{ "MipLODBias" , MipLODBias },
				{ "MaxAnisotropy" , MaxAnisotropy },
				{ "ComparisonFunc" , comparisonFuncToString[ComparisonFunc] },
				{ "BorderColor" , borderColorToString[BorderColor] },
				{ "MinLOD" , MinLOD },
				{ "MaxLOD" , MaxLOD },
				{ "ShaderRegister" , ShaderRegister },
				{ "RegisterSpace" , RegisterSpace },
				{ "ShaderVisibility" , shaderVisibilityToString[ShaderVisibility] }
			}
		);
	}
};

template <>
struct std::hash<std::vector<MaterialSamplerDesc>>
{
	std::size_t operator()(const std::vector<MaterialSamplerDesc>& v) const
	{
		std::string s;
		for (auto& e : v)
		{
			s += filterToString[e.Filter];
			s += textureAddressModeToString[e.AddressU];
			s += textureAddressModeToString[e.AddressV];
			s += textureAddressModeToString[e.AddressW];
			s += std::to_string(e.MipLODBias);
			s += std::to_string(e.MaxAnisotropy);
			s += comparisonFuncToString[e.ComparisonFunc];
			s += borderColorToString[e.BorderColor];
			s += std::to_string(e.MinLOD);
			s += std::to_string(e.MaxLOD);
			s += std::to_string(e.ShaderRegister);
			s += std::to_string(e.RegisterSpace);
			s += shaderVisibilityToString[e.ShaderVisibility];
		}
		return hash<std::string>()(s);
	}
};


inline void TransformJsonToMaterialSamplers(std::vector<MaterialSamplerDesc>& samplers, nlohmann::json object, const std::string& key) {

	if (object.contains(key)) {
		nlohmann::json jsamplers = object[key];

		std::transform(jsamplers.begin(), jsamplers.end(), std::back_inserter(samplers), [](nlohmann::json sampler) {
			MaterialSamplerDesc desc;
			nostd::ReplaceFromJsonUsingMap(desc.Filter, stringToFilter, sampler, "Filter");
			nostd::ReplaceFromJsonUsingMap(desc.AddressU, stringToTextureAddressMode, sampler, "AddressU");
			nostd::ReplaceFromJsonUsingMap(desc.AddressV, stringToTextureAddressMode, sampler, "AddressV");
			nostd::ReplaceFromJsonUsingMap(desc.AddressW, stringToTextureAddressMode, sampler, "AddressW");
			ReplaceFromJson(desc.MipLODBias, sampler, "MipLODBias");
			ReplaceFromJson(desc.MaxAnisotropy, sampler, "MaxAnisotropy");
			nostd::ReplaceFromJsonUsingMap(desc.ComparisonFunc, stringToComparisonFunc, sampler, "ComparisonFunc");
			nostd::ReplaceFromJsonUsingMap(desc.BorderColor, stringToBorderColor, sampler, "BorderColor");
			ReplaceFromJson(desc.MinLOD, sampler, "MinLOD");
			ReplaceFromJson(desc.MaxLOD, sampler, "MaxLOD");
			ReplaceFromJson(desc.ShaderRegister, sampler, "ShaderRegister");
			ReplaceFromJson(desc.RegisterSpace, sampler, "RegisterSpace");
			nostd::ReplaceFromJsonUsingMap(desc.ShaderVisibility, stringToShaderVisibility, sampler, "ShaderVisibility");
			return desc;
			}
		);
	}
}