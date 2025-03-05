#include "pch.h"
#include "PipelineState.h"
#include "../../Renderer.h"
#include "../../VertexFormats.h"
#include "../../../Common/DirectXHelper.h"
#include <ios>

extern std::shared_ptr<Renderer> renderer;
namespace DeviceUtils
{
	CComPtr<ID3D12PipelineState> CreatePipelineState(
		std::string name,
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout,
		ShaderByteCode& vsCode,
		ShaderByteCode& psCode,
		CComPtr<ID3D12RootSignature>& rootSignature,
		const RenderablePipelineState& renderablePipelineState
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
		state.RasterizerState = renderablePipelineState.RasterizerState;
		state.BlendState = renderablePipelineState.BlendState;
		state.SampleDesc.Count = 1;

		//render target based
		state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		if (renderablePipelineState.depthStencilFormat == DXGI_FORMAT_UNKNOWN) { state.DepthStencilState.DepthEnable = false; }
		state.SampleMask = UINT_MAX;
		state.PrimitiveTopologyType = renderablePipelineState.PrimitiveTopologyType;
		state.NumRenderTargets = static_cast<UINT>(max(1, renderablePipelineState.renderTargetsFormats.size()));
		state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		for (unsigned int i = 0; i < renderablePipelineState.renderTargetsFormats.size(); i++)
		{
			state.RTVFormats[i] = renderablePipelineState.renderTargetsFormats[i];
		}
		state.DSVFormat = renderablePipelineState.depthStencilFormat;

		CComPtr<ID3D12PipelineState> pipelineState;
		DX::ThrowIfFailed(renderer->d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&pipelineState)));
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
			_countof(D3D12_BLEND_DESC::RenderTarget),
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
			}
		);
	}

	void ImDrawPipelineRenderTargetFormats(nlohmann::json& PipelineState)
	{
		std::vector<std::string> selectables = nostd::GetKeysFromMap(stringToDxgiFormat);
		ImDrawDynamicArray(
			"RT",
			PipelineState.at("renderTargetsFormats"),
			_countof(D3D12_GRAPHICS_PIPELINE_STATE_DESC::RTVFormats),
			[](nlohmann::json& renderTargetsFormats, unsigned int index)
			{
				auto pos = renderTargetsFormats.begin() + index + 1;
				renderTargetsFormats.insert(pos, dxgiFormatsToString.at(DXGI_FORMAT_R8G8B8A8_UNORM));
			},
			[selectables](nlohmann::json& format, unsigned int index)
			{
				std::string value = format.dump();
				value.erase(remove(value.begin(), value.end(), '\"'), value.end());
				DrawComboSelection(value, selectables, [&format](std::string newValue)
					{
						format = newValue;
					}, ""
				);
				ImGui::SameLine();
				std::string label = (std::string("RT#") + std::to_string(index + 1));
				ImGui::Text(label.c_str());
			}
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
		{ "BlendState", [](nlohmann::json& PipelineState) { PipelineState["BlendState"] = nlohmann::json::object(); } },
		{ "RasterizerState", [](nlohmann::json& PipelineState) { PipelineState["RasterizerState"] = nlohmann::json::object(); } },
		{ "PrimitiveTopologyType", [](nlohmann::json& PipelineState) { PipelineState["PrimitiveTopologyType"] = primitiveTopologyTypeToString.at(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE); } },
		{ "renderTargetsFormats", [](nlohmann::json& PipelineState) { PipelineState["renderTargetsFormats"] = nlohmann::json::array({ dxgiFormatsToString.at(DXGI_FORMAT_R8G8B8A8_UNORM) }); } },
		{ "depthStencilFormat", [](nlohmann::json& PipelineState) { PipelineState["depthStencilFormat"] = dxgiFormatsToString.at(DXGI_FORMAT_D32_FLOAT); } },
	};

	std::map<std::string, std::function<void(nlohmann::json&)>> drawPipelineState = {
		{ "BlendState", [](nlohmann::json& PipelineState)
			{
				ImDrawObject("BlendState", PipelineState.at("BlendState"), addToBlendState, drawBlendState);
			}
		},
		{ "RasterizerState", [](nlohmann::json& PipelineState)
			{
				ImDrawObject("RasterizerState", PipelineState.at("RasterizerState"), addToRasterizerState, drawRasterizerState);
			}
		},
		{ "depthStencilFormat", [](nlohmann::json& PipelineState) { drawFromCombo(PipelineState, "depthStencilFormat", stringToDxgiFormat); } },
		{ "PrimitiveTopologyType", [](nlohmann::json& PipelineState) { drawFromCombo(PipelineState, "PrimitiveTopologyType", stringToPrimitiveTopologyType); } },
		{ "renderTargetsFormats", ImDrawPipelineRenderTargetFormats },
	};

	void ImDrawPipelineState(nlohmann::json& PipelineState)
	{
		ImDrawObject("PipelineState", PipelineState, addToPipelineState, drawPipelineState);
	}
#endif
};
