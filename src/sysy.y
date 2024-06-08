%{
#include <stdio.h>
#include <string>
#include <ast/ast.h>
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>
void yyerror(const char *s);
extern int yylex(void);
extern int yylineno;
extern char* yytext;
extern TreeRoot* root;
%}

/// types
%union {
    int ival;
    std::string *strPtr;
    ExprPtr expr;
    OpType op;
    std::vector<ExprPtr> *vec1Ptr;
    std::vector< std::pair< std::string, varType> > *vec2Ptr;
    std::pair<std::string, varType> *pairPtr;
    std::vector<int> *dimension_size_ptr;
    TreeRoot* rt;
}

%token <ival> Int
%token <strPtr> Ident
%token TyInt TyVoid If Else For While Return Break Continue 
%token LParen RParen LBrace RBrace LBracket RBracket Semicolon Comma SQuote DQuote
%token Assign Eq Neq Lt Gt Lte Gte Plus Minus Mul Div Mod And Or Not Dot
%token Backslash

%start CompUnit

%type <expr> VarDecl VarDef InitVal FuncDef 
            BlockItem Stmt Block
            ReturnExp Exp LVal PrimaryExp UnaryExp  
            MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <op> UnaryOp 
%type <vec1Ptr> FuncRParams MoreFuncRParams ValIndex BlockItems Dimension MoreVarDef
%type <vec2Ptr> MoreFuncFParams FuncFParams
%type <pairPtr> FuncFParam
%type <dimension_size_ptr> DimParams MoreDimParams
%type <rt> CompUnit

/// priority
%nonassoc LowerThanElse
%nonassoc Else
%right Assign
%left Or
%left And
%left Eq Neq
%left Lt Gt Lte Gte
%left Plus Minus
%left MUL DIV Mod
%right Not


%%

CompUnit : CompUnit VarDecl {
                                root->append($2);
                            }
        | CompUnit FuncDef {
                                root->append($2);
                            }
        | VarDecl {
                                root = new TreeRoot(std::vector<ExprPtr>{});
                                root->append($1);
        }
        | FuncDef {
                                root = new TreeRoot(std::vector<ExprPtr>{});
                                root->append($1);
        }
        ;

VarDecl : TyInt VarDef MoreVarDef Semicolon {
                                $3->insert($3->begin(), $2);
                                $$ = new TreeVarDecl(INT, *$3);
        };
MoreVarDef : Comma VarDef MoreVarDef {
                                $3->insert($3->begin(), $2);
                                $$ = $3;
}
        | {
                                $$ = new std::vector<ExprPtr>{};
        }
        ;

VarDef : Ident Assign InitVal {
                                ExprPtr left = new TreeVarExpr(*$1, std::vector<ExprPtr>{});
                                $$ = new TreeAssignStmt(left, $3);
}
        | Ident Dimension {
                                ExprPtr left = new TreeVarExpr(*$1,*$2);
                                $$ = new TreeAssignStmt(left, NULL);                       
        }
        ;
        
Dimension : LBracket Int RBracket Dimension {
                                ExprPtr t = new TreeNumber($2);
                                $4->insert($4->begin(), t); // reverse indexes
                                $$ = $4;
}
        | {
                                $$ = new std::vector<ExprPtr>{};
        }
        ;

InitVal : Exp {
                                $$ = $1;
};

FuncDef : TyInt Ident LParen FuncFParams RParen Block {
                                $$ = new TreeFuncDef(*$2, *$4, INT, $6);
}
        | TyVoid Ident LParen FuncFParams RParen Block {
                                $$ = new TreeFuncDef(*$2, *$4, VOID, $6);
        }
        ;

FuncFParams : FuncFParam MoreFuncFParams {
                                $2->insert($2->begin(), *$1);
                                $$ = $2;
}
        | {
                                $$ = new std::vector< std::pair< std::string, varType> > {};
        }
        ;

FuncFParam : TyInt Ident DimParams {
                                varType v = varType(INT, 0, *$3);
                                $$ = new std::pair<std::string, varType>(*$2, v);
};

DimParams : LBracket RBracket MoreDimParams {
                                $3->insert($3->begin(), 0);
                                $$ = $3;
}
        | {
                                $$ = new std::vector<int>();
        }
        ;

MoreDimParams : LBracket Int RBracket MoreDimParams {
                                $4->insert($4->begin(), $2);
                                $$ = $4;
}
        | {
                                $$ = new std::vector<int>();
        }
        ;

MoreFuncFParams : Comma FuncFParam MoreFuncFParams {
                                $3->insert($3->begin(), *$2);
                                $$ = $3;
}
        | {
                                $$ = new std::vector< std::pair< std::string, varType> > {};
        }
        ;

Block : LBrace BlockItems RBrace {
                                $$ = new TreeBlock(*$2);
};

BlockItems : BlockItem BlockItems {
                                $2->insert($2->begin(), $1);
                                $$ = $2;
}
        | {
                                $$ = new std::vector<ExprPtr>{};
        }
        ;

