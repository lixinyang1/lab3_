%{
#include <stdio.h>
#include <string>
#include <ast/ast.h>
#include <stdlib.h>
#include <stdarg.h>
void yyerror(const char *s);
extern int yylex(void);
extern TreeExpr root;
%}

/// types
%union {
    int ival;
    char* str;
    TreeExpr expr;
    OpType op;
}

%token <ival> Int
%token <str> Ident
%token TyInt TyVoid If Else For While Return Break Continue 
%token LParen RParen LBrace RBrace LBracket RBracket Semicolon Comma SQuote DQuote
%token Assign Eq Neq Lt Gt Lte Gte Plus Minus Mul Div Mod And Or Not Dot
%token Backslash

%start CompUnit

%type <expr> VarDecl MoreVarDef VarDef Dimension InitVal FuncDef FuncFParams
            FuncFParam DimParams MoreDimParams MoreFuncFParams Block BlockItems BlockItem Stmt
            ReturnExp Exp LVal ValIndex PrimaryExp UnaryExp FuncRParams MoreFuncRParams
            UnaryOp MulExp AddExp RelExp EqExp LAndExp LOrExp

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
                                $1->append($2);
                                root = $$ = $1;
                            }
        | CompUnit FuncDef {
                                $1->append($2);
                                root = $$ = $1;
                            }
        | VarDecl {
                                root = $$ = new TreeRoot(std::vector<ExprPtr>{$1});
        }
        | FuncDef {
                                root = $$ = new TreeRoot(std::vector<ExprPtr>{$1});
        }
        ;

VarDecl : TyInt VarDef MoreVarDef Semicolon {
                                $3.insert($3.begin(), $2);
                                $$ = new TreeVarDecl(INT, $3);
        };
MoreVarDef : Comma VarDef MoreVarDef {
                                $3.insert($3.begin(), $2);
                                $$ = $3;
}
        | {
                                $$ = std::vector<ExprPtr>{};
        }
        ;

VarDef : Ident Assign InitVal {
                                ExprPtr left = new TreeVarExpr($1, std::vector<int>{});
                                $$ = new TreeAssignStmt(left, $3);
}
        | Ident Dimension {
                                ExprPtr left = new TreeVarExpr($1,$2);
                                $$ = new TreeAssignStmt(left, NULL);                       
        }
        ;
        
Dimension : LBracket Int RBracket Dimension {
                                $4.insert($4.begine(), $2); // reverse indexes
                                $$ = $4;
}
        | {
                                $$ = std::vector<int>{};
        }
        ;

InitVal : Exp {
                                $$ = $1;
};

FuncDef : TyInt Ident LParen FuncFParams RParen Block {
                                $$ = new TreeFuncDef($2, $3, INT, $5);
}
        | TyVoid Ident LParen FuncFParams RParen Block {
                                $$ = new TreeFuncDef($2, $3, VOID, $5);
        }
        ;

FuncFParams : FuncFParam MoreFuncFParams {
                                $2.insert($1);
                                $$ = $2;
}
        | {
                                $$ = std::map<std::string, varType> {};
        }
        ;

FuncFParam : TyInt Ident DimParams {
                                $$ = std::make_pair($2, $1);
};

DimParams : LBracket RBracket MoreDimParams {
}
        | {
                
        }
        ;

MoreDimParams : LBracket Int RBracket MoreDimParams {
                                $4.insert($4.begin(), $2);
                                $$ = $4;
}
        | {
                                $$ = std::vector<int>{};
        }
        ;

MoreFuncFParams : Comma FuncFParam MoreFuncFParams {
}
        | {
        }
        ;

Block : LBrace BlockItems RBrace;

BlockItems : BlockItem BlockItems
        | 
        ;

BlockItem : VarDecl
        | Stmt
        ;

Stmt : LVal Assign Exp Semicolon
        | Exp Semicolon
        | Block
        | If LParen Exp RParen Stmt Else Stmt
        | If LParen Exp RParen Stmt %prec LowerThanElse
        | While LParen Exp RParen Stmt
        | Break Semicolon
        | Continue Semicolon
        | Return ReturnExp Semicolon
        ;

ReturnExp : Exp
        |
        ;

Exp : LOrExp;

LVal : Ident ValIndex;

ValIndex : LBracket Exp RBracket ValIndex
        | 
        ;

PrimaryExp : LParen Exp RParen
        | LVal
        | Int
        ;

UnaryExp : PrimaryExp
        | Ident LParen FuncRParams RParen
        | UnaryOp UnaryExp
        ;

FuncRParams : Exp MoreFuncRParams
        |
        ;

MoreFuncRParams : Comma Exp MoreFuncRParams
        |
        ;

UnaryOp : Plus
        | Minus
        | Not
        ;

/// The following Exps depends on priority
MulExp : UnaryExp
        | MulExp Mul UnaryExp
        | MulExp Div UnaryExp
        | MulExp Mod UnaryExp
        ;

AddExp : MulExp
        | AddExp Plus MulExp;
        | AddExp Minus MulExp;
        ;

RelExp : AddExp
        | RelExp Lt AddExp
        | RelExp Gt AddExp
        | RelExp Lte AddExp
        | RelExp Gte AddExp

EqExp : RelExp 
        | EqExp Eq RelExp
        | EqExp Neq RelExp
        ;

LAndExp : EqExp
        | LAndExp And EqExp
        ;

LOrExp : LAndExp
        | LOrExp Or LAndExp
        ;


%%

void yyerror(const char *s) {
    printf("error: %s\n", s);
}
