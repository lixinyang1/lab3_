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
    string cur_func_name;
public:
    Table() {
        // TODO: complete your code here
        //do nothing yeah!
        tableVector.clear();
    }

    void set_func_name(string x){
        cur_func_name=x;
    }

    std::string get_cur_func_name(){
        return cur_func_name;
    }

    int add_one_entry(std::string entry_name, T entry_type) {
        // TODO: complete your code here
        int sz=tableVector.size();
        tableVector[sz-1][entry_name]=entry_type;
        return 0;
    }

    void new_env() {
        // TODO: complete your code TODO: here
        std::map<std::string,T>tmp;
        tmp.clear();
        tableVector.push_back(tmp);
    }

    void quit_env() {
        // TODO: complete your code here
        tableVector.pop_back();
    }

    T lookup(std::string entry_name) {
        // TODO: complete your code here
        int sz=tableVector.size();
        if (tableVector[sz-1].count(entry_name)) return tableVector[sz-1][entry_name];
        else return FAIL;
    }

};