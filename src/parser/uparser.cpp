//
// Created by castle on 7/24/23.
//

#include "uparser.h"

#include "build/lexer.hpp"
#include "build/parser.hpp"

#include <stack>

void uparser::parse(const std::string& str) {
  parser_ctx_t ctx;
  
  yy_scan_string(str.c_str(), ctx.lexer);
  std::cout << std::endl << "[[PARSING]]" << std::endl;
  
  ctx.parser->parse();
  
  std::cout << "[TOKEN LIST]" << std::endl;
  for (const auto &tokenv: ctx.current_grammar.top().result_tokens.top()) {
    for (const auto &token: tokenv) {
      if (token.contains(' ')) {
        std::cout << "\"" << token << "\" ";
      } else {
        std::cout << token << " ";
      }
    }
  }
  std::cout << std::endl;
}

