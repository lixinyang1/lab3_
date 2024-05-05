#pragma once
#include <cstdint>
#include <type_traits>
#include <string>
#include <vector>

enum Type
{
    VAL,
    STR
};

struct TreeNode
{
    Type type;
    char *str;
    int val;
    std::vector<TreeNode *> child;
};

TreeNode *newTreeNode(Type type, char *str, int val);

void appendChild(TreeNode *x, TreeNode *y);