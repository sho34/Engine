#pragma once
#include <set>
#include <map>
#include <vector>
#include <memory>
#include <d3d12shader.h>
#include <wrl.h>
#include <wrl/client.h>
#include <dxcapi.h>
#include <string>
#include <nlohmann/json.hpp>
#include <ShaderMaterials.h>
#include <JTemplate.h>
#include <TemplateDecl.h>
#include <JExposeTypes.h>
#include "ShaderInstance.h"

namespace Templates
{
#include <JExposeAttOrder.h>
#include <ShaderAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttDrawersDecl.h>
#include <ShaderAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttRequired.h>
#include <ShaderAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttJsonDecl.h>
#include <ShaderAtt.h>
#include <JExposeEnd.h>

	void ShaderJsonStep();
	void MonitorShaderChanges(std::string folder);

	namespace Shader
	{
		inline static const std::string templateName = "shaders.json";
	};

	struct ShaderJson : public JTemplate
	{
		TEMPLATE_DECL(Shader);

#include <JExposeAttFlags.h>
#include <ShaderAtt.h>
#include <JExposeEnd.h>

#include <JExposeDecl.h>
#include <ShaderAtt.h>
#include <JExposeEnd.h>
	};

	TEMPDECL_FULL(Shader);
	TEMPDECL_REFTRACKER(Shader);
}


