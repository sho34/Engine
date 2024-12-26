#include "pch.h"
#include "Shader.h"
#include <map>
#include <string>
#include "../../Shaders/Compiler/ShaderCompiler.h"

namespace Templates::Shader {

	std::map<std::wstring, ShaderPtr> shaderTemplates;
	std::map<ShaderType, std::wstring> ShaderT::shaderEntryPoint = {
		 {	ShaderType::VERTEX_SHADER, L"main_vs"	},
		 {	ShaderType::PIXEL_SHADER, L"main_ps"	},
	};
	std::map<ShaderType, std::wstring> ShaderT::shaderTarget = {
		 {	ShaderType::VERTEX_SHADER, L"vs_6_1"	},
		 {	ShaderType::PIXEL_SHADER, L"ps_6_1"	},
	};
	TemplatesNotification<ShaderPtr*> shaderChangeNotifications;

	static void OnShaderCompilationStart(void* shaderPtr){
		auto shader = static_cast<ShaderPtr*>(shaderPtr);
		if (shaderChangeNotifications.contains(shader)) {
			NotifyOnLoadStart<ShaderPtr*>(shaderChangeNotifications[shader]);
		}
		(*shader)->loading = true;
		(*shader)->pixelShader = nullptr;
		(*shader)->vertexShader = nullptr;
	}

	static void OnShaderCompilationComplete(void* shaderPtr, void* shaderCompilerOutputPtr) {
		auto shader = static_cast<ShaderPtr*>(shaderPtr);
		auto output = static_cast<ShaderCompilerOutputPtr*>(shaderCompilerOutputPtr);
		switch ((*output)->shaderType) {
		case ShaderType::VERTEX_SHADER:
			(*shader)->vertexShader = *output;
			break;
		case ShaderType::PIXEL_SHADER:
			(*shader)->pixelShader = *output;
			break;
		}
		if ((*shader)->vertexShader && (*shader)->pixelShader != nullptr) {
			(*shader)->loading = false;
			if (shaderChangeNotifications.contains(shader)) {
				NotifyOnLoadComplete(shader,shaderChangeNotifications[shader]);
			}
		}
	}
	
	static void OnShaderCompilerOutputDestroy(void* shaderPtr, void* shaderCompilerOutputPtr) {

	}

	ShaderPtr* CreateNewShader(std::wstring shaderTemplateName, ShaderDefaultValues defaultValues) {
		auto shader = std::make_shared<Shader>();
		shader->defaultValues = defaultValues;
		shader->loading = true;
		shaderTemplates.insert(std::pair<std::wstring, ShaderPtr>(shaderTemplateName, shader));
		return &shaderTemplates.find(shaderTemplateName)->second;
	}

	static std::mutex createShaderMutex;
	Concurrency::task<void> CreateShaderTemplate(std::wstring shaderTemplateName, ShaderDefaultValues defaultValues, LoadShaderFn loadFn)
	{
		auto currentShader = GetShaderTemplate(shaderTemplateName);
		if (currentShader != nullptr) {
			if (loadFn) loadFn(currentShader);
			return concurrency::create_task([] {});
		}

		ShaderPtr* shader = CreateNewShader(shaderTemplateName, defaultValues);

		std::wstring shaderName = defaultValues.shaderFileName == L"" ? shaderTemplateName : defaultValues.shaderFileName;

		return concurrency::create_task([shaderName, shader] {
			std::lock_guard<std::mutex> lock(createShaderMutex);
			auto shaderTasks = {
				ShaderCompiler::Bind(shaderName.c_str(),ShaderType::VERTEX_SHADER, (void*)(shader), NotificationCallbacks({
					.onLoadStart = OnShaderCompilationStart,
					.onLoadComplete = OnShaderCompilationComplete,
					.onDestroy = OnShaderCompilerOutputDestroy,
				})),
				ShaderCompiler::Bind(shaderName.c_str(),ShaderType::PIXEL_SHADER, (void*)(shader), NotificationCallbacks({
					.onLoadStart = OnShaderCompilationStart,
					.onLoadComplete = OnShaderCompilationComplete,
					.onDestroy = OnShaderCompilerOutputDestroy,
				})),
			};

			auto waitForShaders = when_all(std::begin(shaderTasks), std::end(shaderTasks));

			waitForShaders.wait();
		}).then([loadFn, shader] {
			if (loadFn) loadFn(shader);
		});
	}

