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
%type <op> BinOp

%left ADD SUB
%left MUL DIV
%left UMINUS UPLUS


%%
CompUnit : Exp {root = $1; }
Exp : Term { $$ = $1;}
    | SUB Exp %prec UMINUS { $$ = new TreeUnaryExpr(OP_Neg, $2); }
    | ADD Exp %prec UPLUS  { $$ = $2; }
    | Exp BinOp Exp { $$ = new TreeBinaryExpr($2, $1, $3); }
    ;

BinOp : ADD { $$ = OpType::OP_Add; }
      | SUB { $$ = OpType::OP_Sub; }
      | MUL { $$ = OpType::OP_Mul; }
      | DIV { $$ = OpType::OP_Div; }
      ;

Term : INT { $$ = new TreeIntegerLiteral($1); }
     ;
%%

void yyerror(const char *s) {
    printf("error: %s\n", s);
}
