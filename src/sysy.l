%option noinput
%option nounput
%option noyywrap
%option yylineno

%{
  #include <ast/ast.h>
  #include "sysy.tab.hh"
%}

DIGIT         [0-9]
NONZERO       [1-9]
BLANK         [ \t]
NEWLINE       [\n\r]
LETTER        [A-Za-z]
NONDIGIT      {LETTER}|"_"


If            "if"
Else          "else"
For           "for"
While         "while"
Return        "return"
Break         "break"
Continue      "continue"
LParen        "("
RParen        ")"
LBrace        "{"
RBrace        "}"
LBracket      "["
RBracket      "]"
Semicolon     ";"
Comma         ","
SQuote        "'"
DQuote        "\""
Assign        "="
Eq            "=="
Neq           "!="
Lt            "<"
Gt            ">"
Lte           "<="
Gte           ">="
Plus          "+"
Minus         "-"
Mul           "*"
Div           "/"
Mod           "%"
And           "&&"
Or            "||"
Not           "!"
Dot           "."
Backslash     "\\"
TyInt         "int"
TyVoid        "void"
Int           "0"|{NONZERO}{DIGIT}*
Ident         {NONDIGIT}({DIGIT}|{NONDIGIT})*
Comment1      "/*"[^*]*"*"+([^*/][^*]*"*"+)*"/"
Comment2      "//".*      


%%

{If}              { yylval.str = strdup(yytext); printf("If"); return If; }
{Else}            { yylval.str = strdup(yytext); printf("Else"); return Else; }
{For}             { yylval.str = strdup(yytext); printf("For"); return For; }
{While}           { yylval.str = strdup(yytext); printf("While"); return While; }
{Return}          { yylval.str = strdup(yytext); printf("Return"); return Return; }
{Break}           { yylval.str = strdup(yytext); printf("Break"); return Break; }
{Continue}        { yylval.str = strdup(yytext); printf("Continue"); return Continue; }
{LParen}          { yylval.str = strdup(yytext); printf("LParen"); return LParen; }
{RParen}          { yylval.str = strdup(yytext); printf("RParen"); return RParen; }
{LBrace}          { yylval.str = strdup(yytext); printf("LBrace"); return LBrace; }
{RBrace}          { yylval.str = strdup(yytext); printf("RBrace"); return RBrace; }
{LBracket}        { yylval.str = strdup(yytext); printf("LBracket"); return LBracket; }
{RBracket}        { yylval.str = strdup(yytext); printf("RBracket"); return RBracket; }
{Semicolon}       { yylval.str = strdup(yytext); printf(";"); return Semicolon; }
{Comma}           { yylval.str = strdup(yytext); printf(","); return Comma; }
{SQuote}          { yylval.str = strdup(yytext); printf("\'"); return SQuote; }
{DQuote}          { yylval.str = strdup(yytext); printf("\""); return DQuote; }
{Assign}          { yylval.str = strdup(yytext); printf("Assign"); return Assign; }
{Eq}              { yylval.str = strdup(yytext); printf("Eq"); return Eq; }
{Neq}             { yylval.str = strdup(yytext); printf("Neq"); return Neq; }
{Lt}              { yylval.str = strdup(yytext); printf("Lt"); return Lt; }
{Gt}              { yylval.str = strdup(yytext); printf("Gt"); return Gt; }
{Lte}             { yylval.str = strdup(yytext); printf("Lte"); return Lte; }
{Gte}             { yylval.str = strdup(yytext); printf("Gte"); return Gte; }
{Plus}            { yylval.str = strdup(yytext); printf("Plus"); return Plus; }
{Minus}           { yylval.str = strdup(yytext); printf("Minus"); return Minus; }
{Mul}             { yylval.str = strdup(yytext); printf("Mul"); return Mul; }
{Div}             { yylval.str = strdup(yytext); printf("Div"); return Div; }
{Mod}             { yylval.str = strdup(yytext); printf("Mod"); return Mod; }
{And}             { yylval.str = strdup(yytext); printf("And"); return And; }
{Or}              { yylval.str = strdup(yytext); printf("Or"); return Or; }
{Not}             { yylval.str = strdup(yytext); printf("Not"); return Not; }
{Dot}             { yylval.str = strdup(yytext); printf("Dot"); return Dot; }
{Backslash}       { yylval.str = strdup(yytext); printf("Backslash"); return Backslash; }
{TyInt}           { yylval.str = strdup(yytext); printf("TyInt"); return TyInt; }
{TyVoid}          { yylval.str = strdup(yytext); printf("TyVoid"); return TyVoid; }
{Int}             { yylval.ival = atoi(yytext); printf("Int(%s)", yytext); return Int; }
{Ident}           { yylval.str = strdup(yytext); printf("Ident(%s)", yytext); return Ident; }
{NEWLINE}         { printf("%s", yytext); }
{BLANK}           { printf("%s", yytext); }
{Comment1}        { printf("%s", yytext); }
{Comment2}        { printf("%s", yytext); }
.                 { printf("ERROR(%s)\n", yytext); }

%%