	void ReleaseShaderTemplates()
	{
		shaderTemplates.clear();
	}

	Concurrency::task<void> BindToShaderTemplate(const std::wstring& shaderTemplateName, void* target, NotificationCallbacks callbacks)
	{
		auto shader = GetShaderTemplate(shaderTemplateName);
		assert(shader != nullptr);

		return concurrency::create_task([shader, target, callbacks] {
			ChangesNotifications notifications = {
			{
				NotificationTarget({.target = target }), callbacks }
			};
			shaderChangeNotifications[shader] = notifications;
		});
	}

	ShaderPtr* GetShaderTemplate(std::wstring shaderTemplateName)
	{
		auto it = shaderTemplates.find(shaderTemplateName);
		return (it != shaderTemplates.end()) ? &it->second : nullptr;
	}

	std::map<std::wstring, std::shared_ptr<Shader>> GetNamedShaders() {
		return shaderTemplates;
	}

	std::vector<std::wstring> GetShadersNames() {
		std::vector<std::wstring> names;
		std::transform(shaderTemplates.begin(), shaderTemplates.end(), std::back_inserter(names), [](std::pair<std::wstring, std::shared_ptr<Shader>> pair) { return pair.first; });
		return names;
	}

#if defined(_DEBUG)
	nlohmann::json json()
	{
		nlohmann::json j = nlohmann::json({});

		for (auto& [name, shader] : shaderTemplates) {
			if (shader->defaultValues.systemCreated) continue;
			std::string jname = WStringToString(name);
			j[jname] = nlohmann::json({});
			j[jname]["shaderFileName"] = WStringToString(shader->defaultValues.shaderFileName);
			j[jname]["mappedValues"] = TransformMappingToJson(shader->defaultValues.mappedValues);
		}

		return j;
	}
#endif

	Concurrency::task<void> json(std::wstring name, nlohmann::json shaderj)
	{
		auto currentShader = GetShaderTemplate(name);
		if (currentShader != nullptr) {
			return concurrency::create_task([] {});
		}

		ShaderDefaultValues defaultValues = {
			.shaderFileName = (shaderj.contains("shaderFileName") and shaderj["shaderFileName"] != "") ? StringToWString(shaderj["shaderFileName"]) : name,
			.mappedValues = TransformJsonToMapping(shaderj["mappedValues"]),
		};
		replaceFromJson(defaultValues.systemCreated, shaderj, "systemCreated");

		ShaderPtr* shader = CreateNewShader(name, defaultValues);

		std::wstring shaderName = (shaderj.contains("shaderFileName") and shaderj["shaderFileName"] != "") ? StringToWString(shaderj["shaderFileName"]) : name;

		return concurrency::create_task([shaderName, shader] {
			std::lock_guard<std::mutex> lock(createShaderMutex);
			auto shaderTasks = {
				ShaderCompiler::Bind(shaderName.c_str(),ShaderType::VERTEX_SHADER, (void*)(shader), NotificationCallbacks({
					.onLoadStart = OnShaderCompilationStart,
					.onLoadComplete = OnShaderCompilationComplete,
					.onDestroy = OnShaderCompilerOutputDestroy,
				})),
				ShaderCompiler::Bind(shaderName.c_str(),ShaderType::PIXEL_SHADER, (void*)(shader), NotificationCallbacks({
					.onLoadStart = OnShaderCompilationStart,
					.onLoadComplete = OnShaderCompilationComplete,
					.onDestroy = OnShaderCompilerOutputDestroy,
				})),
			};

			auto waitForShaders = when_all(std::begin(shaderTasks), std::end(shaderTasks));

			waitForShaders.wait();
		});
	}

}