#include "ast/ast.h"

#include <fmt/core.h>

extern int yyparse();

TreeRoot *root;
extern FILE *yyin;
extern void print_expr(ExprPtr exp, std::string prefix, std::string ident);

int main(int argc, char **argv)
{
    yyin = fopen(argv[1], "r");
    fmt::print("Start parsing!\n");
    root = new TreeRoot(std::vector<ExprPtr>{});
    int result = yyparse();
    fmt::print("Parse finish!\n");
    print_expr(root, "","",1);
    return result;
}
