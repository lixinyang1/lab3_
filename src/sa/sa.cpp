#include "sa/sa.h"
#include <cassert>

int semantic_analysis(ExprPtr root) {
    Table<varType> varTable;
    Table<FuncType> funcTable;
    return sa(root, varTable, funcTable);
}

int sa(ExprPtr node, Table<varType> varTable, Table<FuncType> funcTable) {

    assert(node != nullptr);

    // root node
    if (auto *root = node->as<TreeRoot *>()) {
        varTable.new_env();
        funcTable.new_env();

        // the code at least has one Decl or FuncDef
        for (int i = 0; i < root->rootItems.size(); i++) {

            // varDecl, may have several var. eg. int a = 1, b, c = 1;
            if (auto *varDeclNode = root->rootItems[i]->as<TreeVarDecl *>()) {

                for (int i = 0; i < varDeclNode->varNames.size(); i++) {
                    // add new var to now table.
                    if (varTable.add_one_entry(varDeclNode->varNames[i], varDeclNode->types[i]) != 0)
                        return -1;

                    // if the varDecl has assign expression, do type_check. eg. int a = 1;
                    if (varDeclNode->assignStmtNodes[i]->is<TreeAssignStmt>())
                        if (type_check(varDeclNode->assignStmtNodes[i], varTable, funcTable).type == 0)
                            return -1;
                }

                continue;
                
            }

            // funcDef
            if (auto *funcDefNode = root->rootItems[i]->as<TreeFuncDef *>()) {

                // add new func to now table
                if (funcTable.add_one_entry(funcDefNode->funcName, funcDefNode->type) != 0)
                    return -1;

                for (int i = 0; i < funcDefNode->varNames.size(); i++)
                    varTable.add_one_entry(funcDefNode->varNames[i], funcDefNode->type.inputType[i]);

                funcTable.set_func_name(funcDefNode->funcName);
                // analysis block
                if (sa(funcDefNode->blockNode, varTable, funcTable) != 0)
                    return -1;

                continue;
            }
        }

        // end sa
        varTable.quit_env();
        funcTable.quit_env();
    }

    // block node
    if (auto *blockNode = node->as<TreeBlock *>()) {

        // enter new env
        varTable.new_env();

        // may have no Decl or Stmt
        for (int i = 0; i < blockNode->blockItems.size(); i++) {

            // varDecl eg. int a = 1;
            if (auto *varDeclNode = blockNode->blockItems[i]->as<TreeVarDecl *>()) {

                for (int i = 0; i < varDeclNode->varNames.size(); i++) {
                    // add new var to now table.
                    if (varTable.add_one_entry(varDeclNode->varNames[i], varDeclNode->types[i]) != 0)
                        return -1;

                    // if the varDecl has assign expression, do type_check. eg. int a = 1;
                    if (varDeclNode->assignStmtNodes[i]->is<TreeAssignStmt>())
                        if (type_check(varDeclNode->assignStmtNodes[i], varTable, funcTable).type == 0)
                            return -1;
                }

                continue;
            }

            // stmt
            if (type_check(blockNode->blockItems[i], varTable, funcTable).type == FAIL)
                return -1;
        }

        // quit env
        varTable.quit_env();

    }

    // stmt
    // assignStmt eg. a = b;
    if (auto *assignStmtNode = node->as<TreeAssignStmt *>()) {

        // do type_check
        if (type_check(assignStmtNode, varTable, funcTable).type == FAIL)
            return -1;
        
        return 0;
    }

    // ExpStmt
    // unaryExp eg. !a, !f(x)
    if (auto *unaryExpNode = node->as<TreeUnaryExpr *>()) {

        // do type_check
        if (type_check(unaryExpNode, varTable, funcTable).type == FAIL)
            return -1;

        return 0;
    }

    // binaryExp
    if (auto *binaryExpNode = node->as<TreeBinaryExpr *>()) {

        // do type_check
        if (type_check(binaryExpNode, varTable, funcTable).type == FAIL)
            return -1;

        return 0;
    }

    // funcExp eg. f(x)
    if (auto *funcExpNode = node->as<TreeFuncExpr *>()) {

        // do type_check
        if (type_check(funcExpNode, varTable, funcTable).type == FAIL)
            return -1;

        return 0;
    }

    // varExp eg. a, a[1]
    if (auto *varExpNode = node->as<TreeVarExpr *>()) {

        // do type_check
        if (type_check(varExpNode, varTable, funcTable).type == FAIL)
            return -1;

        return 0;
    }

    // number
    if (auto *numberNode = node->as<TreeNumber *>()) {

        // do type_check
        if (type_check(numberNode, varTable, funcTable).type == FAIL)
            return -1;

        return 0;
    }

    // inner block
    if (auto *innerBlockNode = node->as<TreeBlock *>()) {

        // enter new env
        varTable.new_env();

        // analysis block
        if (sa(innerBlockNode, varTable, funcTable) != 0)
            return -1;
        
        // quit env
        varTable.quit_env();

        return 0;
    }

    // ifStmt
    if (auto *ifStmtNode = node->as<TreeIfStmt *>()) {

        // check if condition
        varType condition_type = type_check(ifStmtNode->conditionExp, varTable, funcTable);
        if (condition_type.type != INT || condition_type.dimension != 0);
            return -1;

        // analysis true stmt
        if (sa(ifStmtNode->trueStmtNode, varTable, funcTable) != 0)
            return -1;

        // if has else
        if (ifStmtNode->elseStmtNode != nullptr) {
            
            // analysis else stmt
            if (sa(ifStmtNode->elseStmtNode, varTable, funcTable) != 0)
                return -1;
        }

        return 0;
    }

    // while stmt
    if (auto *whileStmtNode = node->as<TreeWhileStmt *>()) {

        // check while condition
        varType condition_type = type_check(whileStmtNode->conditionExp, varTable, funcTable);
        if (condition_type.type != INT || condition_type.dimension != 0);
            return -1;

        // analysis true stmt
        if (sa(whileStmtNode->trueStmtNode, varTable, funcTable) != 0)
            return -1;

        return 0;
    }

    // break stmt, no need to check

    // continue stmt, no need to check

    // return stmt
    if (auto *returnStmtNode = node->as<TreeReturnStmt *>()) {

        // check return type and the function type
        if (type_check(returnStmtNode, varTable, funcTable).type == FAIL)
            return -1;

        return 0;
    }

    return 0;
}

