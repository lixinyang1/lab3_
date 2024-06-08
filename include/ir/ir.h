#pragma once
#include "../accsys/include/ir/ir.h"
#include "ast/ast.h"
#include "sa/sa.h"

enum RightOp {
    OR,
    AND,
    NONE
};

void ir_translate(TreeRoot *root, string output_file="output.acc", bool debug=false);
void ir(ExprPtr node, Table<varType>& varTable, Table<FuncType>& funcTable, 
        Table<Value*>& allocaTavle, Table<Function*>& funcValueTable, Module* module);

FunctionType * translate_func_type(FuncType func_type);

Value* translate_expr(ExprPtr expr, Table<varType>& varTable, Table<FuncType>& funcTable, Table<Value *>& allocaTable,
                     Table<Function*>& funcValueTable, BasicBlock* current_bb, bool is_lhs=false);
BasicBlock* translate_stmt(ExprPtr expr, Table<varType>& varTable, Table<FuncType>& funcTable, Table<Value *>& allocaTable,
                        Table<Function*>& funcValueTable, BasicBlock* current_bb, bool inner_block=true);
BinaryInst::BinaryOps convertOpTypeToBinaryOps(OpType opType);
void split_shortcut_expr(ExprPtr expr, vector<pair<ExprPtr, RightOp>>& split_exprs, RightOp rop);
BasicBlock* translate_shortcut_expr(vector<pair<ExprPtr, RightOp>>& split_exprs, BasicBlock* true_bb, BasicBlock* false_bb, 
    Table<varType>& varTable, Table<FuncType>& funcTable, Table<Value *>& allocaTable, Table<Function*>& funcValueTable);