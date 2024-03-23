#include <fmt/core.h>
#include <ast/ast.h>
#include <string>
#include <cassert>

extern int yyparse();

NodePtr root;

inline std::string op_str(OpType op_type) {
    std::string ret;
    switch (op_type) {
#define OpcodeDefine(x, s)     \
        case x: ret = s;       \
        break;
#include <common/common.def>
    }
    return ret;
}

void print_expr(ExprPtr exp, std::string prefix = "", std::string ident = "") {
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
    if (auto* lit = exp->as < TreeIntegerLiteral *>()) {
        fmt::print("Int {}\n", lit->value);
    }
}

int main() {
    yyparse();
    print_expr(static_cast<ExprPtr>(root));
    fmt::print("Hello, World!\n");
    return 0;
}
