/* This is simple example of embedding V8.
  It must be compiled using the C++20 standard, or you'll get "no member named '...' in namespace 'std'" errors.
*/

#define V8_COMPRESS_POINTERS 1 // Required because the V8 slim builds have v8_enable_pointer_compression turned on
#include <v8/libplatform/libplatform.h>
#include <v8/v8.h>
#include <stdio.h>

// Link against the monolith build
#pragma comment(lib, "v8_monolith")

// Additional libraries needed on Windows
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dbghelp.lib")

// This would also be needed if using a less-slim V8 build
//#pragma comment(lib, "Advapi32.lib")

int main(int argc, char* argv[]) {
  // These would be needed if v8_use_external_startup_data and v8_enable_i18n_support were turned on.
  // They are still safe no-ops with those options disabled, however.
  //v8::V8::InitializeExternalStartupDataFromFile("snapshot_blob.bin");
  //v8::V8::InitializeICU(nullptr);

  // Initialize V8
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  // Create a new Isolate and make it the current one.
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator_shared.reset(
      v8::ArrayBuffer::Allocator::NewDefaultAllocator());
  v8::Isolate* isolate = v8::Isolate::New(create_params);
  {
    v8::Isolate::Scope isolate_scope(isolate);
    // Create a stack-allocated handle scope.
    v8::HandleScope handle_scope(isolate);
    // Create a new context.
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    // Enter the context for compiling and running the hello world script.
    v8::Context::Scope context_scope(context);
    {
      // Create a string containing the JavaScript source code.
      v8::Local<v8::String> source =
          v8::String::NewFromUtf8Literal(isolate, "'Hello' + ', World!'");
      // Compile the source code.
      v8::Local<v8::Script> script =
          v8::Script::Compile(context, source).ToLocalChecked();
      // Run the script to get the result.
      v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
      // Convert the result to an UTF8 string and print it.
      v8::String::Utf8Value utf8(isolate, result);
      printf("%s\n", *utf8);
    }
    {
      const char csource[] = R"(function main() {
        console.log(JSON.stringify(Object.keys(globalThis), null, 4));
        return 3+4;
      }
      main();)";
      // Create a string containing the JavaScript source code.
      v8::Local<v8::String> source =
          v8::String::NewFromUtf8Literal(isolate, csource);
      // Compile the source code.
      v8::Local<v8::Script> script =
          v8::Script::Compile(context, source).ToLocalChecked();
      // Run the script to get the result.
      v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
      // Convert the result to a uint32 and print it.
      uint32_t number = result->Uint32Value(context).ToChecked();
      printf("3 + 4 = %u\n", number);
    }
  }

  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  printf("Success!");
  return 0;
}
