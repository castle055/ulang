//
// Created by castle on 2/11/24.
//

#ifndef ULANG_TYPE_CHECKER_H
#define ULANG_TYPE_CHECKER_H

#include "ast.h"

namespace ulang {
  
  struct type_checker_t: public visitor_base_t<int> {
    explicit type_checker_t(const std::unique_ptr<node::base_t>& _tree)
    : visitor_base_t(_tree) {}
  private:
    int operator()() override { }
  protected:
  };
}

#endif //ULANG_TYPE_CHECKER_H
