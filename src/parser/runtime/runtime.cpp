//
// Created by castle on 9/22/23.
//

#include "runtime.h"

namespace rt = ulang::runtime;

static bool initialized = false;

void rt::init() {
  if (initialized) return;
  initialized = true;
  //std::atexit([] {
  //  rt::deinit();
  //});
}
void rt::deinit() {
  if (!initialized) return;
  initialized = false;
  cl_shutdown();
}

void rt::eval(const std::string& src) {
  cl_object form = ecl_read_from_cstring(src.c_str());
  cl_object result = si_safe_eval(2, form, ECL_NIL);
  cl_print(1, result);
}