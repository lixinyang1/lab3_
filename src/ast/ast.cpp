#include "ast/ast.h"

#include <fmt/core.h>
#include <cassert>

static std::string op_str(OpType op_type) {
    switch (op_type) {
#define OpcodeDefine(x, s)     \
        case x: return s;
#include "common/common.def"
    default:
        return "<unknown>";
    }
}

void print_expr(ExprPtr exp, std::string prefix, std::string ident) {
    assert(exp != nullptr);
    fmt::print(prefix);
    if (auto *bin_op = exp->as<TreeBinaryExpr *>()) {
        fmt::print("BinOp \"{}\"\n", op_str(bin_op->op));
        print_expr(bin_op->lhs, ident + "├─ ", ident + "│  ");
        print_expr(bin_op->rhs, ident + "└─ ", ident + "   ");
    }
    if (auto *un_op = exp->as<TreeUnaryExpr *>()) {
        fmt::print("UnOp \"{}\"\n", op_str(un_op->op));
        print_expr(un_op->operand, ident + "└─ ", ident + "   ");
    }
    if (auto* lit = exp->as < TreeNumber*>()) {
        fmt::print("Int {}\n", lit->value);
    }
    if (auto *it=exp->as<TreeAssignStmt*>()){
        fmt::print("AssignStmt\n");
        print_expr(it->lhs, ident + "├─ ", ident + "│  ");
        print_expr(it->rhs, ident + "└─ ", ident + "   ");
    }
    if (auto *it=exp->as<TreeIfStmt*>()){
        fmt::print("IfStmt\n");
        print_expr(it->conditionExp,ident + "├─ ", ident + "│  ");
        print_expr(it->trueStmtNode,ident + "└─ ", ident + "   ");
        fmt::print("ElseStmt\n");
        print_expr(it->elseStmtNode,ident + "└─ ", ident + "   ");
    }
    if (auto *it=exp->as<TreeWhileStmt*>()){
        fmt::print("WhileStmt\n");
        print_expr(it->conditionExp,ident + "├─ ", ident + "│  ");
        print_expr(it->trueStmtNode,ident + "└─ ", ident + "   ");
    }
    if (auto *it=exp->as<TreeReturnStmt*>()){
        fmt::print("ReturnStmt\n");
        print_expr(it->returnExp,ident + "└─ ", ident + "   ");
    }
    if (auto *it=exp->as<TreeFuncExpr*>()){
        fmt::print("Call "+it->name+"\n");
        for (auto x:it->varNames)
            print_expr(x,ident + "└─ ", ident + "   ");
    }
    if (auto *it=exp->as<TreeVarExpr*>()){
        fmt::print("ident "+it->name+"\n");
    }
    if (auto *it=exp->as<TreeRoot*>()){
        fmt::print("CompUnit\n");
        for (auto it1:(it->rootItems))
            print_expr(it1,ident + "└─ ", ident + "   ");
    }
    if (auto *it=exp->as<TreeVarDecl*>()){
        fmt::print("VarDecl\n");
        for (auto x:it->assignStmtNodes)
            print_expr(x,ident + "└─ ", ident + "   ");
    }
    if (auto *it=exp->as<TreeBlock*>()){
        fmt::print("Block\n");
        for (auto x:it->blockItems)
            print_expr(x,ident + "└─ ", ident + "   ");
    }
    if (auto *it=exp->as<TreeFuncDef*>()){
        fmt::print("FuncDef\n");
        print_expr(it->blockNode,ident + "└─ ", ident + "   ");
    }
}

void free_ast(ExprPtr exp){
    if (auto *bin_op = exp->as<TreeBinaryExpr *>()) {
        free_ast(bin_op->lhs);
        free_ast(bin_op->rhs);
    }
    if (auto *un_op = exp->as<TreeUnaryExpr *>()) {
        free_ast(un_op->operand);
    }
    if (auto *it=exp->as<TreeAssignStmt*>()){
        free_ast(it->lhs);
        free_ast(it->rhs);
    }
    if (auto *it=exp->as<TreeIfStmt*>()){
        free_ast(it->conditionExp);
        free_ast(it->trueStmtNode);
        free_ast(it->elseStmtNode);
    }
    if (auto *it=exp->as<TreeWhileStmt*>()){
        free_ast(it->conditionExp);
        free_ast(it->trueStmtNode);
    }
    if (auto *it=exp->as<TreeReturnStmt*>()){
        free_ast(it->returnExp);
    }
    if (auto *it=exp->as<TreeRoot*>()){
        for (auto it1:(it->rootItems))
            free_ast(it1);
    }
    if (auto *it=exp->as<TreeVarDecl*>()){
        for (auto x:it->assignStmtNodes)
            free_ast(x);
    }
    if (auto *it=exp->as<TreeBlock*>()){
        for (auto x:it->blockItems)
            free_ast(x);
    }
    if (auto *it=exp->as<TreeFuncDef*>())
        free_ast(it->blockNode);
    free(exp);
}