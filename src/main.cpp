#include "ast/ast.h"
#include "sa/sa.h"
#include <iostream>
#include <fmt/core.h>

extern int yyparse();

TreeRoot *root;
extern FILE *yyin;
extern void print_expr(ExprPtr exp, std::string prefix, std::string ident);
// extern int semantic_analysis(TreeRoot *root);

int main(int argc, char **argv)
{
    yyin = fopen(argv[1], "r");
    fmt::print("Start parsing!\n");
    root = new TreeRoot(std::vector<ExprPtr>{});
    int result = yyparse();
    if (result != 0) return result;
    fmt::print("\nParse finish!\n");
    // print_expr(root, "","",1);
    result = semantic_analysis(root);
    if (result == 0)
        cout << "pass test" << endl;
    return result;
}
