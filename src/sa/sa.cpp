#include "sa/sa.h"
#include <cassert>
#include <fmt/core.h>
#include <iostream>


int semantic_analysis(TreeRoot *root) {
    Table<varType> varTable;
    Table<FuncType> funcTable;
    cout << "start semantic analysis" << endl;
    return sa(root, varTable, funcTable, true);
}

int sa(ExprPtr node, Table<varType> varTable, Table<FuncType> funcTable, bool innerBlock) {

    assert(node != nullptr);

    // root node
    if (auto *root = node->as<TreeRoot *>()) {

        cout << "------------------------" << endl;
        fmt::print("analysis root\n");
        varTable.new_env();
        funcTable.new_env();
        funcTable.add_one_entry("getint", FuncType({}, INT));
        funcTable.add_one_entry("getch", FuncType({}, INT));
        funcTable.add_one_entry("getarray", FuncType({varType(INT, 1)}, INT));
        funcTable.add_one_entry("putint", FuncType({varType(INT, 0)}, VOID));
        funcTable.add_one_entry("putch", FuncType({varType(INT, 0)}, VOID));
        funcTable.add_one_entry("putarray", FuncType({varType(INT, 0), varType(INT, 1)}, VOID));
        funcTable.add_one_entry("starttime", FuncType({}, VOID));
        funcTable.add_one_entry("stoptime", FuncType({}, VOID));


        // the code at least has one Decl or FuncDef
        cout << "root items number: " << root->rootItems.size() << endl;
        for (int i = root->rootItems.size() - 1; i >= 0; i--) { // attention to the order
            
            // varDecl, may have several var. eg. int a = 1, b, c = 1;
            if (auto *varDeclNode = root->rootItems[i]->as<TreeVarDecl *>()) {  
                cout << "------------------------" << endl;
                fmt::print("analysis varDel\n");
                for (int i = 0; i < varDeclNode->assignStmtNodes.size(); i++) {
                    
                    auto *assignStmtNode = varDeclNode->assignStmtNodes[i]->as<TreeAssignStmt *>();
                    auto *varExpNode = assignStmtNode->lhs->as<TreeVarExpr *>();
                    // add new var to now table.
                    cout << "add " << varExpNode->name << " to table" << endl;
                    std::vector<int> dimension_size;
                    for (int i = 0; i < varExpNode->index.size(); i++) {
                        auto *numberNode = varExpNode->index[i]->as<TreeNumber *>();
                        dimension_size.push_back(numberNode->value);
                    }
                    if (varTable.add_one_entry(varExpNode->name, varType(varDeclNode->type, 0, dimension_size)) != 0) {
                        cout << "cannot redeclare var" << endl;
                        return -1;
                    }

                    // if the varDecl has assign expression, do type_check. eg. int a = 1;
                    if (type_check(assignStmtNode, varTable, funcTable).type != 0)
                        return -1;
                }

                continue;
                
            }

            // funcDef
            if (auto *funcDefNode = root->rootItems[i]->as<TreeFuncDef *>()) {

                varTable.new_env();

                cout << "------------------------" << endl;
                fmt::print("analysis funcDef\n");
                // add new func to now table
                if (funcTable.add_one_entry(funcDefNode->funcName, funcDefNode->type) != 0) {
                    cout << "cannot redefine func" << endl;
                    return -1;
                }

                for (auto inputParam : funcDefNode->input_params)
                    varTable.add_one_entry(inputParam.first, inputParam.second);

                funcTable.set_func_name(funcDefNode->funcName);
                // analysis block
                if (sa(funcDefNode->blockNode, varTable, funcTable, false) != 0)
                    return -1;
                
                varTable.quit_env();

                continue;
            }
        }

        // end sa
        varTable.quit_env();
        funcTable.quit_env();
        return 0;
    }

    // block node
    if (auto *blockNode = node->as<TreeBlock *>()) {

        cout << "------------------------" << endl;
        fmt::print("analysis block\n");
        // enter new env
        if (innerBlock)
            varTable.new_env();

        cout << "block items number: " << blockNode->blockItems.size() << endl;
        // may have no Decl or Stmt
        for (int i = 0; i < blockNode->blockItems.size(); i++) {
            
            // varDecl eg. int a = 1;
            if (auto *varDeclNode = blockNode->blockItems[i]->as<TreeVarDecl *>()) {

                cout << "------------------------" << endl;
                fmt::print("analysis varDel\n");
                for (int i = 0; i < varDeclNode->assignStmtNodes.size(); i++) {
                    // 
                    auto *assignStmtNode = varDeclNode->assignStmtNodes[i]->as<TreeAssignStmt *>();
                    auto *varExpNode = assignStmtNode->lhs->as<TreeVarExpr *>();
                    // add new var to now table.
                    cout << "add " << varExpNode->name << " to table" << endl; 
                    std::vector<int> dimension_size;
                    for (int i = 0; i < varExpNode->index.size(); i++) {
                        auto *numberNode = varExpNode->index[i]->as<TreeNumber *>();
                        dimension_size.push_back(numberNode->value);
                    }
                    if (varTable.add_one_entry(varExpNode->name, varType(varDeclNode->type, 0, dimension_size)) != 0) {
                        cout << "cannot redeclare var" << endl;
                        return -1;
                    }

                    // if the varDecl has assign expression, do type_check. eg. int a = 1;
                    if (type_check(assignStmtNode, varTable, funcTable).type == FAIL) 
                        return -1;
                }

                continue;
            }

            // stmt
            if (sa(blockNode->blockItems[i], varTable, funcTable, true) != 0)
                return -1;
        }

        // quit env
        if (innerBlock)
            varTable.quit_env();
        return 0;
    }


    // stmt
    // assignStmt eg. a = b;
    if (auto *assignStmtNode = node->as<TreeAssignStmt *>()) {

        cout << "------------------------" << endl;
        fmt::print("analysis assignStmt\n");
        // do type_check
        if (type_check(assignStmtNode, varTable, funcTable).type == FAIL)
            return -1;
        
        return 0;
    }

    // ExpStmt
    // unaryExp eg. !a, !f(x)
    if (auto *unaryExpNode = node->as<TreeUnaryExpr *>()) {

        cout << "------------------------" << endl;
        fmt::print("analysis unaryExp\n");
        // do type_check
        if (type_check(unaryExpNode, varTable, funcTable).type == FAIL)
            return -1;

        return 0;
    }

    // binaryExp
    if (auto *binaryExpNode = node->as<TreeBinaryExpr *>()) {

        cout << "------------------------" << endl;
        fmt::print("analysis binaryExp\n");
        // do type_check
        if (type_check(binaryExpNode, varTable, funcTable).type == FAIL)
            return -1;

        return 0;
    }

    // funcExp eg. f(x)
    if (auto *funcExpNode = node->as<TreeFuncExpr *>()) {

        cout << "------------------------" << endl;
        fmt::print("analysis funcExp\n");
        // do type_check
        if (type_check(funcExpNode, varTable, funcTable).type == FAIL)
            return -1;

        return 0;
    }

    // varExp eg. a, a[1]
    if (auto *varExpNode = node->as<TreeVarExpr *>()) {

        cout << "------------------------" << endl;
        fmt::print("analysis varExp\n");
        // do type_check
        if (type_check(varExpNode, varTable, funcTable).type == FAIL)
            return -1;

        return 0;
    }

    // number
    if (auto *numberNode = node->as<TreeNumber *>()) {

        cout << "------------------------" << endl;
        fmt::print("analysis number\n");
        // do type_check
        if (type_check(numberNode, varTable, funcTable).type == FAIL)
            return -1;

        return 0;
    }

    // ifStmt
    if (auto *ifStmtNode = node->as<TreeIfStmt *>()) {

        cout << "------------------------" << endl;
        fmt::print("analysis ifStmt\n");
        // check if condition
        varType condition_type = type_check(ifStmtNode->conditionExp, varTable, funcTable);
        if (condition_type.type != INT || condition_type.dimension != 0)
            return -1;

        // analysis true stmt
        if (sa(ifStmtNode->trueStmtNode, varTable, funcTable, true) != 0) {
            return -1;
        }

        // if has else
        if (ifStmtNode->elseStmtNode != nullptr) {
            
            // analysis else stmt
            if (sa(ifStmtNode->elseStmtNode, varTable, funcTable, true) != 0)
                return -1;
        }

        return 0;
    }

    // while stmt
    if (auto *whileStmtNode = node->as<TreeWhileStmt *>()) {

        cout << "------------------------" << endl;
        fmt::print("analysis whileStmt\n");
        // check while condition
        varType condition_type = type_check(whileStmtNode->conditionExp, varTable, funcTable);
        if (condition_type.type != INT || condition_type.dimension != 0)
            return -1;

        // analysis true stmt
        if (sa(whileStmtNode->trueStmtNode, varTable, funcTable, true) != 0)
            return -1;

        return 0;
    }

    // break stmt, no need to check

    // continue stmt, no need to check

    // return stmt
    if (auto *returnStmtNode = node->as<TreeReturnStmt *>()) {

        cout << "------------------------" << endl;
        fmt::print("analysis returnStmt\n");
        // check return type and the function type
        if (type_check(returnStmtNode, varTable, funcTable).type == FAIL)
            return -1;

        return 0;
    }

    return 0;
}

