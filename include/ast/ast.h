#pragma once
#include <cstdint>
#include <type_traits>

#define OpcodeDefine(x, s) x,
enum OpType {
#include "common/common.def"
};

#define NodeTypeDefine(x) x,
enum NodeType {
#include "common/common.def"
};

struct Node;
using NodePtr = Node*;
struct TreeExpr;
using ExprPtr = TreeExpr*;
struct TreeType;

struct Node {
    NodeType node_type;
    Node(NodeType type) : node_type(type) {}
    template <typename T> bool is() {
        return node_type == std::remove_pointer_t<T>::this_type;
    }
    template <typename T> T as() {
        if (is<T>())
            return static_cast<T>(this);
        return nullptr;
    }
    template <typename T> T as_unchecked() { return static_cast<T>(this); }
};

#define THIS(x) constexpr inline static NodeType this_type = x

struct TreeExpr : public Node {
    TreeExpr(NodeType type) : Node(type) {}
};
struct TreeBinaryExpr : public TreeExpr {
    THIS(ND_BinaryExpr);
    OpType op;
    ExprPtr lhs, rhs;
    TreeBinaryExpr(OpType op, ExprPtr lhs, ExprPtr rhs)
        : TreeExpr(this_type), op(op), lhs(lhs), rhs(rhs) {
    }
};

struct TreeUnaryExpr : public TreeExpr {
    THIS(ND_UnaryExpr);
    OpType op;
    ExprPtr operand;
    TreeUnaryExpr(OpType op, ExprPtr operand)
        : TreeExpr(this_type), op(op), operand(operand) {
    }
};

struct TreeIntegerLiteral : public TreeExpr {
    THIS(ND_IntegerLiteral);
    int64_t value;
    TreeIntegerLiteral(int64_t value) : TreeExpr(this_type), value(value) {}
};

#undef THIS