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
    #include <iostream>

    #define LOC_OF(INDEX) yystack_[INDEX -1].location
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
%token LEFT_BRK
%token RIGHT_BRK
%token LEFT_PRN
%token RIGHT_PRN
%token LEFT_ANG
%token RIGHT_ANG
%token PIPE
%token SEMICOLON

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

%type <ulang_grammar::non_terminal_node> grammar_rule_node
%type <ulang_grammar::non_terminal_node> grammar_rule_option_list
%type <std::pair<std::string, ulang_grammar::grammar_rule>> grammar_spec_rule_list_item
%type <std::unordered_map<std::string, ulang_grammar::grammar_rule>> grammar_spec_rule_list
%type <std::unordered_map<std::string, ulang_grammar::grammar_rule>> grammar_spec_rules

%type <std::pair<std::string, ulang_grammar::token_rule>> grammar_spec_token_list_item
%type <std::unordered_map<std::string, ulang_grammar::token_rule>> grammar_spec_token_list
%type <std::unordered_map<std::string, ulang_grammar::token_rule>> grammar_spec_tokens

%type <ulang_grammar> grammar_spec_block
%type <ulang_grammar> grammar_spec

%type <std::string> grammar_rule_node_pattern
%type <std::string> grammar_rule_node_pattern_token_list

%start root; // Set start node
%% // Parser rules
root: token_list END { };

token_list: any_token{ /*ctx.current_grammar.top().add_token($1); ctx.token_list.push_back({.str = $1});*/ }
	| grammar_spec{ ctx.grammars[$1.name] = $1; }
	| grammar_block
	| token_list any_token{ /*ctx.current_grammar.top().add_token($2); ctx.token_list.push_back({.str = $2});*/ }
	| token_list grammar_spec{ ctx.grammars[$2.name] = $2; }
	| token_list grammar_block;

grammar_block: grammar_tag token_list RIGHT_BRK { ctx.pop_grammar_ctx(LOC_OF(2), yy::location(LOC_OF(3).begin, LOC_OF(1).end)); };
grammar_tag: GRAMMAR_TAG { ctx.push_grammar_ctx(parser_ctx_t::get_grammar_tag_id($1)); };

grammar_spec: GRAMMAR_SPEC_TAG LEFT_BRC grammar_spec_block RIGHT_BRC { ctx.current_grammar.top().substitutions.emplace_front(yy::location(LOC_OF(4).begin, LOC_OF(1).end), "");$3.name = parser_ctx_t::get_grammar_id($1); $$ = $3; };
grammar_spec_block: grammar_spec_tokens grammar_spec_rules{$$={.token_rules=$1, .grammar_rules=$2}; };

grammar_spec_tokens: TOKENS_KW LEFT_BRC grammar_spec_token_list RIGHT_BRC{$$ = $3;};
grammar_spec_token_list: %empty{$$ = {};}
	| grammar_spec_token_list grammar_spec_token_list_item{$1[$2.first] = $2.second; $$=$1;};
grammar_spec_token_list_item: IDENTIFIER EQUAL TOKEN_REGEX {$$=std::make_pair($1, ulang_grammar::token_rule{.regex = std::regex($3.substr(1, $3.size()-2), std::regex_constants::ECMAScript)});};

grammar_spec_rules: RULES_KW LEFT_BRC grammar_spec_rule_list RIGHT_BRC {$$ = $3;};
grammar_spec_rule_list: %empty{$$ = {};}
	| grammar_spec_rule_list grammar_spec_rule_list_item {$1[$2.first] = $2.second; $$=$1;};
grammar_spec_rule_list_item: IDENTIFIER EQUAL grammar_rule_option_list SEMICOLON {$$=std::make_pair($1, ulang_grammar::grammar_rule{.root_item = $3});};

grammar_rule_option_list: grammar_rule_node{$$={.type = ulang_grammar::non_terminal_node::SUB_OPTIONS, .sub_nodes = {$1}};}
	| grammar_rule_option_list PIPE grammar_rule_node{$1.sub_nodes.push_back($3); $$=$1;}
	| grammar_rule_option_list grammar_rule_node_pattern{/*printf("=====>(%lu) %s\n\r", $1.sub_nodes.size(), $2.c_str());*/$1.sub_nodes.back().pattern = $2; $$=$1;}
	;

grammar_rule_node: IDENTIFIER{$$={.type = ulang_grammar::non_terminal_node::SUB_NODES, .sub_nodes = {{.type = parser_ctx_t::is_terminal($1)? ulang_grammar::non_terminal_node::TERMINAL : ulang_grammar::non_terminal_node::NON_TERMINAL, .str = $1}}};}
	| LEFT_BRC grammar_rule_option_list RIGHT_BRC{$$=$2;}
	| LEFT_PRN grammar_rule_option_list RIGHT_PRN{$$=$2;}
	| grammar_rule_node IDENTIFIER{$1.sub_nodes.push_back({.type = parser_ctx_t::is_terminal($2)? ulang_grammar::non_terminal_node::TERMINAL : ulang_grammar::non_terminal_node::NON_TERMINAL, .str = $2}); $$ = $1;}
	| grammar_rule_node LEFT_BRC grammar_rule_option_list RIGHT_BRC{$3.type = ulang_grammar::non_terminal_node::ZERO_OR_MORE; $1.sub_nodes.push_back($3); $$=$1;}
	| grammar_rule_node LEFT_PRN grammar_rule_option_list RIGHT_PRN{$1.sub_nodes.push_back($3); $$=$1;}
	;

grammar_rule_node_pattern: LEFT_ANG grammar_rule_node_pattern_token_list RIGHT_ANG{$$=ctx.get_str(LOC_OF(2));};
grammar_rule_node_pattern_token_list: any_token{$$={$1};}
	| grammar_rule_node_pattern_token_list any_token{/*$1.append(" ").append($2);*/ $$=$1;};

any_token: RAW_TOKEN { $$ = $1; }
 	| EQUAL { $$ = "="; }
 	| LEFT_BRC { $$ = "{"; }
 	| RIGHT_BRC { $$ = "}"; }
 	| LEFT_BRK { $$ = "["; }
 	| RIGHT_BRK { $$ = "]"; }
 	| LEFT_PRN { $$ = "("; }
 	| RIGHT_PRN { $$ = ")"; }
 	| LEFT_ANG { $$ = "<"; }
 	| RIGHT_ANG { $$ = ">"; }
 	| PIPE { $$ = "|"; }
 	| SEMICOLON { $$ = ";"; }
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