//
// Created by castle on 2/11/24.
//

#ifndef ULANG_COMPILER_H
#define ULANG_COMPILER_H

#include "parser.h"

#include "type_checker.h"

namespace ulang {
  class compiler_t {
  public:
    void compile() {
      file_stream_t fs{""};
      parser_t parser{fs};
      
      auto ast = parser();
      
      type_checker_t{ast};
    }
  };
}

#endif //ULANG_COMPILER_H
