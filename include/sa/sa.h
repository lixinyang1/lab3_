#include "ast/ast.h"
#include <vector>
#include <map>



int semantic_analysis(ExprPtr root);
int sa(ExprPtr node, Table<varType> varTable, Table<FuncType> funcTable);

varType type_check(TreeExpr * node, Table<varType> varTable, Table<FuncType> funcTable);


template<typename T>
class Table {
private:
    // actually funcTable only has one map in vector
    std::vector<std::map<std::string, T>> tableVector;
public:
    Table() {
        // TODO: complete your code here
    }

    int add_one_entry(std::string entry_name, T entry_type) {
        // TODO: complete your code here

        return 0;
    }

    void new_env() {
        // TODO: complete your code TODO: here
    }

    void quit_env() {
        // TODO: complete your code here
    }

    T lookup(std::string entry_name) {
        // TODO: complete your code here


    }

};