%{
#include <stdio.h>
#include <ast/ast.h>
void yyerror(const char *s);
extern int yylex(void);
extern NodePtr root;
%}

/// types
%union {
    int ival;
    ExprPtr expr;
    OpType op;
}

%token <ival> INT
%token ADD SUB MUL DIV

%start CompUnit

%type <expr> Exp Term

%left ADD SUB
%left MUL DIV
%left UMINUS UPLUS


%%
CompUnit : Exp {root = $1; }
Exp : Term { $$ = $1;}
    | SUB Exp %prec UMINUS { $$ = new TreeUnaryExpr(OP_Neg, $2); }
    | ADD Exp %prec UPLUS  { $$ = $2; }
    | Exp ADD Exp { $$ = new TreeBinaryExpr(OpType::OP_Add, $1, $3); }
    | Exp SUB Exp { $$ = new TreeBinaryExpr(OpType::OP_Sub, $1, $3); }
    | Exp MUL Exp { $$ = new TreeBinaryExpr(OpType::OP_Mul, $1, $3); }
    | Exp DIV Exp { $$ = new TreeBinaryExpr(OpType::OP_Div, $1, $3); }
    ;

Term : INT { $$ = new TreeIntegerLiteral($1); }
     ;
%%

void yyerror(const char *s) {
    printf("error: %s\n", s);
}
