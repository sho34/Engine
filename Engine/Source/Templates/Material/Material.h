#pragma once

//#include "../Shader/Shader.h"
#include <VertexFormats.h>
#include "Variables.h"
#include "SamplerDesc.h"
#include "RasterizerDesc.h"
#include "BlendDesc.h"
#include <wrl/client.h>
#include <atlbase.h>
#include <TemplateDecl.h>
#include <Json.h>
#include <DXTypes.h>
#include <NoStd.h>
#include <Textures/Texture.h>
#include <ShaderMaterials.h>
#include <JTemplate.h>
#include <JExposeTypes.h>

namespace Templates
{
	struct ShaderInstance;
};

using namespace Templates;

enum TextureShaderUsage;

typedef std::map<TextureShaderUsage, std::shared_ptr<TextureInstance>> TextureUsageInstanceMap;
typedef std::pair<TextureShaderUsage, std::shared_ptr<TextureInstance>> TextureUsageInstancePair;

inline MaterialInitialValuePair ToMaterialInitialValuePair(nlohmann::json j)
{
	return MaterialInitialValuePair(j.at("variable"), JsonToMaterialInitialValue(j));
}

inline nlohmann::json FromMaterialInitialValuePair(MaterialInitialValuePair p)
{
	nlohmann::json j = nlohmann::json({});

	j["variable"] = p.first;
	j["variableType"] = MaterialVariablesTypesToString.at(p.second.variableType);
	valueMappingToJson(p.second.variableType, j, p.second);
	return j;
}

inline TextureShaderUsagePair ToTextureShaderUsagePair(nlohmann::json::iterator it)
{
	return { StringToTextureShaderUsage.at(it.key()), it.value() };
}

inline nlohmann::json FromTextureShaderUsagePair(TextureShaderUsagePair m)
{
	nlohmann::json j = nlohmann::json::object({});
	j[TextureShaderUsageToString.at(m.first)] = m.second;
	return j;
}

namespace Templates {

#include <JExposeAttOrder.h>
#include <MaterialAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttDrawersDecl.h>
#include <MaterialAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttRequired.h>
#include <MaterialAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttJsonDecl.h>
#include <MaterialAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttCreatorDrawersDecl.h>
#include <MaterialAtt.h>
#include <JExposeEnd.h>

	void MaterialJsonStep();

	struct MaterialJson : public JTemplate
	{
		TEMPLATE_DECL(Material);

#include <JExposeAttFlags.h>
#include <MaterialAtt.h>
#include <JExposeEnd.h>

#include <JExposeDecl.h>
#include <MaterialAtt.h>
#include <JExposeEnd.h>
	};

	TEMPDECL_FULL(Material);

	namespace Material
	{
		inline static const std::string templateName = "materials.json";
#if defined(_EDITOR)
		inline static const std::string fallbackShader_vs = "BaseLighting_vs";
		inline static const std::string fallbackShader_ps = "BaseLighting_ps";
#endif
	};

	//DESTROY
	//void FreeGPUTexturesUploadIntermediateResources();

	struct MaterialInstance
	{
		MaterialInstance(const std::string uuid) { assert(!!!"do not use"); }
		explicit MaterialInstance(
			const std::string instance_uuid,
			const std::string uuid,
			VertexClass vClass,
			bool isShadowed,
			TextureShaderUsageMap overrideTextures = {},
			std::string objectUUID = "",
			JObjectChangeCallback cb = nullptr,
			JObjectChangePostCallback postCb = nullptr
		);
		~MaterialInstance() { Destroy(); }

		std::string materialUUID;
		std::string instanceUUID;

		std::string vertexShaderUUID;
		std::string pixelShaderUUID;

		VertexClass vertexClass;
		bool shadowed;
		MaterialVariablesMapping variablesMapping;
		std::vector<size_t> variablesBufferSize;
		std::vector<std::vector<byte>> variablesBuffer;

		std::vector<std::string> defines;
		std::shared_ptr<ShaderInstance> vertexShader;
		std::shared_ptr<ShaderInstance> pixelShader;
		std::vector<MaterialSamplerDesc> samplers;
		TextureUsageInstanceMap textures;
		std::map<unsigned int, ::CD3DX12_GPU_DESCRIPTOR_HANDLE> uav;

		void CreateMaterialShaderDefines();
		void CreateShaderInstances();
		void Destroy();
		bool ShaderInstanceHasRegister(std::function<int(std::shared_ptr<ShaderInstance>&)> getRegister);
		void LoadVariablesMapping();
		void SetUAVRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& slot);
		void SetSRVRootDescriptorTable(CComPtr<ID3D12GraphicsCommandList2>& commandList, unsigned int& slot);
	};

	void DestroyMaterialInstance(std::shared_ptr<MaterialInstance>& materialInstance);

	TEMPDECL_REFTRACKER(Material);
};
