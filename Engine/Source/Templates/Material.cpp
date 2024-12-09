#include "pch.h"
#include "Material.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/DeviceUtils/Resources/Resources.h"
#include "../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include "../Scene/Lights/Lights.h"
#include "../Scene/Camera/Camera.h"
#include <ppltasks.h>
#include "../Common/d3dx12.h"

namespace Templates::Material {

	std::map<std::wstring, MaterialPtr> materialTemplates;
	TemplatesNotification<MaterialPtr*> materialChangeNotifications;

	static void OnShaderCompilationStart(void* materialPtr) {
		auto material = static_cast<MaterialPtr*>(materialPtr);
		if (materialChangeNotifications.contains(material)) {
			NotifyOnLoadStart<MaterialPtr*>(materialChangeNotifications[material]);
		}
		(*material)->loading = true;
	}

	static void OnShaderCompilationComplete(void* materialPtr, void* shaderPtr) {
		auto& material = *static_cast<MaterialPtr*>(materialPtr);
		BuildMaterialProperties(material);
		material->loading = false;
		if(materialChangeNotifications.contains(&material)) {
			NotifyOnLoadComplete(&material, materialChangeNotifications[&material]);
		}
	}

	static void OnShaderDestroy(void* materialPtr, void* shaderPtr) {
	}

	INT FindCBufferIndexInMaterial(const MaterialPtr& material, std::wstring bufferName)
	{
		//get the buffer on which lights exists if exists
		auto& vsDef = material->shader->vertexShader->cbufferVariablesDef;
		auto& psDef = material->shader->pixelShader->cbufferVariablesDef;
	
		auto bufferDef = vsDef->find(bufferName);
		if (bufferDef != vsDef->end()) {
			return static_cast<INT>(bufferDef->second.bufferIndex);
		}
		
		bufferDef = psDef->find(bufferName);
		if (bufferDef != psDef->end()) {
			return static_cast<INT>(bufferDef->second.bufferIndex);
		}
		return -1;
	}

	inline void WriteMappedInitialValuesToDestination(auto& def, void* dst, size_t size) {
		switch (def.variableType) {
		case MAT_VAR_BOOLEAN:
		{
			auto src = std::any_cast<BOOLEAN>(def.any_value);
			memcpy(dst, &src, size);
		}
			break;
		case MAT_VAR_INTEGER:
		{
			auto src = std::any_cast<INT>(def.any_value);
			memcpy(dst, &src, size);
		}
			break;
		case MAT_VAR_UNSIGNED_INTEGER:
		{
			auto src = std::any_cast<UINT>(def.any_value);
			memcpy(dst, &src, size);
		}
			break;
		case MAT_VAR_RGB:
			break;
		case MAT_VAR_RGBA:
			break;
		case MAT_VAR_FLOAT:
		{
			auto src = std::any_cast<FLOAT>(def.any_value);
			memcpy(dst, &src, size);
		}
		break;
		case MAT_VAR_FLOAT2:
			break;
		case MAT_VAR_FLOAT3:
		{
			auto src = std::any_cast<XMVECTORF32>(def.any_value);
			memcpy(dst, &src.f[0], size);
			int i = 0;
		}
		break;
		case MAT_VAR_FLOAT4:
			break;
		case MAT_VAR_MATRIX4X4:
			break;
		}
	}

	std::shared_ptr<Material>* CreateNewMaterial(std::wstring materialName, MaterialDefinition materialDefinition) {
		auto material = std::make_shared<Material>();
		material->loading = true;
		material->materialDefinition = materialDefinition;
		material->materialName = materialName;
		materialTemplates.insert(std::pair<std::wstring, MaterialPtr>(materialName, material));
		return &materialTemplates.find(materialName)->second;
	}

	void ReleaseMaterialTemplates()
	{
		for (auto& [name, material] : materialTemplates) {

			//destroy the textures
			for (auto& texture : material->textures) {
				texture.texture = nullptr;
				texture.textureUpload = nullptr; //move this to other place
			}
			material->textures.clear();

			//destroy the variables buffers(variables referencing to constants buffer values)
			for (auto& buf : material->variablesBuffer) {
				delete* buf;
			}
			material->variablesBuffer.clear();

		}

		materialTemplates.clear();
	}

