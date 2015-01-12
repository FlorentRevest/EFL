
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eo_Js.hh>
#include <Eina.hh>
#include <Eo.hh>

#include <check.h>

#include <iostream>

#include <cassert>
#include <tuple>

namespace evas {
EAPI void register_box(v8::Handle<v8::Object> global, v8::Isolate* isolate);
}

namespace {

static const char script[] =
  "function assert(condition, message) {\n"
  "  if (!condition) {\n"
  "      print(\"Assertion failed \", message);\n"
  "      throw message || \"Assertion failed\";\n"
  "  }\n"
  "}\n"
  "print(\"teste\");\n"
  "x = new Box();\n"
  ;

const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

// Executes a string within the current v8 context.
bool ExecuteString(v8::Isolate* isolate,
                   v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name)
{
  v8::HandleScope handle_scope
#if 0
    (isolate)
#endif
    ;
  v8::TryCatch try_catch;
  v8::ScriptOrigin origin(name);
  v8::Handle<v8::Script> script = v8::Script::Compile(source, &origin);
  if (script.IsEmpty()) {
    std::abort();
    // Print errors that happened during compilation.
    // if (report_exceptions)
    //   ReportException(isolate, &try_catch);
    return false;
  }
  else
  {
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      std::cout << "Failed with exception thrown" << std::endl;
      assert(try_catch.HasCaught());
      std::abort();
      // Print errors that happened during execution.
      // if (report_exceptions)
      //   ReportException(isolate, &try_catch);
      return false;
    } else {
      assert(!try_catch.HasCaught());
      // if (print_result && !result->IsUndefined()) {
      //   // If all went well and the result wasn't undefined then print
      //   // the returned value.
      //   v8::String::Utf8Value str(result);
      //   const char* cstr = ToCString(str);
      //   printf("%s\n", cstr);
      // }
      return true;
    }
  }
}

#if 0
void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args[i]);
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
}
#else
v8::Handle<v8::Value> Print(v8::Arguments const& args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope;
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args[i]);
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
}
#endif

START_TEST(eolian_js_test_evas_box)
{
  int argc = 1;
  const char* argv[] = {"test"};
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;
  
  //v8::V8::InitializeICU();
  v8::V8::Initialize();
  v8::V8::SetFlagsFromCommandLine(&argc, const_cast<char**>(argv), true);

  v8::Isolate* isolate = v8::Isolate::New();

  v8::Isolate::Scope isolate_scope(isolate);
  
  assert(isolate != 0);

  v8::Handle<v8::Context> context;
  v8::HandleScope handle_scope
#if 0
    (isolate)
#endif
    ;

  {
    // Create a template for the global object.
    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New
      (
#if 0
       isolate
#endif
       );
    // Bind the global 'print' function to the C++ Print callback.
    global->Set(v8::String::New/*FromUtf8*/(/*isolate,*/ "print"),
                v8::FunctionTemplate::New(/*isolate,*/ Print));

    context = v8::Context::New(/*isolate*/NULL, global);

  }
  if (context.IsEmpty()) {
    fprintf(stderr, "Error creating context\n");
  }
  context->Enter();
  {
    // Enter the execution environment before evaluating any code.
    v8::Context::Scope context_scope(context);
    v8::Local<v8::String> name(v8::String::New/*FromUtf8*/(/*context->GetIsolate(),*/ "(shell)"));

    evas::register_box(context->Global(), isolate);

    v8::HandleScope handle_scope/*(context->GetIsolate())*/;
    ExecuteString(/*context->GetIsolate()*/isolate,
                  v8::String::New/*FromUtf8*/(/*context->GetIsolate(), */script),
                  name);
  }
  context->Exit();
}
END_TEST

}

void eolian_js_test_evas_box(TCase* tc)
{
   tcase_add_test(tc, eolian_js_test_evas_box);
}