BlockItem : VarDecl {
                                $$ = $1;
}
        | Stmt {
                                $$ = $1;
        }
        ;

Stmt : LVal Assign Exp Semicolon {
                                $$ = new TreeAssignStmt($1, $3);     
}
        | Exp Semicolon {
                                $$ = $1;      
        }
        | Block {
                                $$ = $1;
        }
        | If LParen Exp RParen Stmt Else Stmt {
                                $$ = new TreeIfStmt($3, $5, $7);
        }
        | If LParen Exp RParen Stmt %prec LowerThanElse {
                                $$ = new TreeIfStmt($3, $5, NULL);
        }
        | While LParen Exp RParen Stmt {
                                $$ = new TreeWhileStmt($3, $5);
        }
        | Break Semicolon {
                                $$ = new TreeWhileControlStmt("Break");
        }
        | Continue Semicolon {
                                $$ = new TreeWhileControlStmt("Continue");
        }
        | Return ReturnExp Semicolon {
                                $$ = new TreeReturnStmt($2);
        }
        ;

ReturnExp : Exp {
                                $$ = $1;
}
        | {
                                $$ = NULL;
        }
        ;

Exp : LOrExp{
                                $$ = $1;
};

LVal : Ident ValIndex {
                                $$ = new TreeVarExpr(*$1, *$2);
};

ValIndex : LBracket Exp RBracket ValIndex {
                                $4->insert($4->begin(), $2);
                                $$ = $4;
}
        | {
                                $$ = new std::vector<ExprPtr> {};
        }
        ;

PrimaryExp : LParen Exp RParen {
                                $$ = $2;
}
        | LVal {
                                $$ = $1;
        }
        | Int {
                                $$ = new TreeNumber($1);
        }
        ;

UnaryExp : PrimaryExp {
                                $$ = $1;
}
        | Ident LParen FuncRParams RParen {
                                $$ = new TreeFuncExpr(*$1, *$3);
        }
        | UnaryOp UnaryExp {
                                $$ = new TreeUnaryExpr($1, $2);
        }
        ;

FuncRParams : Exp MoreFuncRParams{
                                $2->insert($2->begin(), $1);
                                $$ = $2;
}
        | {
                                $$ = new std::vector<ExprPtr>{} ;
        }
        ;

MoreFuncRParams : Comma Exp MoreFuncRParams {
                                $3->insert($3->begin(), $2);
                                $$ = $3;
}
        | {
                                $$ = new std::vector<ExprPtr>{} ;
        }
        ;

UnaryOp : Plus {
                                $$ = OP_Pos;
}
        | Minus{
                                $$ = OP_Neg;
        }
        | Not {
                                $$ = OP_Lnot;
        }
        ;

/// The following Exps depends on priority
MulExp : UnaryExp {
                                $$ = $1;
}
        | MulExp Mul UnaryExp{
                                $$ = new TreeBinaryExpr(OP_Mul, $1, $3);
        }
        | MulExp Div UnaryExp{
                                $$ = new TreeBinaryExpr(OP_Div, $1, $3);
        }
        | MulExp Mod UnaryExp{
                                $$ = new TreeBinaryExpr(OP_Mod, $1, $3);
        }
        ;

AddExp : MulExp {
                                $$ = $1;
}
        | AddExp Plus MulExp {
                                $$ = new TreeBinaryExpr(OP_Add, $1, $3);
        }
        | AddExp Minus MulExp{
                                $$ = new TreeBinaryExpr(OP_Sub, $1, $3);
        }
        ;

RelExp : AddExp {
                                $$ = $1;
}
        | RelExp Lt AddExp {
                                $$ = new TreeBinaryExpr(OP_Lt, $1, $3);
        }
        | RelExp Gt AddExp {
                                $$ = new TreeBinaryExpr(OP_Gt, $1, $3);
        }
        | RelExp Lte AddExp {
                                $$ = new TreeBinaryExpr(OP_Le, $1, $3);
        }
        | RelExp Gte AddExp {
                                $$ = new TreeBinaryExpr(OP_Ge, $1, $3);
        }

EqExp : RelExp {
                                $$ = $1;
}
        | EqExp Eq RelExp {
                                $$ = new TreeBinaryExpr(OP_Eq, $1, $3);
        }
        | EqExp Neq RelExp {
                                $$ = new TreeBinaryExpr(OP_Ne, $1, $3);
        }
        ;

LAndExp : EqExp {
                                $$ = $1;
}
        | LAndExp And EqExp {
                                $$ = new TreeBinaryExpr(OP_Land, $1, $3);
        }
        ;

LOrExp : LAndExp {
                                $$ = $1;
}
        | LOrExp Or LAndExp {
                                $$ = new TreeBinaryExpr(OP_Lor, $1, $3);
        }
        ;


%%

void yyerror(const char* s)
{
    printf("\n\033[1;31m%s at line %d\033[0m: %s\n",s,yylineno,yytext);
}
