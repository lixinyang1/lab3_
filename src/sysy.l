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

{If}              { printf("If"); return If; }
{Else}            { printf("Else"); return Else; }
{For}             { printf("For"); return For; }
{While}           { printf("While"); return While; }
{Return}          { printf("Return"); return Return; }
{Break}           { printf("Break"); return Break; }
{Continue}        { printf("Continue"); return Continue; }
{LParen}          { printf("LParen"); return LParen; }
{RParen}          { printf("RParen"); return RParen; }
{LBrace}          { printf("LBrace"); return LBrace; }
{RBrace}          { printf("RBrace"); return RBrace; }
{LBracket}        { printf("LBracket"); return LBracket; }
{RBracket}        { printf("RBracket"); return RBracket; }
{Semicolon}       { printf(";"); return Semicolon; }
{Comma}           { printf(","); return Comma; }
{SQuote}          { printf("\'"); return SQuote; }
{DQuote}          { printf("\""); return DQuote; }
{Assign}          { printf("Assign"); return Assign; }
{Eq}              { printf("Eq"); return Eq; }
{Neq}             { printf("Neq"); return Neq; }
{Lt}              { printf("Lt"); return Lt; }
{Gt}              { printf("Gt"); return Gt; }
{Lte}             { printf("Lte"); return Lte; }
{Gte}             { printf("Gte"); return Gte; }
{Plus}            { printf("Plus"); return Plus; }
{Minus}           { printf("Minus"); return Minus; }
{Mul}             { printf("Mul"); return Mul; }
{Div}             { printf("Div"); return Div; }
{Mod}             { printf("Mod"); return Mod; }
{And}             { printf("And"); return And; }
{Or}              { printf("Or"); return Or; }
{Not}             { printf("Not"); return Not; }
{Dot}             { printf("Dot"); return Dot; }
{Backslash}       { printf("Backslash"); return Backslash; }
{TyInt}           { printf("TyInt"); return TyInt; }
{TyVoid}          { printf("TyVoid"); return TyVoid; }
{Int}             { yylval.ival = atoi(yytext); printf("Int(%s)", yytext); return Int; }
{Ident}           { yylval.strPtr = new std::string(strdup(yytext)); printf("Ident(%s)", yytext); return Ident; }
{NEWLINE}         { printf("%s", yytext); }
{BLANK}           { printf("%s", yytext); }
{Comment1}        { printf("%s", yytext); }
{Comment2}        { printf("%s", yytext); }
.                 { printf("ERROR(%s)\n", yytext); }

%%
