#include "pch.h"
#include "PipelineState.h"
#include "../../Renderer.h"
#include "../../VertexFormats.h"
#include "../../../Common/DirectXHelper.h"
#include <NoStd.h>
#include <ios>
#include "../RootSignature/RootSignature.h"

extern std::shared_ptr<Renderer> renderer;

template <>
struct std::hash<GraphicsPipelineStateDesc>
{
	std::size_t operator()(const GraphicsPipelineStateDesc& p) const
	{
		using std::hash;
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputElementDescs = std::get<0>(p);
		const ShaderByteCode& vs = std::get<1>(p);
		const ShaderByteCode& ps = std::get<2>(p);
		size_t rootSignatureHash = std::get<3>(p);
		const D3D12_BLEND_DESC& BlendState = std::get<4>(p);
		const D3D12_RASTERIZER_DESC& RasterizerState = std::get<5>(p);
		const D3D12_PRIMITIVE_TOPOLOGY_TYPE& PrimitiveTopologyType = std::get<6>(p);
		const std::vector<DXGI_FORMAT>& renderTargetsFormats = std::get<7>(p);
		const DXGI_FORMAT& depthStencilForma = std::get<8>(p);

		size_t h = 0ULL;
		nostd::hash_combine(h,
			inputElementDescs,
			std::hash<std::string_view>{}({ reinterpret_cast<const char*>(vs.data()), vs.size() }),
			std::hash<std::string_view>{}({ reinterpret_cast<const char*>(ps.data()), ps.size() }),
			rootSignatureHash, BlendState, RasterizerState,
			PrimitiveTopologyType, renderTargetsFormats, depthStencilForma
		);
		return h;
	}
};

template<>
struct std::hash<ComputePipelineStateDesc>
{
	std::size_t operator()(const ComputePipelineStateDesc& p) const
	{
		using std::hash;
		const ShaderByteCode& cs = std::get<0>(p);
		size_t rootSignatureHash = std::get<1>(p);

		size_t h = 0ULL;
		nostd::hash_combine(h,
			std::hash<std::string_view>{}({ reinterpret_cast<const char*>(cs.data()), cs.size() }),
			rootSignatureHash
		);
		return h;
	}
};

namespace DeviceUtils
{
	static nostd::RefTracker<size_t, HashedPipelineState> refTracker;

	HashedPipelineState CreateGraphicsPipelineState(GraphicsPipelineStateDesc& p)
	{
		size_t hash = std::hash<GraphicsPipelineStateDesc>()(p);
		return refTracker.AddRef(hash, [hash, &p]()
			{
				CComPtr<ID3D12RootSignature> rootSignature = GetRootSignature(std::get<3>(p));
				return std::make_tuple(hash,
					CreateGraphicsPipelineState(
						std::to_string(hash),
						std::get<0>(p),
						std::get<1>(p),
						std::get<2>(p),
						rootSignature,
						std::get<4>(p),
						std::get<5>(p),
						std::get<6>(p),
						std::get<7>(p),
						std::get<8>(p)
					)
				);
			}
		);
	}

	CComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(
		std::string name,
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout,
		ShaderByteCode& vsCode,
		ShaderByteCode& psCode,
		CComPtr<ID3D12RootSignature>& rootSignature,
		D3D12_BLEND_DESC& BlendState,
		D3D12_RASTERIZER_DESC& RasterizerState,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE& PrimitiveTopologyType,
		std::vector<DXGI_FORMAT>& renderTargetsFormats,
		DXGI_FORMAT& depthStencilFormat
	)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};

		//input layout
		state.InputLayout = { inputLayout.data(), static_cast<UINT>(inputLayout.size()) };

		//root signature
		state.pRootSignature = rootSignature;

		//shader based state
		state.VS = CD3DX12_SHADER_BYTECODE(vsCode.data(), vsCode.size());
		state.PS = CD3DX12_SHADER_BYTECODE(psCode.data(), psCode.size());

		//material based state
		state.RasterizerState = RasterizerState;
		state.BlendState = BlendState;
		state.SampleDesc.Count = 1;

		//render target based
		state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		if (depthStencilFormat == DXGI_FORMAT_UNKNOWN) { state.DepthStencilState.DepthEnable = false; }
		state.SampleMask = UINT_MAX;
		state.PrimitiveTopologyType = PrimitiveTopologyType;
		state.NumRenderTargets = static_cast<UINT>(max(1, renderTargetsFormats.size()));
		state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		for (unsigned int i = 0; i < renderTargetsFormats.size(); i++)
		{
			state.RTVFormats[i] = renderTargetsFormats[i];
		}
		state.DSVFormat = depthStencilFormat;

		CComPtr<ID3D12PipelineState> pipelineState;
		DX::ThrowIfFailed(renderer->d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&pipelineState)));
		CCNAME_D3D12_OBJECT(pipelineState);

		LogCComPtrAddress(name, pipelineState);

		return pipelineState;
	}

	HashedPipelineState CreateComputePipelineState(ComputePipelineStateDesc& p)
	{
		size_t hash = std::hash<ComputePipelineStateDesc>()(p);
		return refTracker.AddRef(hash, [hash, &p]()
			{
				CComPtr<ID3D12RootSignature> rootSignature = GetRootSignature(std::get<1>(p));
				return std::make_tuple(hash,
					CreateComputePipelineState(
						std::to_string(hash),
						std::get<0>(p),
						rootSignature
					)
				);
			}
		);
	}

	CComPtr<ID3D12PipelineState> CreateComputePipelineState(std::string name, ShaderByteCode& csCode, CComPtr<ID3D12RootSignature>& rootSignature)
	{
		struct PipelineStateStream
		{
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE RootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_CS CS;
		} PSS;

		PSS.RootSignature = rootSignature;
		PSS.CS = CD3DX12_SHADER_BYTECODE(csCode.data(), csCode.size());

		D3D12_PIPELINE_STATE_STREAM_DESC pssDescription = { sizeof(PSS), &PSS };
		CComPtr<ID3D12PipelineState> pipelineState;
		DX::ThrowIfFailed(renderer->d3dDevice->CreatePipelineState(&pssDescription, IID_PPV_ARGS(&pipelineState)));
		CCNAME_D3D12_OBJECT(pipelineState);

		LogCComPtrAddress(name, pipelineState);

		return pipelineState;
	}

