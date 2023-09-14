%require "3.2"
%defines
%skeleton "lalr1.cc"
%define api.value.type variant
%define api.token.constructor
%define parse.trace
%define parse.error verbose
%locations

%code requires
{
    #include <stdio.h>
    #include <string>
    #include "../parser_ctx.h"
}

%code
{
yy::parser::symbol_type yylex(void* yyscanner, yy::location& loc, parser_ctx_t& ctx);
    #include "lexer.hpp"
}

// Tokens
%token END 0;
%token ROOT

%token EQUAL
%token LEFT_BRC
%token RIGHT_BRC

%token TOKENS_KW
%token RULES_KW

%token <std::string> TOKEN_REGEX
%token <std::string> GRAMMAR_SPEC_TAG
%token <std::string> GRAMMAR_TAG
%token <std::string> IDENTIFIER
%token <std::string> RAW_TOKEN

%lex-param {void* scanner} {yy::location& loc} {parser_ctx_t& ctx}
%parse-param {void* scanner} {yy::location& loc} {parser_ctx_t& ctx}

%type <std::string> any_token

%start root; // Set start node
%% // Parser rules
root: token_list END { printf("SOMETHING\n\r"); };

token_list: any_token{ ctx.current_grammar.top().feed_raw_token($1); ctx.token_list.push_back({.str = $1}); }
	| GRAMMAR_TAG{ ctx.push_grammar_ctx($1); }
	| grammar_spec{ printf("GRAMMAR\n\r"); }
	| token_list any_token{ ctx.current_grammar.top().feed_raw_token($2); ctx.token_list.push_back({.str = $2}); }
	| token_list GRAMMAR_TAG{ ctx.push_grammar_ctx($2); }
	| token_list grammar_spec{ printf("GRAMMAR\n\r"); };

grammar_spec: GRAMMAR_SPEC_TAG LEFT_BRC grammar_spec_block RIGHT_BRC { printf("GRAMMAR SPEC: %s\n\r", $1.c_str()); };
grammar_spec_block: %empty;
	| grammar_spec_tokens grammar_spec_rules;

grammar_spec_tokens: TOKENS_KW LEFT_BRC RIGHT_BRC
	| TOKENS_KW LEFT_BRC grammar_spec_token_list RIGHT_BRC
grammar_spec_token_list: grammar_spec_token_list_item
	| grammar_spec_token_list grammar_spec_token_list_item;
grammar_spec_token_list_item: IDENTIFIER EQUAL TOKEN_REGEX;

grammar_spec_rules: RULES_KW LEFT_BRC RIGHT_BRC
	| RULES_KW LEFT_BRC grammar_spec_rule_list RIGHT_BRC
grammar_spec_rule_list: grammar_spec_rule_list_item
	| grammar_spec_rule_list grammar_spec_rule_list_item;
grammar_spec_rule_list_item: IDENTIFIER EQUAL grammar_rule_option_list;

grammar_rule_option_list: grammar_rule_option
	| grammar_rule_option_list "|" grammar_rule_option;
grammar_rule_option: IDENTIFIER
	| grammar_rule_option IDENTIFIER;

any_token: RAW_TOKEN { $$ = $1; };
 	| EQUAL { $$ = "="; }
 	| LEFT_BRC { $$ = "{"; }
 	| RIGHT_BRC { $$ = "}"; }
 	| TOKENS_KW { $$ = "tokens"; }
 	| RULES_KW { $$ = "rules"; }
 	| IDENTIFIER { $$ = $1; };
%%

// Main function for generated executable
#ifdef UPARSER_GEN_EXEC
int main(int argc, char** argv) {
    // Set input file if given
    //if (argc > 1) {
        //yyin = fopen(argv[1], "r");
        //if (yyin == NULL){
        //    printf("syntax: %s filename\n", argv[0]);
        //}
    //}
}
#endif

// Error handling subroutine
void yy::parser::error(const yy::location& l, const std::string& m) {
    std::cerr << l << ": " << m << std::endl;
}