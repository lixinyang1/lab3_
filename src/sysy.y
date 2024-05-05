%{
#include <stdio.h>
#include <string>
#include <ast/ast.h>
#include <stdlib.h>
#include <stdarg.h>
void yyerror(const char *s);
extern int yylex(void);
extern TreeNode* root;
%}

/// types
%union {
    int ival;
    char* str;
    TreeNode* expr;
    OpType op;
}

%token <ival> Int
%token <str> Ident
%token TyInt TyVoid If Else For While Return Break Continue 
%token LParen RParen LBrace RBrace LBracket RBracket Semicolon Comma SQuote DQuote
%token Assign Eq Neq Lt Gt Lte Gte Plus Minus Mul Div Mod And Or Not Dot
%token Backslash

%start CompUnit

%type <expr> Decl VarDecl MoreVarDef VarDef Dimension InitVal FuncDef FuncFParams
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

CompUnit : CompUnit Decl    {
                                root = $$ = newTreeNode(STR, "CompUnit", 0); 
                                appendChild($$, $1);
                                appendChild($$, $2);
                            }
        | CompUnit FuncDef  {
                                root = $$ = newTreeNode(STR, "CompUnit", 0); 
                                appendChild($$, $1);
                                appendChild($$, $2);
                            }
        | FuncDef           {
                                root = $$ = newTreeNode(STR, "CompUnit", 0); 
                                appendChild($$, $1);
                            }
        | Decl              {
                                root = $$ = newTreeNode(STR, "CompUnit", 0); 
                                appendChild($$, $1);
                            }
        ;

Decl : VarDecl  {
                    $$ = newTreeNode(STR, "Decl", 0);
                    appendChild($$, $1);
                }

VarDecl : TyInt VarDef MoreVarDef Semicolon;
MoreVarDef : Comma VarDef MoreVarDef
        | 
        ;

VarDef : Ident Assign InitVal
        | Ident Dimension 
        ;
        
Dimension : LBracket Int RBracket Dimension
        |
        ;

InitVal : Exp;

FuncDef : TyInt Ident LParen FuncFParams RParen Block
        | TyVoid Ident LParen FuncFParams RParen Block
        ;

FuncFParams : FuncFParam MoreFuncFParams
        | 
        ;

FuncFParam : TyInt Ident DimParams;

DimParams : LBracket RBracket MoreDimParams
        |
        ;

MoreDimParams : LBracket Int RBracket MoreDimParams
        |
        ;

MoreFuncFParams : Comma FuncFParam MoreFuncFParams
        |
        ;

Block : LBrace BlockItems RBrace;

BlockItems : BlockItem BlockItems
        | 
        ;

BlockItem : Decl
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
