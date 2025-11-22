#pragma once

namespace Scripting
{
	typedef std::tuple<std::string, void*> V8ObjectPointer;
	typedef std::map<std::string, v8::FunctionCallback> V8MethodsBindings;
	typedef std::map<V8ObjectPointer, V8MethodsBindings> V8ObjectsBindings;

	void InitScripting(const char* path);
	void ShutdownScripting();
	void RunScript(std::string script, RenderableUUID renderable);
}