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

    #define U_TOKEN(TYPE) symbol_type(token::TYPE, loc)
%}

%%
%{
    loc.step();
%}

=      { return U_TOKEN( EQUAL       ); };
\{     { return U_TOKEN( LEFT_BRC    ); };
\}     { return U_TOKEN( RIGHT_BRC   ); };
\[     { return U_TOKEN( LEFT_BRK    ); };
\]     { return U_TOKEN( RIGHT_BRK   ); };
\(     { return U_TOKEN( LEFT_PRN    ); };
\)     { return U_TOKEN( RIGHT_PRN   ); };
\<     { return U_TOKEN( LEFT_ANG    ); };
\>     { return U_TOKEN( RIGHT_ANG   ); };
\|     { return U_TOKEN( PIPE        ); };
;      { return U_TOKEN( SEMICOLON   ); };

"tokens"                        { return U_TOKEN( TOKENS_KW  ); }
"rules"                         { return U_TOKEN( RULES_KW   ); }
\/[^\n\t\r]+\/                  { return parser::make_TOKEN_REGEX(yytext, loc); };
#"[grammar: "[-a-zA-Z]+"]"      { return parser::make_GRAMMAR_SPEC_TAG(yytext, loc); };
"["[-a-zA-Z]+"|>"               { return parser::make_GRAMMAR_TAG(yytext, loc); };
[a-zA-Z_]+[a-zA-Z\-_]*          { return parser::make_IDENTIFIER(yytext, loc); };

[^ \n\t\r\<\>;\[\]\(\)\{\}\|]+  { return parser::make_RAW_TOKEN(yytext, loc); };
[ \n\t\r]                       { /* do nothing */};
.                               { std::cerr << loc << ": [ERROR] unrecognized symbol: " << yytext << std::endl; exit(1);}
%%