bool equal_for_func_call(varType input_type, varType func_param_type) {
    if (input_type.type != func_param_type.type || input_type.dimension != func_param_type.dimension)
        return false;
    if (input_type.dimension <= 1)
        return true;
    for (int i = 1; i < input_type.dimension; i++) {
        if (input_type.dimension_size[i] != func_param_type.dimension_size[i])
            return false;
    }
    return true;
}


/* Actually, we can merge the function of searching for the 
    var/func into type_check, while searching for the type.
*/

varType type_check(TreeExpr * node, Table<varType> varTable, Table<FuncType> funcTable) {

    // assign stmt
    /* check if the type on the left is the same as the right. */
    if (auto *assignStmtNode = node->as<TreeAssignStmt *>()) {
        cout << "check assignStmt " << endl;
        varType left = type_check(assignStmtNode->lhs, varTable, funcTable);
        if (assignStmtNode->rhs == nullptr) {
            cout << "pass(in varDecl)" << endl;
            return left;
        }
        varType right = type_check(assignStmtNode->rhs, varTable, funcTable);
        if (left.type == FAIL || right.type == FAIL) {
            cout << "fail: right ot left not pass" << endl;
            return varType(FAIL, 0);
        }
        if (left.type != INT || left.dimension != 0 || right.type != INT || right.dimension != 0) {
            cout << "fail: assign type must be INT" << endl;
            return varType(FAIL, 0);
        } else {
            cout << "pass" << endl;
            return left;
        }

    }

    // return stmt
    /* check if the return type is the same as the declared one. */
    if (auto *returnStmtNode = node->as<TreeReturnStmt *>()) {
        cout << "check returnStmt " << endl;
        // TODO: complete your code here
        varType res;
        if (returnStmtNode->returnExp == nullptr) {
            res.type = VOID;
        } else {
            res=type_check(returnStmtNode->returnExp,varTable,funcTable);
        }
        if (res.type==FAIL) {
            cout << "fail: returnExp not pass" << endl;
            return varType(FAIL, 0);
        }
        FuncType *typPtr = funcTable.lookup(funcTable.get_cur_func_name());
        if (typPtr == nullptr) {
            cout << "fail: current function not found" << endl;
            return varType(FAIL, 0);
        }
        FuncType typ = *typPtr;
        if ((res.type == VOID && typ.returnType == VOID) || 
            (res.type == INT && res.dimension == 0 && typ.returnType == INT)) {
            cout << "pass" << endl;
            return res;
        }
        cout << "fail: type dosen't match" << endl;
        return varType(FAIL,0);
    }

    // unary exp
    /* return the result type */
    if (auto *unaryExpNode = node->as<TreeUnaryExpr *>()) {
        cout << "check unaryExp " << endl;
        varType expType = type_check(unaryExpNode->operand, varTable, funcTable);
        if (expType.type != INT && expType.dimension != 0) {
            cout << "fail: unary exp type must be INT" << endl;
            return varType(FAIL, 0);
        }
        return type_check(unaryExpNode->operand, varTable, funcTable);
    }

    // binary exp
    /* check the type of two operands. */
    if (auto *binaryExpNode = node->as<TreeBinaryExpr *>()) {
        cout << "check binaryExp" << endl;
        // TODO: complete your code here
        varType left=type_check(binaryExpNode->lhs,varTable,funcTable);
        varType right=type_check(binaryExpNode->rhs,varTable,funcTable);
        if (left.type==FAIL||right.type==FAIL) {
            cout << "fail: right ot left not pass" << endl;
            return varType(FAIL,0);
        }
        if (left.type != INT || left.dimension != 0 || right.type != INT || right.dimension != 0) {
            cout << "fail: binary type must be INT" << endl;
            return varType(FAIL, 0);
        } else {
            cout << "pass" << endl;
            return left;
        }
    }

    // func exp
    /* check the type of the input params and the declared ones. */
    if (auto *funcExpNode = node->as<TreeFuncExpr *>()) {
        // TODO: complete your code here
        cout << "check funcExp " << endl;
        std::vector<ExprPtr> name_vec=funcExpNode->varNames;
        string name = funcExpNode->name;
        FuncType *funcPtr = funcTable.lookup(name);
        if (funcPtr == nullptr) {
            cout << "fail: no function named \"" << name << "\"" << endl;
            return varType(FAIL, 0);
        }
        FuncType func = *funcPtr;
        std::vector<varType> input_param=func.inputType;
        int cur=0;
        if (input_param.size() != name_vec.size()) {
            cout << "fail: params number doesn't match" << endl;
            return varType(FAIL, 0);
        }
        for (int i = 0; i < name_vec.size(); i++) {
            varType inputType = type_check(name_vec[i], varTable, funcTable);
            if (inputType.type == FAIL) {
                cout << "fail: input varExp not pass" << endl;
                return varType(FAIL, 0);
            }

            if (!equal_for_func_call(inputType, input_param[i])) {
                cout << "fail: input param doesn't match" << endl;
                return varType(FAIL, 0);
            }
        }
        cout << "pass" << endl;
        return varType(func.returnType, 0);
    }

    // var exp
    /* check the use of the var and the declared one. */
    if (auto *varExpNode = node->as<TreeVarExpr *>()) {
        cout << "check varExp " << endl;
        // TODO: complete your code here
        for (int i = 0; i < varExpNode->index.size(); i++) {
            varType index_type = type_check(varExpNode->index[i], varTable, funcTable);
            if (index_type.type != INT || index_type.dimension != 0) {
                cout << "fail: index must be INT" << endl;
                return varType(FAIL, 0);
            }
        }
        varType *typePtr = varTable.lookup(varExpNode->name);
        if (typePtr == nullptr) {
            cout << "fail: var named \"" << varExpNode->name << "\" not found" << endl;
            return varType(FAIL, 0);
        }
        varType type = *typePtr;
        if (type.dimension < varExpNode->index.size()) {
            cout << "fail: var doesn't have such dimension" << endl;
            return varType(FAIL, 0);
        }
        cout << "pass" << endl;
        std::vector<int> dimension_size;
        for (int i = varExpNode->index.size(); i < type.dimension; i++)
            dimension_size.push_back(type.dimension_size[i]);
        return varType(type.type, 0, dimension_size);
    }

    // number
    /* just return INT */
    if (auto *numberNode = node->as<TreeNumber *>()) {

        return varType(INT, 0);
    }

}