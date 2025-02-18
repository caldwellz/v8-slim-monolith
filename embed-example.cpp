/* This is simple example of embedding the modern V8 monoliths built by this repository.
 * It must be compiled using the bundled "hermetic" libc++ (except on Windows; V8 doesn't generate libc++abi build
 * targets on Windows, and there's no easy workaround yet) and at least the C++20 standard, or you'll get weird errors.
 *
 * Needed CXXFLAGS on Windows:
 *  -std=c++20 -D_MT -I/path/to/includes/v8 -I/path/to/includes
 * Needed CXXFLAGS elsewhere:
 *  -std=c++20 -nostdinc++ -isystem/path/to/includes/libc++ -I/path/to/includes/v8 -I/path/to/includes
 * Needed LDFLAGS:
 *  -L/path/to/libs
 */
#define V8_COMPRESS_POINTERS 1  // Required whenever v8_enable_pointer_compression is enabled
#include <stdio.h>
#include <v8/libplatform/libplatform.h>
#include <v8/v8.h>

using namespace v8;

#pragma comment(lib, "v8_monolith")
#ifdef _WIN32
// Additional libraries needed on Windows
#  pragma comment(lib, "winmm")
#  pragma comment(lib, "dbghelp")
// This would also be needed if using a less-slim V8 build
// #pragma comment(lib, "Advapi32")
#else
// Everything but Windows uses libc++
#  pragma comment(lib, "c++_chrome")
#  pragma comment(lib, "c++abi_chrome")
#endif

int main(int argc, char* argv[]) {
  // These could be needed if v8_use_external_startup_data and v8_enable_i18n_support were enabled.
  // They are still safe no-ops with those options disabled, however.
  // V8::InitializeExternalStartupDataFromFile("snapshot_blob.bin");
  // V8::InitializeICU(nullptr);

  // Initialize V8
  std::unique_ptr<Platform> platform = platform::NewDefaultPlatform();
  V8::InitializePlatform(platform.get());
  V8::Initialize();

  // Create a new Isolate and make it the current one.
  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator_shared.reset(ArrayBuffer::Allocator::NewDefaultAllocator());
  Isolate* isolate = Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);
    // Create a stack-allocated handle scope.
    HandleScope handle_scope(isolate);
    // Create a new context.
    Local<Context> context = Context::New(isolate);
    // Enter the context for compiling and running the hello world script.
    Context::Scope context_scope(context);
    {
      // Create a string containing the JavaScript source code.
      Local<String> source = String::NewFromUtf8Literal(isolate, "'Hello' + ', World!'");
      // Compile the source code.
      Local<Script> script = Script::Compile(context, source).ToLocalChecked();
      // Run the script to get the result.
      Local<Value> result = script->Run(context).ToLocalChecked();
      // Convert the result to an UTF8 string and print it.
      String::Utf8Value utf8(isolate, result);
      printf("%s\n", *utf8);
    }
    {
      const char csource[] = R"(function main() {
        console.log(JSON.stringify(Object.keys(globalThis), null, 4));
        return 3+4;
      }
      main();)";
      // Create a string containing the JavaScript source code.
      Local<String> source = String::NewFromUtf8Literal(isolate, csource);
      // Compile the source code.
      Local<Script> script = Script::Compile(context, source).ToLocalChecked();
      // Run the script to get the result.
      Local<Value> result = script->Run(context).ToLocalChecked();
      // Convert the result to a uint32 and print it.
      uint32_t number = result->Uint32Value(context).ToChecked();
      printf("3 + 4 = %u\n", number);
    }
  }

  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  V8::Dispose();
  V8::DisposePlatform();
  printf("Success!");
  return 0;
}
