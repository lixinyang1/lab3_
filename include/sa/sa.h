#pragma once
#include "ast/ast.h"
#include <vector>
#include <map>
#include <iostream>
#include <fmt/core.h>

template<typename T>
class Table {
private:
    // actually funcTable only has one map in vector
    std::vector<std::map<std::string, T>> tableVector;
    string cur_func_name;
public:
    Table() {
        // TODO: complete your code here
        // do nothing yeah!
        tableVector.clear();
    }

    std::vector<std::map<std::string, T>> getTable() {
        return tableVector;
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
        if (tableVector[sz-1].count(entry_name)) {
            return -1;
        }
        tableVector[sz-1][entry_name]=entry_type;
        // print_table();
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

    T* lookup(std::string entry_name) {
        // TODO: complete your code here
        int sz=tableVector.size();
        for (int i = tableVector.size() - 1; i >= 0; i--) {
            if (tableVector[i].count(entry_name))
                return new T(tableVector[i][entry_name]);
        }
        return nullptr;
    }

    // T lookup_value(std::string entry_name) {
    //     // TODO: complete your code here
    //     int sz=tableVector.size();
    //     for (int i = tableVector.size() - 1; i >= 0; i--) {
    //         if (tableVector[i].count(entry_name)) 
    //             return tableVector[i][entry_name];
    //     }
    //     fmt::print("wrong lookup\n");
    // }

    void print_table() {
        cout << "table: " << endl;
        for (int i = 0; i < tableVector.size(); i++) {
            cout << "env " << i << ": ";
            for (auto pair : tableVector[i]) {
                cout << pair.first << " ";
            }
            cout << endl;
        }
    }

};


int semantic_analysis(TreeRoot *root);
int sa(ExprPtr node, Table<varType>& varTable, Table<FuncType>& funcTable, bool innerBlock);

varType type_check(ExprPtr node, Table<varType> varTable, Table<FuncType> funcTable);