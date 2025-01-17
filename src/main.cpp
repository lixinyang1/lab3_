#include "ast/ast.h"
#include "sa/sa.h"
#include "ir/ir.h"
#include <iostream>
#include <fmt/core.h>

extern int yyparse();

TreeRoot *root;
extern FILE *yyin;
// extern int semantic_analysis(TreeRoot *root);

int main(int argc, char **argv)
{
    yyin = fopen(argv[1], "r");
    fmt::print("Start parsing!\n");
    root = new TreeRoot(std::vector<ExprPtr>{});
    int result = yyparse();
    if (result != 0) return result;
    fmt::print("\nParse finish!\n");
    print_expr(root, "","",1);
    result = semantic_analysis(root);
    if (result != 0) return result;
    cout << "passing semantic analysis" << endl;
    ir_translate(root, argv[2]);
    // ir_translate(root, argv[2], true);  // debug version
    return result;
}
