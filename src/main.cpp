#include <fmt/core.h>
#include <ast/ast.h>

extern int yyparse();

NodePtr root;

int main() {
  yyparse();
  fmt::print("Hello, World!\n");
  return 0;
}
