#pragma once
#include <d3d12.h>
#include <d3dx12.h>

struct RenderTargetBlendDesc : D3D12_RENDER_TARGET_BLEND_DESC
{
	RenderTargetBlendDesc() : D3D12_RENDER_TARGET_BLEND_DESC({
		FALSE, FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		}) {
	}

	RenderTargetBlendDesc(const RenderTargetBlendDesc& other)
	{
		BlendEnable = other.BlendEnable;
		LogicOpEnable = other.LogicOpEnable;
		SrcBlend = other.SrcBlend;
		DestBlend = other.DestBlend;
		BlendOp = other.BlendOp;
		SrcBlendAlpha = other.SrcBlendAlpha;
		DestBlendAlpha = other.DestBlendAlpha;
		BlendOpAlpha = other.BlendOpAlpha;
		BlendOpAlpha = other.BlendOpAlpha;
		LogicOp = other.LogicOp;
		RenderTargetWriteMask = other.RenderTargetWriteMask;
	}

	RenderTargetBlendDesc(nlohmann::json& j)
	{
		ReplaceFromJson(BlendEnable, j, "BlendEnable");
		ReplaceFromJson(LogicOpEnable, j, "LogicOpEnable");
		nostd::ReplaceFromJsonUsingMap(SrcBlend, StringToD3D12_BLEND, j, "SrcBlend");
		nostd::ReplaceFromJsonUsingMap(DestBlend, StringToD3D12_BLEND, j, "DestBlend");
		nostd::ReplaceFromJsonUsingMap(BlendOp, StringToD3D12_BLEND_OP, j, "BlendOp");
		nostd::ReplaceFromJsonUsingMap(SrcBlendAlpha, StringToD3D12_BLEND, j, "SrcBlendAlpha");
		nostd::ReplaceFromJsonUsingMap(DestBlendAlpha, StringToD3D12_BLEND, j, "DestBlendAlpha");
		nostd::ReplaceFromJsonUsingMap(BlendOpAlpha, StringToD3D12_BLEND_OP, j, "BlendOpAlpha");
		nostd::ReplaceFromJsonUsingMap(BlendOpAlpha, StringToD3D12_BLEND_OP, j, "BlendOpAlpha");
		nostd::ReplaceFromJsonUsingMap(LogicOp, StringToD3D12_LOGIC_OP, j, "LogicOp");
		ReplaceFromJson(RenderTargetWriteMask, j, "RenderTargetWriteMask");
	}

	RenderTargetBlendDesc(const D3D12_RENDER_TARGET_BLEND_DESC& other)
	{
		BlendEnable = other.BlendEnable;
		LogicOpEnable = other.LogicOpEnable;
		SrcBlend = other.SrcBlend;
		DestBlend = other.DestBlend;
		BlendOp = other.BlendOp;
		SrcBlendAlpha = other.SrcBlendAlpha;
		DestBlendAlpha = other.DestBlendAlpha;
		BlendOpAlpha = other.BlendOpAlpha;
		LogicOp = other.LogicOp;
		RenderTargetWriteMask = other.RenderTargetWriteMask;
	}

	bool operator<(const RenderTargetBlendDesc& other) const
	{
		return std::tie(
			BlendEnable, LogicOpEnable, SrcBlend, DestBlend, BlendOp,
			SrcBlendAlpha, DestBlendAlpha, BlendOpAlpha,
			LogicOp, RenderTargetWriteMask) < std::tie(
				other.BlendEnable, other.LogicOpEnable, other.SrcBlend, other.DestBlend,
				other.BlendOp, other.SrcBlendAlpha, other.DestBlendAlpha,
				other.BlendOpAlpha, other.LogicOp, other.RenderTargetWriteMask
			);
	}

	nlohmann::json json()
	{
		nlohmann::json j =
		{
			{ "BlendEnable", BlendEnable },
			{ "LogicOpEnable", LogicOpEnable },
			{ "SrcBlend", D3D12_BLENDToString.at(SrcBlend) },
			{ "DestBlend", D3D12_BLENDToString.at(DestBlend) },
			{ "BlendOp", D3D12_BLEND_OPToString.at(BlendOp) },
			{ "SrcBlendAlpha", D3D12_BLENDToString.at(SrcBlendAlpha) } ,
			{ "DestBlendAlpha", D3D12_BLENDToString.at(DestBlendAlpha) },
			{ "BlendOpAlpha", D3D12_BLEND_OPToString.at(BlendOpAlpha) },
			{ "LogicOp", D3D12_LOGIC_OPToString.at(LogicOp) },
			{ "RenderTargetWriteMask", RenderTargetWriteMask }
		};
		return j;
	}
};

template <>
struct std::hash<std::vector<RenderTargetBlendDesc>>
{
	std::size_t operator()(const std::vector<RenderTargetBlendDesc>& v) const
	{
		std::string s;
		for (auto& e : v)
		{
			s += RenderTargetBlendDesc(e).json().dump();
		}
		return hash<std::string>()(s);
	}

