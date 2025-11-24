#include "pch.h"
#include "Scripting.h"
#include <JObject.h>

#if defined(_EDITOR)
namespace Editor
{
	extern bool IsPlaying();
	extern bool IsPaused();
}
#endif

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
#if defined(_EDITOR)
		if (!Editor::IsPlaying() || Editor::IsPaused())
		{
			return;
		}
#endif
		if (script.empty()) return;

		v8::Isolate::Scope isolate_scope(isolate);

		v8::Locker locker(isolate);

		// Create a stack-allocated handle scope.
		v8::HandleScope handle_scope(isolate);

		// Create a new V8 context
		v8::Local<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New(isolate);
		v8::Local<v8::Context> context = v8::Context::New(isolate, nullptr, global_template);
		v8::Context::Scope context_scope(context);

		V8ObjectsBindings bindings;
		renderable->CreateV8Bindings(bindings);

		for (auto& it : bindings)
		{
			auto& [name, ptr] = it.first;

			// Create a V8 object to represent the C++ instance
			v8::Local<v8::Object> jsObject = v8::Object::New(isolate);

			// Create a v8::External to hold a pointer to the C++ instance
			v8::Local<v8::External> external_ptr = v8::External::New(isolate, ptr);

			for (auto& fit : it.second)
			{
				auto& [fname, fptr] = fit;

				// Create a FunctionTemplate for the method and associate the external_ptr as data
				v8::Local<v8::FunctionTemplate> func_template = v8::FunctionTemplate::New(isolate, *fptr, external_ptr);

				// Get the function from the template
				v8::Local<v8::Function> jsFunction = func_template->GetFunction(context).ToLocalChecked();

				// Set the function as a property on the V8 object
				jsObject->Set(context, v8::String::NewFromUtf8(isolate, fname.c_str()).ToLocalChecked(), jsFunction).FromJust();
			}

			// Set the V8 object as a global variable
			context->Global()->Set(context, v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked(), jsObject).FromJust();
		}

		// Enter the context for compiling and running the hello world script.
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
			//OutputDebugStringA(std::string(std::string(*utf8) + "\n").c_str());
		}
	}
}