#include "pch.h"
#include "RootSignature.h"
#include "../../../Renderer/Renderer.h"
#include "../../../Common/DirectXHelper.h"

extern std::shared_ptr<Renderer> renderer;

template <>
struct std::hash<RootSignatureDesc>
{
	std::size_t operator()(const RootSignatureDesc& r) const
	{
		using std::hash;
		const ShaderConstantsBufferParametersMap& cbufferVSParamsDef = std::get<0>(r);
		const ShaderConstantsBufferParametersMap& cbufferPSParamsDef = std::get<1>(r);
		const ShaderUAVParametersMap& uavParamsDef = std::get<2>(r);
		const ShaderSRVCSParametersMap& srvCSParamsDef = std::get<3>(r);
		const ShaderSRVTexParametersMap& srvTexParamsDef = std::get<4>(r);
		const ShaderSamplerParametersMap& samplersDef = std::get<5>(r);
		const std::vector<MaterialSamplerDesc>& matSampler = std::get<6>(r);

		size_t h = 0ULL;
		nostd::hash_combine(h,
			std::hash<ShaderConstantsBufferParametersMap>()(cbufferVSParamsDef),
			std::hash<ShaderConstantsBufferParametersMap>()(cbufferPSParamsDef),
			std::hash<ShaderUAVParametersMap>()(uavParamsDef),
			std::hash<ShaderSRVCSParametersMap>()(srvCSParamsDef),
			std::hash<ShaderSRVTexParametersMap>()(srvTexParamsDef),
			std::hash<ShaderSamplerParametersMap>()(samplersDef),
			std::hash<std::vector<MaterialSamplerDesc>>()(matSampler)
		);
		return h;
	}
};

namespace DeviceUtils
{
	static nostd::RefTracker<size_t, HashedRootSignature> refTracker;

	using namespace Templates;

	HashedRootSignature CreateRootSignature(RootSignatureDesc& r)
	{
		size_t hash = std::hash<RootSignatureDesc>()(r);
		return refTracker.AddRef(hash, [hash, &r]()
			{
				return std::make_tuple(hash, CreateRootSignature(std::to_string(hash),
					std::get<0>(r),
					std::get<1>(r),
					std::get<2>(r),
					std::get<3>(r),
					std::get<4>(r),
					std::get<5>(r),
					std::get<6>(r)
					)
				);
			}
		);
	}

	CComPtr<ID3D12RootSignature> CreateRootSignature(
		std::string name,
		ShaderConstantsBufferParametersMap& cbufferVSParamsDef,
		ShaderConstantsBufferParametersMap& cbufferPSParamsDef,
		ShaderUAVParametersMap& uavParamsDef,
		ShaderSRVCSParametersMap& srvCSParamsDef,
		ShaderSRVTexParametersMap& srvTexParamsDef,
		ShaderSamplerParametersMap& samplersDef,
		std::vector<MaterialSamplerDesc>& matSamplers
	)
	{
		auto& d3dDevice = renderer->d3dDevice;

		//keep this flags as default??
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

		std::map<int, CD3DX12_DESCRIPTOR_RANGE> ranges = GetRootSignatureRanges(cbufferVSParamsDef, cbufferPSParamsDef, uavParamsDef, srvCSParamsDef, srvTexParamsDef);

		std::vector<CD3DX12_ROOT_PARAMETER> parameters;
		for (auto& [reg, range] : ranges)
		{
			parameters.push_back(CD3DX12_ROOT_PARAMETER());
			auto& last = parameters.back();
			last.InitAsDescriptorTable(1, &range);
		}

		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers = GetRootSignatureSamplerDesc(samplersDef, matSamplers);

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
		CCNAME_D3D12_OBJECT_N(rootSignature, name);

		return rootSignature;
	}

	CComPtr<ID3D12RootSignature> CreateComputeShaderRootSignature(std::string name)
	{
		return CComPtr<ID3D12RootSignature>();
	}

	CComPtr<ID3D12RootSignature> GetRootSignature(size_t rootSignatureHash)
	{
		return std::get<1>(refTracker.FindValue(rootSignatureHash));
	}

	const ShaderConstantsBufferParameter& FindRegisterByName(ShaderConstantsBufferParametersMap& paramsVS, ShaderConstantsBufferParametersMap& paramsPS, std::string name) {
		if (paramsVS.find(name) != paramsVS.end()) return paramsVS.at(name);
		return paramsPS.at(name);
	}

	std::map<INT, CD3DX12_DESCRIPTOR_RANGE> GetRootSignatureRanges(
		ShaderConstantsBufferParametersMap& cbufferVSParamsDef,
		ShaderConstantsBufferParametersMap& cbufferPSParamsDef,
		ShaderUAVParametersMap& uavParamsDef,
		ShaderSRVCSParametersMap& srvCSParamsDef,
		ShaderSRVTexParametersMap& srvTexParamsDef
	) {

		//get the CBuffer Names for VS and PS
		std::vector<std::string> cbuffersVS = nostd::GetKeysFromMap(cbufferVSParamsDef);
		std::vector<std::string> cbuffersPS = nostd::GetKeysFromMap(cbufferPSParamsDef);

		//merge the cbuffer names
		std::set<std::string> cbuffersNames;
		for (std::string& name : cbuffersVS) { cbuffersNames.insert(name); }
		for (std::string& name : cbuffersPS) { cbuffersNames.insert(name); }

		//create the cbuffer descriptor ranges
		std::map<INT, CD3DX12_DESCRIPTOR_RANGE> ranges;
		INT maxRegister = -1;
		for (auto name : cbuffersNames)
		{
			const ShaderConstantsBufferParameter cbv = FindRegisterByName(cbufferVSParamsDef, cbufferPSParamsDef, name);
			ranges[cbv.registerId].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, cbv.numConstantsBuffers, cbv.registerId);
			maxRegister = max(static_cast<int>(cbv.registerId), maxRegister);
		}

		//fill UAV
		for (auto& [name, uav] : uavParamsDef)
		{
			ranges[++maxRegister].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, uav.numUAV, uav.registerId);
		}

		//fill SRV
		for (auto& [name, srv] : srvCSParamsDef)
		{
			ranges[++maxRegister].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srv.numSRV, srv.registerId);
		}
		for (auto& [name, srv] : srvTexParamsDef)
		{
			ranges[++maxRegister].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srv.numSRV, srv.registerId);
		}

		return ranges;
	}

	std::vector<D3D12_STATIC_SAMPLER_DESC> GetRootSignatureSamplerDesc(ShaderSamplerParametersMap& samplersDef, std::vector<MaterialSamplerDesc>& matSamplers)
	{
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;

		UINT defaultShaderRegister = 0U;
		for (auto& [name, sampler] : samplersDef)
		{
			samplers.push_back((matSamplers.size() > 0) ? matSamplers[0] : MaterialSamplerDesc());

			D3D12_STATIC_SAMPLER_DESC& last = samplers.back();
			last.ShaderRegister = (sampler.registerId < matSamplers.size()) ? sampler.registerId : defaultShaderRegister;

			defaultShaderRegister++;
		}

		return samplers;
	}
};