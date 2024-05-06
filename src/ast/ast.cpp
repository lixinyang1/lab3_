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
}