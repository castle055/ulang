//
// Created by castle on 9/22/23.
//

#ifndef ULANG_RUNTIME_H
#define ULANG_RUNTIME_H

#include <string>
#include <ecl/ecl.h>

namespace ulang::runtime {
  void init();
  void deinit();
  
  void eval(const std::string& str);
}

#endif //ULANG_RUNTIME_H
