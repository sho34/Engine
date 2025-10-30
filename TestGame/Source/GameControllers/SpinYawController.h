#pragma once
#include <Controller.h>

namespace Game
{
	struct SpinYawController : Controller
	{
		virtual void Step(float delta);
	};
}