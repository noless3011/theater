// native/addon.cpp
#include <napi.h>
#include "../include/theater.h"

// Wrapper function for the 'add' function from your DLL
Napi::Number AddWrapper(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  // Check argument count
  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Expected 2 arguments").ThrowAsJavaScriptException();
    return Napi::Number::New(env, 0);
  }

  // Check argument types
  if (!info[0].IsNumber() || !info[1].IsNumber()) {
    Napi::TypeError::New(env, "Arguments must be numbers").ThrowAsJavaScriptException();
    return Napi::Number::New(env, 0);
  }

  // Get the arguments
  int a = info[0].As<Napi::Number>().Int32Value();
  int b = info[1].As<Napi::Number>().Int32Value();

  // Call the DLL function
  int result = add(a, b);

  // Return the result
  return Napi::Number::New(env, result);
}

// Wrapper function for the 'print_message' function from your DLL
Napi::Value PrintMessageWrapper(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  // Call the DLL function
  print_message();

  return env.Undefined();
}

// Initialize the addon
Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(
    Napi::String::New(env, "add"),
    Napi::Function::New(env, AddWrapper)
  );

  exports.Set(
    Napi::String::New(env, "printMessage"),
    Napi::Function::New(env, PrintMessageWrapper)
  );

  return exports;
}

NODE_API_MODULE(native_addon, Init)