	std::size_t operator()(const nlohmann::json& v, std::string attribute) const
	{
		std::vector<RenderTargetBlendDesc> sv;
		for (unsigned int i = 0; i < v.at(attribute).size(); i++)
		{
			nlohmann::json j = v.at(attribute).at(i);
			sv.push_back(j);
		}
		return operator()(sv);
	}
};

struct BlendDesc : D3D12_BLEND_DESC
{
	BlendDesc() : D3D12_BLEND_DESC(CD3DX12_BLEND_DESC(D3D12_DEFAULT)) {}

	BlendDesc(const BlendDesc& other)
	{
		/*
		auto FromRenderTargetBlendDesc = [](D3D12_RENDER_TARGET_BLEND_DESC& rtDst, D3D12_RENDER_TARGET_BLEND_DESC rtSrc)
			{
				rtDst.BlendEnable = rtSrc.BlendEnable;
				rtDst.LogicOpEnable = rtSrc.LogicOpEnable;
				rtDst.SrcBlend = rtSrc.SrcBlend;
				rtDst.DestBlend = rtSrc.DestBlend;
				rtDst.BlendOp = rtSrc.BlendOp;
				rtDst.SrcBlendAlpha = rtSrc.SrcBlendAlpha;
				rtDst.DestBlendAlpha = rtSrc.DestBlendAlpha;
				rtDst.BlendOpAlpha = rtSrc.BlendOpAlpha;
				rtDst.LogicOp = rtSrc.LogicOp;
				rtDst.RenderTargetWriteMask = rtSrc.RenderTargetWriteMask;
			};
		*/

		AlphaToCoverageEnable = other.AlphaToCoverageEnable;
		IndependentBlendEnable = other.IndependentBlendEnable;
		for (unsigned int i = 0; i < _countof(other.RenderTarget); i++)
		{
			RenderTarget[i] = RenderTargetBlendDesc(other.RenderTarget[i]);
			//FromRenderTargetBlendDesc(RenderTarget[i], other.RenderTarget[i]);
		}
	}

	BlendDesc(nlohmann::json& j)
	{
		/*
		auto ToRenderTargetBlendDesc = [](D3D12_RENDER_TARGET_BLEND_DESC& rt, nlohmann::json j)
			{
				ReplaceFromJson(rt.BlendEnable, j, "BlendEnable");
				ReplaceFromJson(rt.LogicOpEnable, j, "LogicOpEnable");
				nostd::ReplaceFromJsonUsingMap(rt.SrcBlend, StringToD3D12_BLEND, j, "SrcBlend");
				nostd::ReplaceFromJsonUsingMap(rt.DestBlend, StringToD3D12_BLEND, j, "DestBlend");
				nostd::ReplaceFromJsonUsingMap(rt.BlendOp, StringToD3D12_BLEND_OP, j, "BlendOp");
				nostd::ReplaceFromJsonUsingMap(rt.SrcBlendAlpha, StringToD3D12_BLEND, j, "SrcBlendAlpha");
				nostd::ReplaceFromJsonUsingMap(rt.DestBlendAlpha, StringToD3D12_BLEND, j, "DestBlendAlpha");
				nostd::ReplaceFromJsonUsingMap(rt.BlendOpAlpha, StringToD3D12_BLEND_OP, j, "BlendOpAlpha");
				nostd::ReplaceFromJsonUsingMap(rt.LogicOp, StringToD3D12_LOGIC_OP, j, "LogicOp");
				ReplaceFromJson(rt.RenderTargetWriteMask, j, "RenderTargetWriteMask");
			};
		*/
		ReplaceFromJson(AlphaToCoverageEnable, j, "AlphaToCoverageEnable");
		ReplaceFromJson(IndependentBlendEnable, j, "IndependentBlendEnable");
		if (j.contains("RenderTarget") && !j["RenderTarget"].empty())
		{
			for (unsigned int i = 0U; i < j.count("RenderTarget"); i++)
			{
				RenderTarget[i] = RenderTargetBlendDesc(j["RenderTarget"][i]);
				//ToRenderTargetBlendDesc(RenderTarget[i], j["RenderTarget"][i]);
			}
		}
	}

