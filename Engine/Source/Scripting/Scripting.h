#pragma once

namespace Scripting
{
	void InitScripting(const char* path);
	void ShutdownScripting();
	void RunScript(std::string script, RenderableUUID renderable);
}