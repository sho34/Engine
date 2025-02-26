#pragma once

#include "CustomIncludeHandler.h"
#include "CompilerQueue.h"
#include "../../Resources/ShaderByteCode.h"
#include "../../Templates/Shader/Shader.h"

namespace ShaderCompiler {

	inline static const std::string shaderRootFolder = "Shaders/";

	std::shared_ptr<ShaderBinary> GetShaderBinary(Source& params);
	std::shared_ptr<ShaderBinary> Compile(Source& params);
	void DestroyShaderBinary(std::shared_ptr<ShaderBinary>& shaderBinary);
	//Concurrency::task<void> Bind(const std::string& shaderName, ShaderType shaderType, void* target, NotificationCallbacks callbacks);

	//compiler and utils
	static ComPtr<IDxcCompiler3> pCompiler;
	static ComPtr<IDxcUtils> pUtils;
	void BuildShaderCompiler();
	void MonitorShaderChanges(std::string folder);
	void DestroyShaderCompiler();
};