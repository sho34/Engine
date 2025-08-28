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

	MaterialSamplerDesc(nlohmann::json& sampler)
	{
		nostd::ReplaceFromJsonUsingMap(Filter, StringToD3D12_FILTER, sampler, "Filter");
		nostd::ReplaceFromJsonUsingMap(AddressU, StringToD3D12_TEXTURE_ADDRESS_MODE, sampler, "AddressU");
		nostd::ReplaceFromJsonUsingMap(AddressV, StringToD3D12_TEXTURE_ADDRESS_MODE, sampler, "AddressV");
		nostd::ReplaceFromJsonUsingMap(AddressW, StringToD3D12_TEXTURE_ADDRESS_MODE, sampler, "AddressW");
		ReplaceFromJson(MipLODBias, sampler, "MipLODBias");
		ReplaceFromJson(MaxAnisotropy, sampler, "MaxAnisotropy");
		nostd::ReplaceFromJsonUsingMap(ComparisonFunc, StringToD3D12_COMPARISON_FUNC, sampler, "ComparisonFunc");
		nostd::ReplaceFromJsonUsingMap(BorderColor, StringToD3D12_STATIC_BORDER_COLOR, sampler, "BorderColor");
		ReplaceFromJson(MinLOD, sampler, "MinLOD");
		ReplaceFromJson(MaxLOD, sampler, "MaxLOD");
		ReplaceFromJson(ShaderRegister, sampler, "ShaderRegister");
		ReplaceFromJson(RegisterSpace, sampler, "RegisterSpace");
		nostd::ReplaceFromJsonUsingMap(ShaderVisibility, StringToD3D12_SHADER_VISIBILITY, sampler, "ShaderVisibility");
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
				{ "Filter" , D3D12_FILTERToString[Filter] },
				{ "AddressU" , D3D12_TEXTURE_ADDRESS_MODEToString[AddressU] },
				{ "AddressV" , D3D12_TEXTURE_ADDRESS_MODEToString[AddressV] },
				{ "AddressW" , D3D12_TEXTURE_ADDRESS_MODEToString[AddressW] },
				{ "MipLODBias" , MipLODBias },
				{ "MaxAnisotropy" , MaxAnisotropy },
				{ "ComparisonFunc" , D3D12_COMPARISON_FUNCToString[ComparisonFunc] },
				{ "BorderColor" , D3D12_STATIC_BORDER_COLORToString[BorderColor] },
				{ "MinLOD" , MinLOD },
				{ "MaxLOD" , MaxLOD },
				{ "ShaderRegister" , ShaderRegister },
				{ "RegisterSpace" , RegisterSpace },
				{ "ShaderVisibility" , D3D12_SHADER_VISIBILITYToString[ShaderVisibility] }
			}
		);
	}
};

inline MaterialSamplerDesc ToMaterialSamplerDesc(nlohmann::json j)
{
	return MaterialSamplerDesc(j);
}

inline nlohmann::json FromMaterialSamplerDesc(MaterialSamplerDesc d)
{
	return d.json();
}

template <>
struct std::hash<std::vector<MaterialSamplerDesc>>
{
	std::size_t operator()(const std::vector<MaterialSamplerDesc>& v) const
	{
		std::string s;
		for (auto& e : v)
		{
			s += D3D12_FILTERToString[e.Filter];
			s += D3D12_TEXTURE_ADDRESS_MODEToString[e.AddressU];
			s += D3D12_TEXTURE_ADDRESS_MODEToString[e.AddressV];
			s += D3D12_TEXTURE_ADDRESS_MODEToString[e.AddressW];
			s += std::to_string(e.MipLODBias);
			s += std::to_string(e.MaxAnisotropy);
			s += D3D12_COMPARISON_FUNCToString[e.ComparisonFunc];
			s += D3D12_STATIC_BORDER_COLORToString[e.BorderColor];
			s += std::to_string(e.MinLOD);
			s += std::to_string(e.MaxLOD);
			s += std::to_string(e.ShaderRegister);
			s += std::to_string(e.RegisterSpace);
			s += D3D12_SHADER_VISIBILITYToString[e.ShaderVisibility];
		}
		return hash<std::string>()(s);
	}

	std::size_t operator()(const nlohmann::json& v, std::string attribute) const
	{
		std::vector<MaterialSamplerDesc> sv;
		for (unsigned int i = 0; i < v.at(attribute).size(); i++)
		{
			sv.push_back(ToMaterialSamplerDesc(v.at(attribute).at(i)));
		}
		return operator()(sv);
	}
};

/*
inline void TransformJsonToMaterialSamplers(std::vector<MaterialSamplerDesc>& samplers, nlohmann::json object, const std::string& key) {

	if (object.contains(key)) {
		nlohmann::json jsamplers = object[key];

		std::transform(jsamplers.begin(), jsamplers.end(), std::back_inserter(samplers), [](nlohmann::json sampler) {
			MaterialSamplerDesc desc;
			nostd::ReplaceFromJsonUsingMap(desc.Filter, StringToD3D12_FILTER, sampler, "Filter");
			nostd::ReplaceFromJsonUsingMap(desc.AddressU, StringToD3D12_TEXTURE_ADDRESS_MODE, sampler, "AddressU");
			nostd::ReplaceFromJsonUsingMap(desc.AddressV, StringToD3D12_TEXTURE_ADDRESS_MODE, sampler, "AddressV");
			nostd::ReplaceFromJsonUsingMap(desc.AddressW, StringToD3D12_TEXTURE_ADDRESS_MODE, sampler, "AddressW");
			ReplaceFromJson(desc.MipLODBias, sampler, "MipLODBias");
			ReplaceFromJson(desc.MaxAnisotropy, sampler, "MaxAnisotropy");
			nostd::ReplaceFromJsonUsingMap(desc.ComparisonFunc, StringToD3D12_COMPARISON_FUNC, sampler, "ComparisonFunc");
			nostd::ReplaceFromJsonUsingMap(desc.BorderColor, StringToD3D12_STATIC_BORDER_COLOR, sampler, "BorderColor");
			ReplaceFromJson(desc.MinLOD, sampler, "MinLOD");
			ReplaceFromJson(desc.MaxLOD, sampler, "MaxLOD");
			ReplaceFromJson(desc.ShaderRegister, sampler, "ShaderRegister");
			ReplaceFromJson(desc.RegisterSpace, sampler, "RegisterSpace");
			nostd::ReplaceFromJsonUsingMap(desc.ShaderVisibility, StringToD3D12_SHADER_VISIBILITY, sampler, "ShaderVisibility");
			return desc;
			}
		);
	}
}
*/