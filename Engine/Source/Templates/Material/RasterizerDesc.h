#pragma once
#include <d3d12.h>
#include <d3dx12.h>

struct RasterizerDesc : D3D12_RASTERIZER_DESC
{
	RasterizerDesc() : D3D12_RASTERIZER_DESC(CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT)) {}

	RasterizerDesc(const RasterizerDesc& other)
	{
		FillMode = other.FillMode;
		CullMode = other.CullMode;
		FrontCounterClockwise = other.FrontCounterClockwise;
		DepthBias = other.DepthBias;
		DepthBiasClamp = other.DepthBiasClamp;
		SlopeScaledDepthBias = other.SlopeScaledDepthBias;
		DepthClipEnable = other.DepthClipEnable;
		MultisampleEnable = other.MultisampleEnable;
		AntialiasedLineEnable = other.AntialiasedLineEnable;
		ForcedSampleCount = other.ForcedSampleCount;
		ConservativeRaster = other.ConservativeRaster;
	}

	RasterizerDesc(nlohmann::json& j) : D3D12_RASTERIZER_DESC(CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT))
	{
		nostd::ReplaceFromJsonUsingMap(FillMode, StringToD3D12_FILL_MODE, j, "FillMode");
		nostd::ReplaceFromJsonUsingMap(CullMode, StringToD3D12_CULL_MODE, j, "CullMode");
		ReplaceFromJson(FrontCounterClockwise, j, "FrontCounterClockwise");
		ReplaceFromJson(DepthBias, j, "DepthBias");
		ReplaceFromJson(DepthBiasClamp, j, "DepthBiasClamp");
		ReplaceFromJson(SlopeScaledDepthBias, j, "SlopeScaledDepthBias");
		ReplaceFromJson(DepthClipEnable, j, "DepthClipEnable");
		ReplaceFromJson(MultisampleEnable, j, "MultisampleEnable");
		ReplaceFromJson(AntialiasedLineEnable, j, "AntialiasedLineEnable");
		ReplaceFromJson(ForcedSampleCount, j, "ForcedSampleCount");
		nostd::ReplaceFromJsonUsingMap(ConservativeRaster, StringToD3D12_CONSERVATIVE_RASTERIZATION_MODE, j, "ConservativeRaster");
	}

	RasterizerDesc(const D3D12_RASTERIZER_DESC& other)
	{
		FillMode = other.FillMode;
		CullMode = other.CullMode;
		FrontCounterClockwise = other.FrontCounterClockwise;
		DepthBias = other.DepthBias;
		DepthBiasClamp = other.DepthBiasClamp;
		SlopeScaledDepthBias = other.SlopeScaledDepthBias;
		DepthClipEnable = other.DepthClipEnable;
		MultisampleEnable = other.MultisampleEnable;
		AntialiasedLineEnable = other.AntialiasedLineEnable;
		ForcedSampleCount = other.ForcedSampleCount;
		ConservativeRaster = other.ConservativeRaster;
	}

	bool operator<(const RasterizerDesc& other) const {
		return std::tie(
			FillMode, CullMode, FrontCounterClockwise, DepthBias, DepthBiasClamp,
			SlopeScaledDepthBias, DepthClipEnable, MultisampleEnable, AntialiasedLineEnable,
			ForcedSampleCount, ConservativeRaster)
			< std::tie(
				other.FillMode, other.CullMode, other.FrontCounterClockwise, other.DepthBias,
				other.DepthBiasClamp, other.SlopeScaledDepthBias, other.DepthClipEnable, other.MultisampleEnable,
				other.AntialiasedLineEnable, other.ForcedSampleCount, other.ConservativeRaster);
	}

	nlohmann::json json()
	{
		return {
			{ "FillMode", D3D12_FILL_MODEToString.at(FillMode) },
			{ "CullMode", D3D12_CULL_MODEToString.at(CullMode) },
			{ "FrontCounterClockwise", static_cast<bool>(!!FrontCounterClockwise) },
			{ "DepthBias", DepthBias },
			{ "DepthBiasClamp", DepthBiasClamp },
			{ "SlopeScaledDepthBias", SlopeScaledDepthBias },
			{ "DepthClipEnable", static_cast<bool>(!!DepthClipEnable) },
			{ "MultisampleEnable", static_cast<bool>(!!MultisampleEnable) },
			{ "AntialiasedLineEnable", static_cast<bool>(!!AntialiasedLineEnable) },
			{ "ForcedSampleCount", ForcedSampleCount },
			{ "ConservativeRaster", D3D12_CONSERVATIVE_RASTERIZATION_MODEToString.at(ConservativeRaster) }
		};
	}
};

inline RasterizerDesc ToRasterizerDesc(nlohmann::json j)
{
	return RasterizerDesc(j);
}

inline nlohmann::json FromRasterizerDesc(RasterizerDesc r)
{
	return r.json();
}