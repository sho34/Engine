#include "pch.h"
#include "Scripting.h"
#include <JObject.h>

extern void BindGameObjectsToV8Context(v8::Local<v8::Context>& context);

namespace Scripting
{
	static std::unique_ptr<v8::Platform> platform;
	static v8::Isolate* isolate = nullptr;
	static v8::Isolate::CreateParams create_params;

	void InitScripting(const char* path)
	{
		// Initialize V8.
		v8::V8::InitializeICUDefaultLocation(path);
		v8::V8::InitializeExternalStartupData(path);
		platform = v8::platform::NewDefaultPlatform();
		v8::V8::InitializePlatform(platform.get());
		v8::V8::Initialize();

		// Create a new Isolate and make it the current one.
		create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
		isolate = v8::Isolate::New(create_params);
	}

	void ShutdownScripting()
	{
		isolate->Dispose();
		v8::V8::Dispose();
		v8::V8::DisposePlatform();
		delete create_params.array_buffer_allocator;
	}

	void RunScript(std::string script, RenderableUUID renderable)
	{
		v8::Isolate::Scope isolate_scope(isolate);
		// Create a stack-allocated handle scope.
		v8::HandleScope handle_scope(isolate);
		// Create a new context.
		v8::Local<v8::Context> context = v8::Context::New(isolate);

		renderable->BindToV8Context(context);

		BindGameObjectsToV8Context(context);

		// Enter the context for compiling and running the hello world script.
		v8::Context::Scope context_scope(context);
		{
			// Create a string containing the JavaScript source code.
			v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, script.data()).ToLocalChecked();

			// Compile the source code.
			v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();

			// Run the script to get the result.
			v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

			// Convert the result to an UTF8 string and print it.
			v8::String::Utf8Value utf8(isolate, result);
			//printf("%s\n", *utf8);
			OutputDebugStringA(std::string(std::string(*utf8) + "\n").c_str());
		}
	}
}