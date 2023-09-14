//
// Created by castle on 7/24/23.
//

#include "parser_ctx.h"
#include "build/lexer.hpp"
#include "build/parser.hpp"


parser_ctx_t::parser_ctx_t() {
  yylex_init(&lexer);
  loc = new yy::location();
  parser = new yy::parser(lexer, *loc, *this);
}

parser_ctx_t::~parser_ctx_t() {
  yylex_destroy(lexer);
  delete loc;
  delete parser;
}