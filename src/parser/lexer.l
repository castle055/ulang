%option reentrant
%option noyywrap
%option debug
%{  // Some C declarations
    #include <iostream>
    #include "parser.hpp"
    #define YY_DECL yy::parser::symbol_type yylex(yyscan_t yyscanner, yy::location& loc, parser_ctx_t& ctx)

    #define YY_USER_ACTION loc.columns(yyleng);
    #define yyterminate() return parser::make_END(loc);
    using token = yy::parser::token;
    using token_type = yy::parser::token_type;
    using symbol_type = yy::parser::symbol_type;
    using namespace yy;
//[^ \n\t\r]+ { return parser::make_RAW_TOKEN(yytext, loc); }
%}

%%
%{
    loc.step();
%}

=     { return symbol_type(token::EQUAL, loc); };
\{     { return symbol_type(token::LEFT_BRC, loc); };
\}     { return symbol_type(token::RIGHT_BRC, loc); };

"tokens" { return symbol_type(token::TOKENS_KW, loc); }
"rules" { return symbol_type(token::RULES_KW, loc); }
\/[^ \n\t\r]+\/ { return parser::make_TOKEN_REGEX(yytext, loc); };
#"[grammar: "[-a-zA-Z]+"]" { return parser::make_GRAMMAR_SPEC_TAG(yytext, loc); };
"["[-a-zA-Z]+"]" { return parser::make_GRAMMAR_TAG(yytext, loc); };
[-a-zA-Z]+ { return parser::make_IDENTIFIER(yytext, loc); };
"//"[^\n]*\n            ;

[^ \n\t\r]+ { return parser::make_RAW_TOKEN(yytext, loc); };
[ \n\t\r] { /* do nothing */};
.       { std::cerr << loc << ": [ERROR] unrecognized symbol: " << yytext << std::endl; exit(1);}
%%