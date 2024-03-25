#include "ast/ast.h"

#include <fmt/core.h>

extern int yyparse();

NodePtr root;

int main(int argc, char **argv) {
    yyparse();
    print_expr(static_cast<ExprPtr>(root));
    fmt::print("Hello, World!\n");
    return 0;
}