	void BuildMaterialProperties(std::shared_ptr<Material>& material) {

		//get the shader and initialize the memory space for the constant buffer definition space
		material->shader = *Shader::GetShaderTemplate(material->materialDefinition.shaderTemplate);

		material->variablesBufferSize.clear();
		for (auto buff : material->variablesBuffer) { delete* buff; }
		material->variablesBuffer.clear();

		INT numCBuffers = static_cast<INT>(max(material->shader->vertexShader->cbufferSize.size(), material->shader->pixelShader->cbufferSize.size())); //do i really need this?
		for (INT CBufferIndex = 0; CBufferIndex < numCBuffers; CBufferIndex++) {
			//get the CBuffer size
			size_t vsCBSize = material->shader->vertexShader->cbufferSize.size() > CBufferIndex ? material->shader->vertexShader->cbufferSize[CBufferIndex] : 0;
			size_t psCBSize = material->shader->pixelShader->cbufferSize.size() > CBufferIndex ? material->shader->pixelShader->cbufferSize[CBufferIndex] : 0;

			UINT CBufferSize = static_cast<UINT>(max(vsCBSize, psCBSize));
			//lights cbuffer is unbounded so let's give it some special space for it's special needs
			//if (material->lightsCBufferRegister == CBufferIndex) CBufferSize*= Light::MaxLights;
			material->variablesBufferSize.push_back(CBufferSize);

			//allocate memory for the cbuffer on the CPU
			std::shared_ptr<UINT8*> buffer = std::make_shared<UINT8*>(new UINT8[CBufferSize]);
			ZeroMemory(buffer.get()[0], CBufferSize);
			material->variablesBuffer.push_back(buffer);
		}

		//initialize the variables mapping
		material->variablesMapping.clear();
		for (auto [varName, initValue] : material->materialDefinition.mappedValues) {

			//find the variable in the vertex shader
			auto cbufferDef = material->shader->vertexShader->cbufferVariablesDef->find(varName);
			if (cbufferDef != material->shader->vertexShader->cbufferVariablesDef->end()) {
				material->variablesMapping[varName] = MaterialVariableMapping({
					.variableType = initValue.variableType,
					.mapping = cbufferDef->second
					});
				continue;
			}

			//find the variable in the pixel shader
			cbufferDef = material->shader->pixelShader->cbufferVariablesDef->find(varName);
			if (cbufferDef != material->shader->pixelShader->cbufferVariablesDef->end()) {
				material->variablesMapping[varName] = MaterialVariableMapping({
					.variableType = initValue.variableType,
					.mapping = cbufferDef->second
					});
				continue;
			}
		}

		std::set<std::wstring> unmapped;
		//write the default values from the shader to the mapped memory
		for (auto& [varName, mapping] : material->shader->defaultValues.mappedValues) {

			auto def = material->shader->vertexShader->cbufferVariablesDef->find(varName);
			if (def == material->shader->vertexShader->cbufferVariablesDef->end()) continue;

			unmapped.insert(varName);

			UINT8* dst = static_cast<UINT8*>(material->variablesBuffer[def->second.bufferIndex].get()[0]);
			dst += def->second.offset;
			size_t size = def->second.size;
			WriteMappedInitialValuesToDestination(mapping, dst, size);

		}

		//write the values to mapped memory
		for (auto [varName, mapping] : material->variablesMapping) {

			auto def = material->materialDefinition.mappedValues.find(varName);
			if (def == material->materialDefinition.mappedValues.end()) continue;

			UINT8* dst = static_cast<UINT8*>(material->variablesBuffer[mapping.mapping.bufferIndex].get()[0]);
			dst += mapping.mapping.offset;
			size_t size = mapping.mapping.size;
			WriteMappedInitialValuesToDestination(def->second, dst, size);
			unmapped.erase(varName);
		}

		//map variables defined in the shader that the material has not yet define, so it can be updated in the rendereable cbuffer
		for (auto varName : unmapped) {
			auto shdef = material->shader->defaultValues.mappedValues.find(varName);
			auto cbdef = material->shader->vertexShader->cbufferVariablesDef->find(varName);
			if (shdef == material->shader->defaultValues.mappedValues.end()) continue;
			if (cbdef == material->shader->vertexShader->cbufferVariablesDef->end()) continue;

			material->variablesMapping.insert(std::pair<std::wstring, MaterialVariableMapping>(
				varName, MaterialVariableMapping({
					.variableType = shdef->second.variableType,
					.mapping = cbdef->second
				})
			));
		}

		//build the textures info
		material->textures.clear();
		for (auto& textDef : material->materialDefinition.textures) {

			material->textures.push_back(MaterialTextures());
			auto& last = material->textures.back();
			last.texturePath = textDef.texturePath;
			last.textureFormat = textDef.textureFormat;
			last.numFrames = textDef.numFrames;

		}

		//copy the sampler information
		material->samplers = material->materialDefinition.samplers;
	}

