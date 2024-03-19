%option noinput
%option nounput
%option noyywrap

%{
#include <ast/ast.h>
#include "sysy.tab.hh"
%}

digit [0-9]
blank [ \t\n]

%%

{digit}+        { yylval.ival = atoi(yytext); return INT; }
"+"             { return ADD; }
"-"             { return SUB; }
"*"             { return MUL; }
"/"             { return DIV; }
{blank}         { }
.               { printf("ERROR(%s)\n", yytext); }

%%
