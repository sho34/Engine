#pragma once

#include <JObject.h>
#include <nlohmann/json.hpp>

namespace Templates
{
	struct JTemplate : JObject
	{
		JTemplate(nlohmann::json json) :JObject(json) {}
	};
};