	BlendDesc(const D3D12_BLEND_DESC& other)
	{
		/*
		auto FromRenderTargetBlendDesc = [](D3D12_RENDER_TARGET_BLEND_DESC& rtDst, D3D12_RENDER_TARGET_BLEND_DESC rtSrc)
			{
				rtDst.BlendEnable = rtSrc.BlendEnable;
				rtDst.LogicOpEnable = rtSrc.LogicOpEnable;
				rtDst.SrcBlend = rtSrc.SrcBlend;
				rtDst.DestBlend = rtSrc.DestBlend;
				rtDst.BlendOp = rtSrc.BlendOp;
				rtDst.SrcBlendAlpha = rtSrc.SrcBlendAlpha;
				rtDst.DestBlendAlpha = rtSrc.DestBlendAlpha;
				rtDst.BlendOpAlpha = rtSrc.BlendOpAlpha;
				rtDst.LogicOp = rtSrc.LogicOp;
				rtDst.RenderTargetWriteMask = rtSrc.RenderTargetWriteMask;
			};
		*/

		AlphaToCoverageEnable = other.AlphaToCoverageEnable;
		IndependentBlendEnable = other.IndependentBlendEnable;
		for (unsigned int i = 0; i < _countof(other.RenderTarget); i++)
		{
			RenderTarget[i] = RenderTargetBlendDesc(other.RenderTarget[i]);
			//FromRenderTargetBlendDesc(RenderTarget[i], other.RenderTarget[i]);
		}
	}

	bool operator<(const BlendDesc& other) const
	{
		RenderTargetBlendDesc rt0 = RenderTargetBlendDesc(RenderTarget[0]);
		RenderTargetBlendDesc rt1 = RenderTargetBlendDesc(RenderTarget[1]);
		RenderTargetBlendDesc rt2 = RenderTargetBlendDesc(RenderTarget[2]);
		RenderTargetBlendDesc rt3 = RenderTargetBlendDesc(RenderTarget[3]);
		RenderTargetBlendDesc rt4 = RenderTargetBlendDesc(RenderTarget[4]);
		RenderTargetBlendDesc rt5 = RenderTargetBlendDesc(RenderTarget[5]);
		RenderTargetBlendDesc rt6 = RenderTargetBlendDesc(RenderTarget[6]);
		RenderTargetBlendDesc rt7 = RenderTargetBlendDesc(RenderTarget[7]);
		RenderTargetBlendDesc rto0 = RenderTargetBlendDesc(other.RenderTarget[0]);
		RenderTargetBlendDesc rto1 = RenderTargetBlendDesc(other.RenderTarget[1]);
		RenderTargetBlendDesc rto2 = RenderTargetBlendDesc(other.RenderTarget[2]);
		RenderTargetBlendDesc rto3 = RenderTargetBlendDesc(other.RenderTarget[3]);
		RenderTargetBlendDesc rto4 = RenderTargetBlendDesc(other.RenderTarget[4]);
		RenderTargetBlendDesc rto5 = RenderTargetBlendDesc(other.RenderTarget[5]);
		RenderTargetBlendDesc rto6 = RenderTargetBlendDesc(other.RenderTarget[6]);
		RenderTargetBlendDesc rto7 = RenderTargetBlendDesc(other.RenderTarget[7]);

		return std::tie(AlphaToCoverageEnable, IndependentBlendEnable, rt0, rt1, rt2, rt3, rt4, rt5, rt6, rt7)
			< std::tie(other.AlphaToCoverageEnable, other.IndependentBlendEnable, rto0, rto1, rto2, rto3, rto4, rto5, rto6, rto7);
	}

	nlohmann::json json()
	{
		/*
		auto FromRenderTargetBlendDesc = [](nlohmann::json& j, D3D12_RENDER_TARGET_BLEND_DESC rt)
			{
				j["BlendEnable"] = rt.BlendEnable;
				j["LogicOpEnable"] = rt.LogicOpEnable;
				j["SrcBlend"] = D3D12_BLENDToString.at(rt.SrcBlend);
				j["DestBlend"] = D3D12_BLENDToString.at(rt.DestBlend);
				j["BlendOp"] = D3D12_BLEND_OPToString.at(rt.BlendOp);
				j["SrcBlendAlpha"] = D3D12_BLENDToString.at(rt.SrcBlendAlpha);
				j["DestBlendAlpha"] = D3D12_BLENDToString.at(rt.DestBlendAlpha);
				j["BlendOpAlpha"] = D3D12_BLEND_OPToString.at(rt.BlendOpAlpha);
				j["LogicOp"] = D3D12_LOGIC_OPToString.at(rt.LogicOp);
				j["RenderTargetWriteMask"] = rt.RenderTargetWriteMask;
			};
		*/
		nlohmann::json j;
		j["AlphaToCoverageEnable"] = AlphaToCoverageEnable;
		j["IndependentBlendEnable"] = IndependentBlendEnable;
		j["RenderTarget"] = nlohmann::json::array();

		for (unsigned int i = 0; i < _countof(RenderTarget); i++)
		{
			j["RenderTarget"].push_back(RenderTargetBlendDesc(RenderTarget[i]).json());
			/*
			j["RenderTarget"].push_back({});
			FromRenderTargetBlendDesc(j["RenderTarget"].at(i), RenderTarget[i]);
			*/
		}
		return j;
	}
};

inline BlendDesc ToBlendDesc(nlohmann::json j)
{
	return BlendDesc(j);
}

inline nlohmann::json FromBlendDesc(BlendDesc b)
{
	return b.json();
}

