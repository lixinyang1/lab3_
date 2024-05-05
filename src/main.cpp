#include "ast/ast.h"

#include <fmt/core.h>

extern int yyparse();

TreeNode *root;
extern FILE *yyin;
extern void print_expr(TreeNode *cur, int prefix);

int main(int argc, char **argv)
{
    yyin = fopen(argv[1], "r");
    fmt::print("Start parsing!\n");
    int result = yyparse();
    fmt::print("Parse finish!\n");
    print_expr(root, 0);
    return 0;
}
