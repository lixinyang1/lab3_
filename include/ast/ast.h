#pragma once
#include <cstdint>
#include <type_traits>
#include <string>
#include <vector>
#include <map>

enum Type {
    INT, VOID, 
    FAIL    // to show that Table.lookup didn't find var or func,
            // or type doesn't match
};

struct varType {

    // actually only need INT, but can be 'VOID' to 
    // represent that the function is void
    Type type;

    // 0: scalar, >0: array
    int dimension;

    varType(Type type, int d): type(type), dimension(d){}
};


struct FuncType {

    // input types
    std::vector<varType> inputType;

    // return type, here varType.type can be 'VOID'
    varType returnType;
};

enum OpType {
#define OpcodeDefine(x, s) x,
#include "common/common.def"
};

enum NodeType {
#define TreeNodeDefine(x) x,
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

struct TreeExpr : public Node {
    TreeExpr(NodeType type) : Node(type) {}
};


struct TreeNumber : public TreeExpr {
    constexpr static NodeType this_type = ND_Number;
    int64_t value;
    TreeNumber(int64_t value) : TreeExpr(this_type), value(value) {}
};


// Stmt node
struct TreeAssignStmt : public TreeExpr {
    // TODO: complete your code here;
    ExprPtr lhs, rhs;
};


struct TreeIfStmt : public TreeExpr {
    // TODO: complete your code here;
    ExprPtr conditionExp, trueStmtNode, elseStmtNode;
};


struct TreeWhileStmt : public TreeExpr {
    // TODO: complete your code here;
    ExprPtr conditionExp, trueStmtNode;
};


struct TreeReturnStmt : public TreeExpr {
    // TODO: complete your code here;
    ExprPtr returnExp;
};


// Exp node
struct TreeUnaryExpr : public TreeExpr {
    constexpr static NodeType this_type = ND_UnaryExpr;
    OpType op;
    ExprPtr operand;
    TreeUnaryExpr(OpType op, ExprPtr operand)
        : TreeExpr(this_type), op(op), operand(operand) {
    }
};

struct TreeBinaryExpr : public TreeExpr {
    constexpr static NodeType this_type = ND_BinaryExpr;
    OpType op;
    ExprPtr lhs, rhs;
    TreeBinaryExpr(OpType op, ExprPtr lhs, ExprPtr rhs)
        : TreeExpr(this_type), op(op), lhs(lhs), rhs(rhs) {
    }
};

struct TreeFuncExpr : public TreeExpr {
    // TODO: complete your code here;
    std::string name;
    std::map<std::string, varType> input_params;
};


struct TreeVarExpr : public TreeExpr {
    // TODO: complete your code here;
    std::string name;
    std::vector<int> index; // eg. a[1][2] means pushing 1 and 2 to vector
};

struct TreeNumber : public TreeExpr {
    constexpr static NodeType this_type = ND_Number;
    int64_t value;
    TreeNumber(int64_t value) : TreeExpr(this_type), value(value) {}
};


// other node
struct TreeRoot : public TreeExpr {
    // TODO: complete your code here;
    std::vector<ExprPtr> rootItems;
};

struct TreeVarDecl : public TreeExpr {
    // TODO: complete your code here;
    std::vector<std::string> varNames;
    std::vector<varType> types;
    std::vector<ExprPtr> assignStmtNodes;
};

struct TreeBlock : public TreeExpr {
    // TODO: complete your code here;
    std::vector<ExprPtr> blockItems;
};

struct TreeFuncDef : public TreeExpr {
    // TODO: complete your code here;
    std::string funcName;
    std::vector<std::string> varNames;
    FuncType type;
    ExprPtr blockNode;
};


// A possible helper function dipatch based on the type of `TreeExpr`
void print_expr(ExprPtr exp, std::string prefix = "", std::string ident = "");