bool equal(varType type1, varType type2) {
    return type1.type == type2.type && type1.dimension == type2.dimension;
}

/* Actually, we can merge the function of searching for the 
    var/func into type_check, while searching for the type.
*/

varType type_check(TreeExpr * node, Table<varType> varTable, Table<FuncType> funcTable) {

    // assign stmt
    /* check if the type on the left is the same as the right. */
    if (auto *assignStmtNode = node->as<TreeAssignStmt *>()) {
        varType left = type_check(assignStmtNode->lhs, varTable, funcTable);
        varType right = type_check(assignStmtNode->rhs, varTable, funcTable);
        if (left.type == FAIL || right.type == FAIL)
            return varType(FAIL, 0);
        if (equal(left, right))
            return left;
        return varType(FAIL, 0);

    }

    // return stmt
    /* check if the return type is the same as the declared one. */
    if (auto *returnStmtNode = node->as<TreeReturnStmt *>()) {
        // TODO: complete your code here
        varType res=type_check(returnStmtNode->returnExp,varTable,funcTable);
        if (res.type==FAIL) return varType(FAIL, 0);
        FuncType typ=funcTable.lookup(funcTable.get_cur_func_name());
        if (equal(res,typ.returnType)) return res;
        return varType(FAIL,0);
    }

    // unary exp
    /* return the result type */
    if (auto *unaryExpNode = node->as<TreeUnaryExpr *>()) {
        return type_check(unaryExpNode->operand, varTable, funcTable);
    }

    // binary exp
    /* check the type of two operands. */
    if (auto *binaryExpNode = node->as<TreeBinaryExpr *>()) {
        // TODO: complete your code here
        varType left=type_check(binaryExpNode->lhs,varTable,funcTable);
        varType right=type_check(binaryExpNode->rhs,varTable,funcTable);
        if (left.type==FAIL||right.type==FAIL) return varType(FAIL,0);
        if (equal(left,right)) return left;
        return varType(FAIL, 0);
    }

    // func exp
    /* check the type of the input params and the declared ones. */
    if (auto *funcExpNode = node->as<TreeFuncExpr *>()) {
        // TODO: complete your code here
        std::map<std::string,varType>input_params=funcExpNode->input_params;
        for (auto it=input_params.begin();it!=input_params.end();it++){
            if (equal(varTable.lookup(it->first),it->second)) continue;
            return varType(FAIL,0);
        }
        return funcTable.lookup(funcExpNode->name).returnType;
    }

    // var exp
    /* check the use of the var and the declared one. */
    if (auto *varExpNode = node->as<TreeVarExpr *>()) {
        // TODO: complete your code here
        varType var=varTable.lookup(varExpNode->name);
        if (var.dimension==varExpNode->index.size()) return var;
        return varType(FAIL, 0);
    }

    // number
    /* just return INT */
    if (auto *numberNode = node->as<TreeNumber *>()) {

        return varType(INT, 0);
    }

}