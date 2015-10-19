/**
 * Copyright (c) 2015 - The CM Authors <legal@clickmatcher.com>
 *   All Rights Reserved.
 *
 * This file is CONFIDENTIAL -- Distribution or duplication of this material or
 * the information contained herein is strictly forbidden unless prior written
 * permission is obtained.
 */
#include "stx/inspect.h"
#include "zbase/JavaScriptContext.h"
#include "js/Conversions.h"

using namespace stx;

namespace zbase {

static JSClass global_class = { "global", JSCLASS_GLOBAL_FLAGS };

JavaScriptContext::JavaScriptContext() {
  runtime_ = JS_NewRuntime(8 * 1024 * 1024);
  if (!runtime_) {
    RAISE(kRuntimeError, "error while initializing JavaScript runtime");
  }

  ctx_ = JS_NewContext(runtime_, 8192);
  if (!ctx_) {
    RAISE(kRuntimeError, "error while initializing JavaScript context");
  }

  JSAutoRequest js_req(ctx_);

  global_ = JS_NewGlobalObject(
      ctx_,
      &global_class,
      nullptr,
      JS::FireOnNewGlobalHook);

  if (!global_) {
    RAISE(kRuntimeError, "error while initializing JavaScript context");
  }

  {
    JSAutoCompartment ac(ctx_, global_);
    JS_InitStandardClasses(ctx_, global_);
  }
}

JavaScriptContext::~JavaScriptContext() {
  JS_DestroyContext(ctx_);
  JS_DestroyRuntime(runtime_);
}

void JavaScriptContext::loadProgram(const String& program) {
  JSAutoRequest js_req(ctx_);
  JSAutoCompartment ac(ctx_, global_);

  JS::RootedValue rval(ctx_);

  JS::CompileOptions opts(ctx_);
  opts.setFileAndLine("<mapreduce>", 1);

  if (!JS::Evaluate(
        ctx_,
        global_,
        opts,
        program.c_str(),
        program.size(),
        &rval)) {
    RAISE(kRuntimeError, "JavaScript execution failed");
  }
}

void JavaScriptContext::callMapFunction(
    const String& method_name,
    const String& json_string,
    Vector<Pair<String, String>>* tuples) {
  auto json_wstring = StringUtil::convertUTF8To16(json_string);

  JSAutoRequest js_req(ctx_);
  JSAutoCompartment js_comp(ctx_, global_);

  JS::RootedValue json(ctx_);
  if (!JS_ParseJSON(ctx_, json_wstring.c_str(), json_wstring.size(), &json)) {
    RAISE(kRuntimeError, "invalid JSON");
  }

  JS::AutoValueArray<1> argv(ctx_);
  argv[0].set(json);

  JS::RootedValue rval(ctx_);
  if (!JS_CallFunctionName(ctx_, global_, method_name.c_str(), argv, &rval)) {
    RAISE(kRuntimeError, "map function failed");
  }

  if (!rval.isObject()) {
    RAISE(kRuntimeError, "map function must return a list/array of tuples");
  }

  JS::RootedObject list(ctx_, &rval.toObject());
  JS::AutoIdArray list_enum(ctx_, JS_Enumerate(ctx_, list));
  for (size_t i = 0; i < list_enum.length(); ++i) {
    JS::RootedValue elem(ctx_);
    JS::RootedValue elem_key(ctx_);
    JS::RootedValue elem_value(ctx_);
    JS::Rooted<jsid> elem_id(ctx_, list_enum[i]);
    if (!JS_GetPropertyById(ctx_, list, elem_id, &elem)) {
      RAISE(kIllegalStateError);
    }

    if (!elem.isObject()) {
      RAISE(kRuntimeError, "map function must return a list/array of tuples");
    }

    JS::RootedObject elem_obj(ctx_, &elem.toObject());

    if (!JS_GetProperty(ctx_, elem_obj, "0", &elem_key)) {
      RAISE(kRuntimeError, "map function must return a list/array of tuples");
    }

    if (!JS_GetProperty(ctx_, elem_obj, "1", &elem_value)) {
      RAISE(kRuntimeError, "map function must return a list/array of tuples");
    }

    auto tkey_jstr = JS::ToString(ctx_, elem_key);
    if (!tkey_jstr) {
      RAISE(kRuntimeError, "first tuple element must be a string");
    }

    auto tkey_cstr = JS_EncodeString(ctx_, tkey_jstr);
    String tkey(tkey_cstr);
    JS_free(ctx_, tkey_cstr);

    auto tval_jstr = JS_ValueToSource(ctx_, elem_value);
    if (!tval_jstr) {
      RAISE(kRuntimeError, "first tuple element must be a string");
    }

    auto tval_cstr = JS_EncodeString(ctx_, tval_jstr);
    String tval(tval_cstr);
    JS_free(ctx_, tval_cstr);

    tuples->emplace_back(tkey, tval);
  }

}

} // namespace zbase
