//
// Created by castle on 7/24/23.
//

#include "uparser.h"

#include "build/lexer.hpp"
#include "build/parser.hpp"

#include <stack>

void uparser::parse(const std::string& str) {
  parser_ctx_t ctx{str};
  
  yy_scan_string(str.c_str(), ctx.lexer);
  ctx.parser->parse();
  
  std::string text = str;
  std::string t;
  for (const auto &sub: ctx.current_grammar.top().substitutions) {
    auto &[p, s] = sub;
    t = "";
    if (p.begin.column > 1)
      t.append(text.substr(0, p.begin.column));
    t.append(s);
    t.append(text.substr(p.end.column-1));
    text = t;
  }
  std::string res = ctx.current_grammar.top().parse(text);
  
  std::cout << res << std::endl;
}

