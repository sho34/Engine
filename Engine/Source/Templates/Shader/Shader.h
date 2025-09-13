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
#include <JTypes.h>
#include "ShaderInstance.h"

namespace Templates
{
#include <Attributes/JOrder.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

	void ShaderJsonStep();
	void MonitorShaderChanges(std::string folder);

	namespace Shader
	{
		inline static const std::string templateName = "shaders.json";
	};

	struct ShaderJson : public JTemplate
	{
		TEMPLATE_DECL(Shader);

#include <Attributes/JFlags.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>
	};

	TEMPDECL_FULL(Shader);
	TEMPDECL_REFTRACKER(Shader);
}