#if defined(_EDITOR)
	std::map<std::string, std::function<void(nlohmann::json&)>> addToBlendDescRenderTarget = {
		{ "BlendEnable", [](nlohmann::json& RenderTargetBlendDesc) { RenderTargetBlendDesc["BlendEnable"] = false; }},
		{ "LogicOpEnable", [](nlohmann::json& RenderTargetBlendDesc) { RenderTargetBlendDesc["LogicOpEnable"] = false; }},
		{ "SrcBlend", [](nlohmann::json& RenderTargetBlendDesc) { RenderTargetBlendDesc["SrcBlend"] = blendToString.at(D3D12_BLEND_ONE); }},
		{ "DestBlend", [](nlohmann::json& RenderTargetBlendDesc) { RenderTargetBlendDesc["DestBlend"] = blendToString.at(D3D12_BLEND_ZERO); }},
		{ "BlendOp", [](nlohmann::json& RenderTargetBlendDesc) { RenderTargetBlendDesc["BlendOp"] = blendOpToString.at(D3D12_BLEND_OP_ADD); }},
		{ "SrcBlendAlpha", [](nlohmann::json& RenderTargetBlendDesc) { RenderTargetBlendDesc["SrcBlendAlpha"] = blendToString.at(D3D12_BLEND_ONE); }},
		{ "DestBlendAlpha", [](nlohmann::json& RenderTargetBlendDesc) { RenderTargetBlendDesc["DestBlendAlpha"] = blendToString.at(D3D12_BLEND_ZERO); }},
		{ "BlendOpAlpha", [](nlohmann::json& RenderTargetBlendDesc) { RenderTargetBlendDesc["BlendOpAlpha"] = blendOpToString.at(D3D12_BLEND_OP_ADD); }},
		{ "LogicOp", [](nlohmann::json& RenderTargetBlendDesc) { RenderTargetBlendDesc["LogicOp"] = logicOpToString.at(D3D12_LOGIC_OP_NOOP); }},
		{ "RenderTargetWriteMask", [](nlohmann::json& RenderTargetBlendDesc) { RenderTargetBlendDesc["RenderTargetWriteMask"] = D3D12_COLOR_WRITE_ENABLE_ALL; }},
	};

	std::map<std::string, std::function<void(nlohmann::json&)>> drawBlendDescRenderTarget = {
		{ "BlendEnable", [](nlohmann::json& BlendDesc) { drawFromCheckBox(BlendDesc,"BlendEnable"); }},
		{ "LogicOpEnable", [](nlohmann::json& BlendDesc) { drawFromCheckBox(BlendDesc,"LogicOpEnable"); }},
		{ "BlendEnable", [](nlohmann::json& BlendDesc) { drawFromCheckBox(BlendDesc, "BlendEnable"); } },
		{ "LogicOpEnable", [](nlohmann::json& BlendDesc) { drawFromCheckBox(BlendDesc,"LogicOpEnable"); } },
		{ "SrcBlend", [](nlohmann::json& BlendDesc) { drawFromCombo(BlendDesc,"SrcBlend",stringToBlend); } },
		{ "DestBlend", [](nlohmann::json& BlendDesc) { drawFromCombo(BlendDesc,"DestBlend",stringToBlend); } },
		{ "BlendOp", [](nlohmann::json& BlendDesc) { drawFromCombo(BlendDesc,"BlendOp",stringToBlendOp); } },
		{ "SrcBlendAlpha", [](nlohmann::json& BlendDesc) { drawFromCombo(BlendDesc,"SrcBlendAlpha",stringToBlend); } },
		{ "DestBlendAlpha", [](nlohmann::json& BlendDesc) { drawFromCombo(BlendDesc,"DestBlendAlpha",stringToBlend); } },
		{ "BlendOpAlpha", [](nlohmann::json& BlendDesc) { drawFromCombo(BlendDesc,"BlendOpAlpha",stringToBlendOp); } },
		{ "LogicOp", [](nlohmann::json& BlendDesc) { drawFromCombo(BlendDesc,"LogicOp",stringToLogicOp); } },
		{ "RenderTargetWriteMask", [](nlohmann::json& BlendDesc) { drawFromUInt(BlendDesc,"RenderTargetWriteMask"); } },
	};

	void ImDrawBlendStatesRenderTargets(nlohmann::json& BlendState)
	{
		ImDrawDynamicArray(
			"BLEND#RT",
			BlendState.at("RenderTarget"),
			[](nlohmann::json& RenderTarget, unsigned int index)
			{
				auto pos = RenderTarget.begin() + index + 1;
				RenderTarget.insert(pos, nlohmann::json::object());
			},
			[](nlohmann::json& RTBlendDesc, unsigned int index)
			{
				std::string label = (std::string("BLEND#") + std::to_string(index + 1));
				ImGui::Text(label.c_str());
				ImDrawObject(
					label,
					RTBlendDesc,
					addToBlendDescRenderTarget,
					drawBlendDescRenderTarget
				);
			},
			_countof(D3D12_BLEND_DESC::RenderTarget),
			1U
		);
	}

	std::map<std::string, std::function<void(nlohmann::json&)>> addToBlendState = {
		{ "AlphaToCoverageEnable", [](nlohmann::json& BlendState) { BlendState["AlphaToCoverageEnable"] = false; } },
		{ "IndependentBlendEnable", [](nlohmann::json& BlendState) { BlendState["IndependentBlendEnable"] = false; } },
		{ "RenderTarget", [](nlohmann::json& BlendState) { BlendState["RenderTarget"] = nlohmann::json::array({ nlohmann::json::object() }); }},
	};

	std::map<std::string, std::function<void(nlohmann::json&)>> drawBlendState = {
		{ "AlphaToCoverageEnable", [](nlohmann::json& BlendState) { drawFromCheckBox(BlendState,"AlphaToCoverageEnable"); } },
		{ "IndependentBlendEnable", [](nlohmann::json& BlendState) { drawFromCheckBox(BlendState,"IndependentBlendEnable"); } },
		{ "RenderTarget", ImDrawBlendStatesRenderTargets },
	};

	std::map<std::string, std::function<void(nlohmann::json&)>> addToRasterizerState = {
		{ "FillMode", [](nlohmann::json& RasterizerState) { RasterizerState["FillMode"] = fillModeToString.at(D3D12_FILL_MODE_SOLID); }},
		{ "CullMode", [](nlohmann::json& RasterizerState) { RasterizerState["CullMode"] = cullModeToString.at(D3D12_CULL_MODE_BACK); } },
		{ "FrontCounterClockwise", [](nlohmann::json& RasterizerState) { RasterizerState["FrontCounterClockwise"] = false; } },
		{ "DepthBias", [](nlohmann::json& RasterizerState) { RasterizerState["DepthBias"] = D3D12_DEFAULT_DEPTH_BIAS; } },
		{ "DepthBiasClamp", [](nlohmann::json& RasterizerState) { RasterizerState["DepthBiasClamp"] = D3D12_DEFAULT_DEPTH_BIAS_CLAMP; } },
		{ "SlopeScaledDepthBias", [](nlohmann::json& RasterizerState) { RasterizerState["SlopeScaledDepthBias"] = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS; } },
		{ "DepthClipEnable", [](nlohmann::json& RasterizerState) { RasterizerState["DepthClipEnable"] = true; } },
		{ "MultisampleEnable", [](nlohmann::json& RasterizerState) { RasterizerState["MultisampleEnable"] = false; } },
		{ "AntialiasedLineEnable", [](nlohmann::json& RasterizerState) { RasterizerState["AntialiasedLineEnable"] = false; } },
		{ "ForcedSampleCount", [](nlohmann::json& RasterizerState) { RasterizerState["ForcedSampleCount"] = 0; } },
		{ "ConservativeRaster", [](nlohmann::json& RasterizerState) { RasterizerState["ConservativeRaster"] = conservativeRasterizationModeToString.at(D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF); } },
	};

	std::map<std::string, std::function<void(nlohmann::json&)>> drawRasterizerState = {
		{ "FillMode", [](nlohmann::json& RasterizerState) { drawFromCombo(RasterizerState, "FillMode", stringToFillMode); }},
		{ "CullMode", [](nlohmann::json& RasterizerState) { drawFromCombo(RasterizerState, "CullMode", stringToCullMode); } },
		{ "FrontCounterClockwise", [](nlohmann::json& RasterizerState) { drawFromCheckBox(RasterizerState,"FrontCounterClockwise"); } },
		{ "DepthBias", [](nlohmann::json& RasterizerState) { drawFromInt(RasterizerState,"DepthBias"); } },
		{ "DepthBiasClamp", [](nlohmann::json& RasterizerState) { drawFromFloat(RasterizerState,"DepthBiasClamp"); } },
		{ "SlopeScaledDepthBias", [](nlohmann::json& RasterizerState) { drawFromFloat(RasterizerState,"SlopeScaledDepthBias"); } },
		{ "DepthClipEnable", [](nlohmann::json& RasterizerState) { drawFromCheckBox(RasterizerState,"DepthClipEnable"); } },
		{ "MultisampleEnable", [](nlohmann::json& RasterizerState) { drawFromCheckBox(RasterizerState,"MultisampleEnable"); } },
		{ "AntialiasedLineEnable", [](nlohmann::json& RasterizerState) { drawFromCheckBox(RasterizerState,"AntialiasedLineEnable"); } },
		{ "ForcedSampleCount", [](nlohmann::json& RasterizerState) { drawFromUInt(RasterizerState,"ForcedSampleCount"); } },
		{ "ConservativeRaster", [](nlohmann::json& RasterizerState) { drawFromCombo(RasterizerState,"ConservativeRaster",stringToConservativeRasterizationMode); } },
	};

	std::map<std::string, std::function<void(nlohmann::json&)>> addToPipelineState = {
		{ "blendState", [](nlohmann::json& json) { json["blendState"] = nlohmann::json::object(); } },
		{ "rasterizerState", [](nlohmann::json& json) { json["rasterizerState"] = nlohmann::json::object(); } },
		{ "primitiveTopologyType", [](nlohmann::json& json) { json["primitiveTopologyType"] = primitiveTopologyTypeToString.at(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE); } },
	};

	std::map<std::string, std::function<void(nlohmann::json&)>> drawPipelineState = {
		{ "blendState", [](nlohmann::json& json)
			{
				ImDrawObject("BlendState", json.at("blendState"), addToBlendState, drawBlendState);
			}
		},
		{ "rasterizerState", [](nlohmann::json& json)
			{
				ImDrawObject("RasterizerState", json.at("rasterizerState"), addToRasterizerState, drawRasterizerState);
			}
		},
		{ "primitiveTopologyType", [](nlohmann::json& json) { drawFromCombo(json, "primitiveTopologyType", stringToPrimitiveTopologyType); } },
	};

	void ImDrawGraphicsPipelineState(nlohmann::json& json)
	{
		ImDrawObject("PipelineState", json, addToPipelineState, drawPipelineState);
	}
#endif
};
