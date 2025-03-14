#pragma once

#include "CustomIncludeHandler.h"
#include "CompilerQueue.h"
#include "../../Templates/Shader/Shader.h"

using namespace Templates;

namespace ShaderCompiler {

	void BuildShaderCompiler();
	std::shared_ptr<ShaderInstance> Compile(Source params, ShaderIncludesDependencies& dependencies);
	void DestroyShaderCompiler();

	//compiler and utils
	static ComPtr<IDxcCompiler3> pCompiler;
	static ComPtr<IDxcUtils> pUtils;
};