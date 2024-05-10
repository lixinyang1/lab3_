#pragma once
#include <cstdint>
#include <type_traits>
#include <string>
#include <vector>
#include <map>

using namespace std;

enum Type
{
    INT,
    VOID,
    ALL,
    FAIL // to show that Table.lookup didn't find var or func,
         // or type doesn't match
};

struct varType
{

    // actually only need INT, but can be 'VOID' to
    // represent that the function is void
    Type type;

    // 0: scalar, >0: array
    int dimension;

    varType(Type type, int d) : type(type), dimension(d) {}
};

struct FuncType
{

    // input types
    std::vector<varType> inputType;

    // return type, here varType.type can be 'VOID'
    varType returnType;
};

enum OpType
{
#define OpcodeDefine(x, s) x,
#include "common/common.def"
};

enum NodeType
{
#define TreeNodeDefine(x) x,
#include "common/common.def"
};

struct Node;
using NodePtr = Node *;
struct TreeExpr;
using ExprPtr = TreeExpr *;
struct TreeType;

struct Node
{
    NodeType node_type;
    Node(NodeType type) : node_type(type) {}
    template <typename T>
    bool is()
    {
        return node_type == std::remove_pointer_t<T>::this_type;
    }
    template <typename T>
    T as()
    {
        if (is<T>())
            return static_cast<T>(this);
        return nullptr;
    }
    template <typename T>
    T as_unchecked() { return static_cast<T>(this); }
};

struct TreeExpr : public Node
{
    TreeExpr(NodeType type) : Node(type) {}
};

// Stmt node
struct TreeAssignStmt : public TreeExpr
{
    // TODO: complete your code here;
    constexpr static NodeType this_type = ND_AssignExpr;
    ExprPtr lhs, rhs;
    TreeAssignStmt(ExprPtr lhs, ExprPtr rhs) : TreeExpr(ND_AssignExpr), lhs(lhs), rhs(rhs) {}
};

struct TreeIfStmt : public TreeExpr
{
    // TODO: complete your code here;
    constexpr static NodeType this_type = ND_IfElseExpr;
    ExprPtr conditionExp, trueStmtNode, elseStmtNode;
    TreeIfStmt(ExprPtr a, ExprPtr b, ExprPtr c) : TreeExpr(ND_IfElseExpr), conditionExp(a), trueStmtNode(b), elseStmtNode(c) {}
};

struct TreeWhileStmt : public TreeExpr
{
    // TODO: complete your code here;
    constexpr static NodeType this_type = ND_LoopExpr;
    ExprPtr conditionExp, trueStmtNode;
    TreeWhileStmt(ExprPtr a, ExprPtr b) : TreeExpr(ND_LoopExpr), conditionExp(a), trueStmtNode(b) {}
};

struct TreeWhileControlStmt : public TreeExpr
{
    constexpr static NodeType this_type = ND_LoopControlExpr;
    string controlType;
    TreeWhileControlStmt(string controlType) : TreeExpr(ND_LoopControlExpr), controlType(controlType) {}
};

struct TreeReturnStmt : public TreeExpr
{
    // TODO: complete your code here;
    constexpr static NodeType this_type = ND_ReturnExpr;
    ExprPtr returnExp;
    TreeReturnStmt(ExprPtr returnExp) : TreeExpr(ND_ReturnExpr), returnExp(returnExp) {}
};

// Exp node
struct TreeUnaryExpr : public TreeExpr
{
    constexpr static NodeType this_type = ND_UnaryExpr;
    OpType op;
    ExprPtr operand;
    TreeUnaryExpr(OpType op, ExprPtr operand)
        : TreeExpr(this_type), op(op), operand(operand)
    {
    }
};

struct TreeBinaryExpr : public TreeExpr
{
    constexpr static NodeType this_type = ND_BinaryExpr;
    OpType op;
    ExprPtr lhs, rhs;
    TreeBinaryExpr(OpType op, ExprPtr lhs, ExprPtr rhs)
        : TreeExpr(this_type), op(op), lhs(lhs), rhs(rhs)
    {
    }
};

struct TreeFuncExpr : public TreeExpr
{
    // TODO: complete your code here;
    constexpr static NodeType this_type = ND_FuncExpr;
    std::string name;
    std::vector<ExprPtr> varNames;

    void append(string x)
    {
        varNames.push_back(x);
    }
    TreeFuncExpr(std::string name, map<std::string, varType> input_params) : TreeExpr(this_type), name(name), varNames(varNames) {}
};

struct TreeVarExpr : public TreeExpr
{
    // TODO: complete your code here;
    constexpr static NodeType this_type = ND_ValExpr;
    std::string name;
    std::vector<int> index; // eg. a[1][2] means pushing 1 and 2 to vector
    void append(int x)
    {
        index.push_back(x);
    }
    TreeVarExpr(std::string name, std::vector<int> index) : TreeExpr(this_type), name(name), index(index) {}
};

struct TreeNumber : public TreeExpr
{
    constexpr static NodeType this_type = ND_IntegerLiteral;
    int64_t value;
    void inc() { value++; }
    TreeNumber(int64_t value) : TreeExpr(this_type), value(value) {}
};

// other node
struct TreeRoot : public TreeExpr
{
    // TODO: complete your code here;
    constexpr static NodeType this_type = ND_Root;
    std::vector<ExprPtr> rootItems;
    void append(ExprPtr x)
    {
        rootItems.push_back(x);
    }
    TreeRoot(std::vector<ExprPtr> rootItems) : TreeExpr(this_type), rootItems(rootItems) {}
};

struct TreeVarDecl : public TreeExpr
{
    Type type;
    std::vector<ExprPtr> assignStmtNodes;
    constexpr static NodeType this_type = ND_VarDecl;
    TreeVarDecl(Type type, vector<ExprPtr> assignStmtNodes) : TreeExpr(this_type), type(type), assignStmtNodes(assignStmtNodes) {}
    void append(ExprPtr x)
    {
        assignStmtNodes.push_back(x);
    }
};

struct TreeBlock : public TreeExpr
{
    // TODO: complete your code here;
    constexpr static NodeType this_type = ND_VarDecl;
    std::vector<ExprPtr> blockItems;
    TreeBlock(std::vector<ExprPtr> blockItems) : TreeExpr(this_type), blockItems(blockItems) {}
    void append(ExprPtr x)
    {
        blockItems.push_back(x);
    }
};

struct TreeFuncDef : public TreeExpr
{
    // TODO: complete your code here;
    constexpr static NodeType this_type = ND_FuncDef;
    std::string funcName;
    std::map<std::string, varType> input_params;
    FuncType type;
    ExprPtr blockNode;
    TreeFuncDef(std::string funcName, std::map<std::string, varType> input_params, FuncType type, ExprPtr blockNode) : TreeExpr(this_type), funcName(funcName), input_params(input_params), type(type), blockNode(blockNode) {}
    void append(string x, varType y)
    {
        input_params[x] = y;
    }
};

// A possible helper function dipatch based on the type of `TreeExpr`
void print_expr(ExprPtr exp, std::string prefix = "", std::string ident = "");