	Concurrency::task<void> CreateMaterialTemplate(std::wstring materialName, MaterialDefinition materialDefinition, LoadMaterialFn loadFn)
	{
		auto currentMaterial = GetMaterialTemplate(materialName);
		if (currentMaterial != nullptr) {
			if(loadFn) loadFn(currentMaterial);
			return concurrency::create_task([] {});
		}

		auto material = CreateNewMaterial(materialName, materialDefinition);

		return concurrency::create_task([material] {
			using namespace Scene;

			BuildMaterialProperties(*material);

			//bind the material to the shader
			return Shader::BindToShaderTemplate((*material)->materialDefinition.shaderTemplate, material, 
				{ .onLoadStart = OnShaderCompilationStart, .onLoadComplete = OnShaderCompilationComplete, .onDestroy = OnShaderDestroy, }
			);
		}).then([material, loadFn] {
			(*material)->loading = false;
			if(loadFn) loadFn(*material);
		});
	}

	Concurrency::task<void> BindToMaterialTemplate(const std::wstring& materialName, void* target, NotificationCallbacks callbacks)
	{
		auto material = GetMaterialTemplatePtr(materialName);
		assert(material != nullptr);

		if (materialChangeNotifications.contains(material)) return concurrency::create_task([] {});

		return concurrency::create_task([material, target, callbacks] {
			ChangesNotifications notifications = {
			{
				NotificationTarget({.target = target }), callbacks }
			};
			materialChangeNotifications[material] = notifications;
		});
	}

	std::shared_ptr<Material> GetMaterialTemplate(std::wstring materialName)
	{
		auto it = materialTemplates.find(materialName);
		return (it != materialTemplates.end()) ? it->second : nullptr;
	}

	std::shared_ptr<Material>* GetMaterialTemplatePtr(std::wstring materialName) {
		auto it = materialTemplates.find(materialName);
		return (it != materialTemplates.end()) ? &it->second : nullptr;
	}

	static std::mutex buildTexturesMutex;
	void BuildMaterialTextures(const std::shared_ptr<Renderer>& renderer, std::shared_ptr<Material>& material)
	{
		std::lock_guard<std::mutex> lock(buildTexturesMutex);
		using namespace DeviceUtils::Resources;
		for (auto& texture : material->textures) {
			if (texture.numFrames == 0U) {
				CreateTextureResource(
					renderer->d3dDevice,
					renderer->commandList,
					(LPWSTR)texture.texturePath.c_str(),
					texture.texture,
					texture.textureUpload,
					texture.textureViewDesc,
					texture.textureFormat
				);
			}
			else
			{
				CreateTextureArrayResource(
					renderer->d3dDevice,
					renderer->commandList,
					(LPWSTR)texture.texturePath.c_str(),
					texture.texture,
					texture.textureUpload,
					texture.textureViewDesc,
					texture.numFrames,
					texture.textureFormat
				);
			}
			using namespace DeviceUtils;
			texture.textureCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(ConstantsBuffer::GetCpuDescriptorHandleCurrent());
			texture.textureGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(ConstantsBuffer::GetGpuDescriptorHandleCurrent());
			renderer->d3dDevice->CreateShaderResourceView(texture.texture, &texture.textureViewDesc, texture.textureCpuHandle);
			ConstantsBuffer::GetCpuDescriptorHandleCurrent().Offset(ConstantsBuffer::GetCSUDescriptorSize());
			ConstantsBuffer::GetGpuDescriptorHandleCurrent().Offset(ConstantsBuffer::GetCSUDescriptorSize());
		}
	}

}
