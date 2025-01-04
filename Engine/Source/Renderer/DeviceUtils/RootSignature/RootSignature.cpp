#include "pch.h"
#include "RootSignature.h"
#include "../../../Common/DirectXHelper.h"

namespace DeviceUtils::RootSignature
{
	static std::mutex rootSignatureMutex;
	CComPtr<ID3D12RootSignature> CreateRootSignature(CComPtr<ID3D12Device2> d3dDevice, const MaterialPtr& material)
	{
		std::lock_guard<std::mutex> lock(rootSignatureMutex);

		//keep this flags as default??
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

		std::map<INT,CD3DX12_DESCRIPTOR_RANGE> ranges = GetRootSignatureRanges(material);
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers = GetRootSignatureSamplerDesc(material);
		std::vector<CD3DX12_ROOT_PARAMETER> parameters;
		for (auto& [reg, range] : ranges) {
			parameters.push_back(CD3DX12_ROOT_PARAMETER());
			auto& last = parameters.back();
			last.InitAsDescriptorTable(1, &range);
		}

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(
			static_cast<UINT>(parameters.size()), parameters.data(),
			static_cast<UINT>(samplers.size()), samplers.data(),
			rootSignatureFlags
		);

		//build the root signature
		CComPtr<ID3D12RootSignature> rootSignature;
		CComPtr<ID3DBlob> pSignature;
		CComPtr<ID3DBlob> pError;
		DX::ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError));
		DX::ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
		CCNAME_D3D12_OBJECT(rootSignature);

		return rootSignature;
	}

	auto GetRegistersNames = [](auto& paramsDef) {
		std::vector<std::string> names;
		std::transform(paramsDef->begin(), paramsDef->end(), std::back_inserter(names), [](auto& paramDef) {
			return paramDef.first;
		});
		return names;
	};

	CBufferParameter* FindRegisterByName(auto& paramsVS, auto& paramsPS, std::string name) {
		if (paramsVS->find(name) != paramsVS->end()) {
			return &paramsVS->find(name)->second;
		}
		if (paramsPS->find(name) != paramsPS->end()) {
			return &paramsPS->find(name)->second;
		}
		return nullptr;
	}

	std::map<INT, CD3DX12_DESCRIPTOR_RANGE> GetRootSignatureRanges(const MaterialPtr& material){
		
		//get the CBuffer Names for VS and PS
		auto& cbufferVSParamsDef = material->shader->vertexShader->cbufferParametersDef;
		auto& cbufferPSParamsDef = material->shader->pixelShader->cbufferParametersDef;
		auto cbuffersVS = GetRegistersNames(cbufferVSParamsDef);
		auto cbuffersPS = GetRegistersNames(cbufferPSParamsDef);

		//merge the cbuffer names
		std::set<std::string> cbuffersNames;
		for (auto name : cbuffersVS) { cbuffersNames.insert(name); }
		for (auto name : cbuffersPS) { cbuffersNames.insert(name); }

		//create the cbuffer descriptor ranges
		std::map<INT, CD3DX12_DESCRIPTOR_RANGE> ranges;
		INT maxRegister = -1;
		for (auto name : cbuffersNames) {
			auto cbv = FindRegisterByName(cbufferVSParamsDef, cbufferPSParamsDef, name);
			ranges[cbv->registerId].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, cbv->numCBuffers, cbv->registerId); 
			maxRegister = max(static_cast<INT>(cbv->registerId), maxRegister);
;		}

		//fill SRV
		auto& srvPSParamsDef = material->shader->pixelShader->texturesParametersDef;
		for (auto& [name, srv] : *srvPSParamsDef) {
			ranges[++maxRegister].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srv.numTextures, srv.registerId);
		}

		return ranges;
	}

	std::vector<D3D12_STATIC_SAMPLER_DESC> GetRootSignatureSamplerDesc(const MaterialPtr& material)
	{
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;

		auto& samplersDef = material->shader->pixelShader->samplersParametersDef;

		UINT defaultShaderRegister = 0U;
		for (auto& [name, sampler] : *samplersDef) {
			samplers.push_back(MaterialSamplerDesc());
			auto& last = samplers.back();

			if (sampler.registerId < material->samplers.size()) {
				last.ShaderRegister = sampler.registerId;
			}	else {
				last.ShaderRegister = defaultShaderRegister;
			}
			defaultShaderRegister++;
		}

		return samplers;
	}

};