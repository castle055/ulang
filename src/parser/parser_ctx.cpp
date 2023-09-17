//
// Created by castle on 7/24/23.
//

#include "parser_ctx.h"

#include <utility>
#include "build/lexer.hpp"
#include "build/parser.hpp"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-member-init" // Yes, it does
parser_ctx_t::parser_ctx_t(std::string  str)
  : str(std::move(str)) {
  yylex_init(&lexer);
  loc = new yy::location();
  parser = new yy::parser(lexer, *loc, *this);
}
#pragma clang diagnostic pop

parser_ctx_t::~parser_ctx_t() {
  yylex_destroy(lexer);
  delete loc;
  delete parser;
}