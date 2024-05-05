
#include "ast/ast.h"

#include <fmt/core.h>
#include <cassert>

static std::string op_str(OpType op_type)
{
    switch (op_type)
    {
#define OpcodeDefine(x, s) \
    case x:                \
        return s;
#include "common/common.def"
    default:
        return "<unknown>";
    }
}

void print_expr(ExprPtr exp, std::string prefix, std::string ident)
{
    assert(exp != nullptr);
    fmt::print(prefix);
    if (auto *bin_op = exp->as<TreeBinaryExpr *>())
    {
        fmt::print("BinOp \"{}\"\n", op_str(bin_op->op));
        print_expr(bin_op->lhs, ident + "├─ ", ident + "│  ");
        print_expr(bin_op->rhs, ident + "└─ ", ident + "   ");
    }
    if (auto *un_op = exp->as<TreeUnaryExpr *>())
    {
        fmt::print("UnOp \"{}\"\n", op_str(un_op->op));
        print_expr(un_op->operand, ident + "└─ ", ident + "   ");
    }
    if (auto *lit = exp->as<TreeIntegerLiteral *>())
    {
        fmt::print("Int {}\n", lit->value);
    }
}

TreeNode *newTreeNode(Type type, char *str, int val)
{
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    node->type = type;
    if (type == Type::STR)
        node->str = strdup(str);
    else
        node->val = val;
    node->child.clear();
    return node;
}

void appendChild(TreeNode *x, TreeNode *y)
{
    x->child.push_back(y);
}

void print_expr(TreeNode *cur, int prefix)
{
    if (cur->type == Type::STR)
        printf("%s\n", cur->str);
    else
        printf("%d\n", cur->val);
    for (auto x : cur->child)
    {
        for (int i = 1; i <= prefix; i++)
            -printf("\t");
        printf("└─ ");
        print_expr(x, prefix + 1);
    }
}