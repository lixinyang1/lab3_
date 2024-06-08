#include "ir/ir.h"
#include "sa/sa.h"
#include <cassert>
#include <fmt/core.h>
#include <iostream>
#include <fstream>
#include <sstream>

void ir_translate(TreeRoot *root, string output_file, bool debug) {
    Table<varType> varTable;
    Table<FuncType> funcTable;
    Table<Value *> allocaTable;
    Table<Function*> funcValueTable;
    Module* module = new Module();
    ir(root, varTable, funcTable, allocaTable, funcValueTable, module);
    std::ofstream outFile(output_file);
    module->print(std::cout, debug); // for debug
    module->print(outFile, debug);
    outFile.close();
}

void ir(ExprPtr node, Table<varType>& varTable, Table<FuncType>& funcTable, Table<Value *>& allocaTable,
        Table<Function*>& funcValueTable, Module* module) {

    assert(node != nullptr);

    // root node
    if (auto *root = node->as<TreeRoot *>()) {

        fmt::print("translating root\n");
        varTable.new_env();
        funcTable.new_env();
        allocaTable.new_env();
        funcValueTable.new_env();
        funcTable.add_one_entry("getint", FuncType({}, INT));
        funcTable.add_one_entry("getch", FuncType({}, INT));
        funcTable.add_one_entry("getarray", FuncType({varType(INT, 1)}, INT));
        funcTable.add_one_entry("putint", FuncType({varType(INT, 0)}, VOID));
        funcTable.add_one_entry("putch", FuncType({varType(INT, 0)}, VOID));
        funcTable.add_one_entry("putarray", FuncType({varType(INT, 0), varType(INT, 1)}, VOID));
        funcTable.add_one_entry("starttime", FuncType({}, VOID));
        funcTable.add_one_entry("stoptime", FuncType({}, VOID));

        // create GlobalVariable Value
        vector<ExprPtr> gVarAssign;
        for (int i = root->rootItems.size() - 1; i >= 0; i--) {
            if (auto *varDeclNode = root->rootItems[i]->as<TreeVarDecl *>()) {
                for (int i = 0; i < varDeclNode->assignStmtNodes.size(); i++) {
                    
                    auto *assignStmtNode = varDeclNode->assignStmtNodes[i]->as<TreeAssignStmt *>();
                    auto *varExpNode = assignStmtNode->lhs->as<TreeVarExpr *>();
                    // add new var to now table.
                    std::vector<int> dimension_size;
                    int num_elements = 1;
                    for (int i = 0; i < varExpNode->index.size(); i++) {
                        auto *numberNode = varExpNode->index[i]->as<TreeNumber *>();
                        dimension_size.push_back(numberNode->value);
                        num_elements *= numberNode->value;
                    }
                    varTable.add_one_entry(varExpNode->name, varType(varDeclNode->type, 0, dimension_size));
                    // Create global variable
                    GlobalVariable *g_var = GlobalVariable::Create(Type::getIntegerTy(), num_elements, false, varExpNode->name, module);
                    allocaTable.add_one_entry(varExpNode->name, g_var);
                    gVarAssign.push_back(assignStmtNode);
                }
            }
        }

        // create runtime Fucntion Value
        auto func_map = funcTable.getTable()[0];
        for (auto it = func_map.begin(); it != func_map.end(); it++) {
            // the runtime function has external linkage
            Function* function = Function::Create(translate_func_type(it->second), true, it->first, module);
            funcValueTable.add_one_entry(it->first, function);
        }

        // create Function Value
        for (int i = root->rootItems.size() - 1; i >= 0; i--) {
            if (auto *funcDefNode = root->rootItems[i]->as<TreeFuncDef *>()) {
                // add new func to now table
                funcTable.add_one_entry(funcDefNode->funcName, funcDefNode->type);
                Function* function = Function::Create(translate_func_type(funcDefNode->type), false, funcDefNode->funcName, module);
                funcValueTable.add_one_entry(funcDefNode->funcName, function);
                BasicBlock* entry_bb = BasicBlock::Create(function);
                BasicBlock* ret_bb = BasicBlock::Create(function);
                entry_bb->setName("entry");
                ret_bb->setName("exit");
                // rename
                auto input_type = funcDefNode->input_params;
                for (int i = 0; i < function->arg_size(); i++) {
                    function->getArg(i)->setName(input_type[i].first);
                    fmt::print("name {}\n", input_type[i].first);
                }
            }
        }

        // create global var's Store Instruction to main 
        for (int i = 0; i < gVarAssign.size(); i++) {
            if (Function* main_func = module->getFunction("main"))
                translate_stmt(gVarAssign[i], varTable, funcTable, allocaTable, funcValueTable, &main_func->getEntryBlock());
        }

        // enter each function
        for (int i = root->rootItems.size() - 1; i >= 0; i--) { // attention to the order
            
            if (auto *funcDefNode = root->rootItems[i]->as<TreeFuncDef *>()) {

                varTable.new_env();
                allocaTable.new_env();

                Function *function = module->getFunction(funcDefNode->funcName);

                // create ret IR
                if (funcDefNode->type.returnType != VOID) {
                    AllocaInst* alloca_inst = AllocaInst::Create(Type::getIntegerTy(), 1, &function->getEntryBlock());
                    alloca_inst->setName("ret.addr");
                    allocaTable.add_one_entry("ret", alloca_inst);
                    LoadInst* load_inst = LoadInst::Create(alloca_inst, &function->back());
                    RetInst::Create(load_inst, &function->back());
                } else {
                    RetInst::Create(ConstantUnit::Create(), &function->back());
                }

                // alloca and store for input params
                for (int i = 0; i < funcDefNode->input_params.size(); i++) {
                    auto inputParam = funcDefNode->input_params[i];
                    varTable.add_one_entry(inputParam.first, inputParam.second);
                    if (inputParam.second.dimension == 0) {
                        AllocaInst* alloca_inst = AllocaInst::Create(Type::getIntegerTy(), 1, &function->getEntryBlock());
                        alloca_inst->setName(inputParam.first + ".addr");
                        allocaTable.add_one_entry(inputParam.first, alloca_inst);
                        Argument* argument = function->getArg(i);
                        StoreInst::Create(argument, alloca_inst, &function->getEntryBlock());
                    } else {
                        allocaTable.add_one_entry(inputParam.first, function->getArg(i));
                    }
                }

                funcTable.set_func_name(funcDefNode->funcName);
                // analysis block
                translate_stmt(funcDefNode->blockNode, varTable, funcTable, allocaTable, funcValueTable, &function->getEntryBlock(), false);
                
                varTable.quit_env();
                allocaTable.quit_env();

                continue;
            }
        }

        // end ir
        varTable.quit_env();
        funcTable.quit_env();
        allocaTable.quit_env();
        funcValueTable.quit_env();
    }
}

Value* translate_expr(ExprPtr expr, Table<varType>& varTable, Table<FuncType>& funcTable, Table<Value *>& allocaTable,
                    Table<Function*>& funcValueTable, BasicBlock* current_bb, bool is_lhs) {
    if (auto* number_exp = expr->as<TreeNumber*>()) {
        fmt::print("translating number expr\n");
        uint32_t number = number_exp->value;
        return ConstantInt::Create(number);
    }

    if (auto* var_exp = expr->as<TreeVarExpr*>()) {
        fmt::print("translating var expr\n");
        vector<ExprPtr> indices_expr = var_exp->index;

        if (indices_expr.size() == 0) {
            Value* lookup_res = *(allocaTable.lookup(var_exp->name));
            varType lookup_type = *(varTable.lookup(var_exp->name));
            if (GlobalVariable* global_var = dyn_cast<GlobalVariable>(lookup_res)) {
                if (lookup_type.dimension == 0)
                    return LoadInst::Create(global_var, current_bb);
                else
                    return global_var;
            }
            if (AllocaInst* local_var = dyn_cast<AllocaInst>(lookup_res)) {
                if (lookup_type.dimension == 0)
                    return LoadInst::Create(local_var, current_bb);
                else
                    return local_var;
            }
            if (Argument* argument_var = dyn_cast<Argument>(lookup_res)) {
                if (lookup_type.dimension == 0)
                    return LoadInst::Create(argument_var, current_bb);
                else
                    return argument_var;
            }
        
        } else {
            vector<Value*> indices;
            vector<optional<size_t>> bounds;
            varType var_type = *varTable.lookup(var_exp->name);
            for (int i = 0; i < var_type.dimension_size.size(); i++) {
                if (i < indices_expr.size())
                    indices.push_back(translate_expr(indices_expr[i], varTable, funcTable, allocaTable, funcValueTable, current_bb));
                else
                    indices.push_back(ConstantInt::Create(0));

                if (var_type.dimension_size[i] == 0)
                    bounds.push_back(nullopt);
                else
                    bounds.push_back((size_t)var_type.dimension_size[i]);
            }
            Value* lookup_res = *(allocaTable.lookup(var_exp->name));
            OffsetInst* offset_inst;
            if (GlobalVariable* global_var = dyn_cast<GlobalVariable>(lookup_res))
                offset_inst = OffsetInst::Create(Type::getIntegerTy(), global_var, indices, bounds, current_bb);
            if (AllocaInst* local_var = dyn_cast<AllocaInst>(lookup_res))
                offset_inst = OffsetInst::Create(Type::getIntegerTy(), local_var, indices, bounds, current_bb);
            if (Argument* argument = dyn_cast<Argument>(lookup_res))
                offset_inst = OffsetInst::Create(Type::getIntegerTy(), argument, indices, bounds, current_bb);
            if (is_lhs)
                return offset_inst;
            return LoadInst::Create(offset_inst, current_bb);
        }
    }

    if (auto* binary_exp = expr->as<TreeBinaryExpr*>()) {
        fmt::print("translating binary expr\n");
        auto* expr_lhs = translate_expr(binary_exp->lhs, varTable, funcTable, allocaTable, funcValueTable, current_bb);
        auto* expr_rhs = translate_expr(binary_exp->rhs, varTable, funcTable, allocaTable, funcValueTable, current_bb);
        return BinaryInst::Create(convertOpTypeToBinaryOps(binary_exp->op), expr_lhs, expr_rhs, Type::getIntegerTy(), current_bb);
    }

    if (auto* unary_exp = expr->as<TreeUnaryExpr*>()) {
        fmt::print("translating unary expr\n");
        auto* expr_zero = ConstantInt::Create(0);
        auto* expr = translate_expr(unary_exp->operand, varTable, funcTable, allocaTable, funcValueTable, current_bb);
        return BinaryInst::Create(convertOpTypeToBinaryOps(unary_exp->op), expr_zero, expr, Type::getIntegerTy(), current_bb);
    }

    if (auto* func_exp = expr->as<TreeFuncExpr*>()) {
        fmt::print("translating function expr\n");
        Function* function = *funcValueTable.lookup(func_exp->name);
        vector<Value*> arguments;
        // varNames is actully the vector of the exprs, just forget to change the name (x
        for (int i = 0; i < func_exp->varNames.size(); i++) {
            // scalar
            if (function->getArg(i)->getType() == Type::getIntegerTy())
                arguments.push_back(translate_expr(func_exp->varNames[i], varTable, funcTable, allocaTable, funcValueTable, current_bb));
            // array
            else
                arguments.push_back(translate_expr(func_exp->varNames[i], varTable, funcTable, allocaTable, funcValueTable, current_bb, true));
        }
        return CallInst::Create(function, arguments, current_bb);
    }
}

BasicBlock* translate_stmt(ExprPtr expr, Table<varType>& varTable, Table<FuncType>& funcTable, Table<Value *>& allocaTable, 
                            Table<Function*>& funcValueTable, BasicBlock* current_bb, bool inner_block) {
    // block
    if (auto *blockNode = expr->as<TreeBlock *>()) {
        fmt::print("translating block\n");
        // enter new env
        if (inner_block) {
            varTable.new_env();
            allocaTable.new_env();
        }

        // may have no Decl or Stmt
        for (int i = 0; i < blockNode->blockItems.size(); i++) {
            current_bb = translate_stmt(blockNode->blockItems[i], varTable, funcTable, allocaTable, funcValueTable, current_bb);
            if(current_bb == nullptr)
                break;
        }

        // quit env
        if (inner_block) {
            varTable.quit_env();
            allocaTable.quit_env();
        }
        return current_bb;
    }

    // var decl
    if (auto *varDeclNode = expr->as<TreeVarDecl *>()) {
        fmt::print("translating varDel\n");
        for (int i = 0; i < varDeclNode->assignStmtNodes.size(); i++) {
            auto *assignStmtNode = varDeclNode->assignStmtNodes[i]->as<TreeAssignStmt *>();
            auto *varExpNode = assignStmtNode->lhs->as<TreeVarExpr *>();
            size_t num_element = 1;
            std::vector<int> dimension_size;
            for (int i = 0; i < varExpNode->index.size(); i++) {
                auto *numberNode = varExpNode->index[i]->as<TreeNumber *>();
                dimension_size.push_back(numberNode->value);
                num_element *= numberNode->value;
            }
            varTable.add_one_entry(varExpNode->name, varType(varDeclNode->type, 0, dimension_size));

            AllocaInst* alloca_inst = AllocaInst::Create(Type::getIntegerTy(), num_element, &current_bb->getParent()->getEntryBlock().back());
            allocaTable.add_one_entry(varExpNode->name, alloca_inst);
            current_bb = translate_stmt(assignStmtNode, varTable, funcTable, allocaTable, funcValueTable, current_bb);
        }
        return current_bb;
    }   

    // assign statement
    if (auto* assign_stmt = expr->as<TreeAssignStmt *>()) {
        auto* varExpNode = assign_stmt->lhs->as<TreeVarExpr *>();
        fmt::print("translating assign stmt: " + varExpNode->name + "\n");
        if (assign_stmt->rhs == nullptr)
            return current_bb;
        // don't need offset for left var
        if (varExpNode->index.size() == 0) {
            Value* lookup_res = *(allocaTable.lookup(varExpNode->name)); // left var
            auto* result_value = translate_expr(assign_stmt->rhs, varTable, funcTable, allocaTable, funcValueTable, current_bb, false);
            if (GlobalVariable* global_var = dyn_cast<GlobalVariable>(lookup_res))
                StoreInst::Create(result_value, global_var, current_bb);
            if (AllocaInst* local_var = dyn_cast<AllocaInst>(lookup_res))
                StoreInst::Create(result_value, local_var, current_bb);
            if (Argument* argument = dyn_cast<Argument>(lookup_res))
                StoreInst::Create(result_value, argument, current_bb);
            return current_bb;
        // need offset for left var
        } else {
            auto* left_value = translate_expr(varExpNode, varTable, funcTable, allocaTable, funcValueTable, current_bb, true);
            auto* result_value = translate_expr(assign_stmt->rhs, varTable, funcTable, allocaTable, funcValueTable, current_bb, false);
            StoreInst::Create(result_value, left_value, current_bb);
            return current_bb;         
        }
    }

    // if statement
    if (auto* if_stmt = expr->as<TreeIfStmt*>()) {
        fmt::print("translating if stmt\n");
        Function* function = current_bb->getParent();
        // auto* cond_value = translate_expr(if_stmt->conditionExp, varTable, funcTable, allocaTable, funcValueTable, current_bb);
        vector<pair<ExprPtr, RightOp>> split_exprs;
        if (if_stmt->elseStmtNode == nullptr) {
            BasicBlock* true_bb = BasicBlock::Create(function, &function->back());  
            BasicBlock* exit_bb = BasicBlock::Create(function, &function->back());  
            // BranchInst::Create(true_bb, exit_bb, cond_value, current_bb);
            split_shortcut_expr(if_stmt->conditionExp, split_exprs, NONE);
            BasicBlock* first_cond_bb = translate_shortcut_expr(split_exprs, true_bb, exit_bb, varTable, funcTable, allocaTable, funcValueTable);
            JumpInst::Create(first_cond_bb, current_bb);

            BasicBlock* true_exit_bb = translate_stmt(if_stmt->trueStmtNode, varTable, funcTable, allocaTable, funcValueTable, true_bb);
            if (true_exit_bb != nullptr) // don't have return 
                JumpInst::Create(exit_bb, true_exit_bb);
            return exit_bb;
        } else {
            BasicBlock* true_bb = BasicBlock::Create(function, &function->back());
            BasicBlock* false_bb = BasicBlock::Create(function, &function->back());
            BasicBlock* exit_bb = BasicBlock::Create(function, &function->back());
            // BranchInst::Create(true_bb, false_bb, cond_value, current_bb);
            split_shortcut_expr(if_stmt->conditionExp, split_exprs, NONE);
            BasicBlock* first_cond_bb = translate_shortcut_expr(split_exprs, true_bb, false_bb, varTable, funcTable, allocaTable, funcValueTable);
            JumpInst::Create(first_cond_bb, current_bb);

            BasicBlock* true_exit_bb = translate_stmt(if_stmt->trueStmtNode, varTable, funcTable, allocaTable, funcValueTable, true_bb);
            if (true_exit_bb != nullptr)
                JumpInst::Create(exit_bb, true_exit_bb);
            BasicBlock* false_exit_bb = translate_stmt(if_stmt->elseStmtNode, varTable, funcTable, allocaTable, funcValueTable, false_bb);
            if (false_exit_bb != nullptr)
                JumpInst::Create(exit_bb, false_exit_bb); 
            return exit_bb;       
        }
    }

    // while statement
    if (auto* while_stmt = expr->as<TreeWhileStmt*>()) {
        fmt::print("translating while stmt\n");
        Function* function = current_bb->getParent();
        // BasicBlock* cond_bb = BasicBlock::Create(function, &function->back());  
        BasicBlock* body_bb = BasicBlock::Create(function, &function->back());  
        BasicBlock* exit_bb = BasicBlock::Create(function, &function->back());  
        // JumpInst::Create(cond_bb, current_bb);
        // auto* cond_value = translate_expr(while_stmt->conditionExp, varTable, funcTable, allocaTable, funcValueTable, cond_bb);
        // BranchInst::Create(body_bb, exit_bb, cond_value, cond_bb);
        vector<pair<ExprPtr, RightOp>> split_exprs;
        split_shortcut_expr(while_stmt->conditionExp, split_exprs, NONE);
        BasicBlock* first_cond_bb = translate_shortcut_expr(split_exprs, body_bb, exit_bb, varTable, funcTable, allocaTable, funcValueTable);
        JumpInst::Create(first_cond_bb, current_bb);

        BasicBlock* body_exit_bb = translate_stmt(while_stmt->trueStmtNode, varTable, funcTable, allocaTable, funcValueTable, body_bb);
        if (body_exit_bb != nullptr)
            // JumpInst::Create(cond_bb, body_exit_bb);
            JumpInst::Create(first_cond_bb, body_exit_bb);
        return exit_bb;
    }

    // return stmt
    if (auto* return_stmt = expr->as<TreeReturnStmt*>()) {
        fmt::print("translating return stmt\n");
        BasicBlock* ret_bb = &current_bb->getParent()->back();
        if (return_stmt->returnExp != nullptr) {
            AllocaInst* ret_alloca = dyn_cast<AllocaInst>(*allocaTable.lookup("ret"));
            auto* ret_value = translate_expr(return_stmt->returnExp, varTable, funcTable, allocaTable, funcValueTable, current_bb);
            StoreInst::Create(ret_value, ret_alloca, current_bb);
        }
        JumpInst::Create(ret_bb, current_bb);
        return nullptr;
    }

    // exp
    if (expr->is<TreeUnaryExpr*>() || expr->is<TreeBinaryExpr*>() || expr->is<TreeVarExpr*>() || 
        expr->is<TreeFuncExpr*>() || expr->is<TreeNumber*>()) {
        translate_expr(expr, varTable, funcTable, allocaTable, funcValueTable, current_bb);
        return current_bb;
    }
}


// split highest expr into small exprs
void split_shortcut_expr(ExprPtr expr, vector<pair<ExprPtr, RightOp>>& split_exprs, RightOp rop) {
    if (auto* binary_expr = expr->as<TreeBinaryExpr*>()) {
        if (binary_expr->op == OP_Lor) {
            split_shortcut_expr(binary_expr->lhs, split_exprs, OR);
            split_shortcut_expr(binary_expr->rhs, split_exprs, rop);

        } else if (binary_expr->op == OP_Land) {
            split_shortcut_expr(binary_expr->lhs, split_exprs, AND);
            split_shortcut_expr(binary_expr->rhs, split_exprs, rop);

        } else {
            split_exprs.push_back(make_pair(expr, rop));
        }

    } else {
        split_exprs.push_back(make_pair(expr, rop));
    }
}


BasicBlock* translate_shortcut_expr(vector<pair<ExprPtr, RightOp>>& split_exprs, BasicBlock* final_true_bb, BasicBlock* final_false_bb, 
    Table<varType>& varTable, Table<FuncType>& funcTable, Table<Value *>& allocaTable, Table<Function*>& funcValueTable) {
    vector<BasicBlock*> bbs;
    for (int i = 0; i < split_exprs.size(); i++)
        bbs.push_back(BasicBlock::Create(final_true_bb->getParent(), final_true_bb));
    BasicBlock* true_bb = final_true_bb;
    BasicBlock* false_bb = final_false_bb;
    for (int i = split_exprs.size()-1; i >= 0; i--) {
        auto* cond_value = translate_expr(split_exprs[i].first, varTable, funcTable, allocaTable, funcValueTable, bbs[i]);

        if (split_exprs[i].second == NONE) {
            BranchInst::Create(true_bb, false_bb, cond_value, bbs[i]);

        } else if (split_exprs[i].second == OR) {
            true_bb = final_true_bb;
            false_bb = bbs[i+1];
            BranchInst::Create(true_bb, false_bb, cond_value, bbs[i]);

        } else {
            true_bb = bbs[i+1];
            BranchInst::Create(true_bb, false_bb, cond_value, bbs[i]);
        }
    }
    return bbs[0];
}


// translate FuncType to FunctionType*
FunctionType * translate_func_type(FuncType func_type) {
    Type *ret_type;
    if (func_type.returnType == VOID)
        ret_type = Type::getUnitTy();
    else
        ret_type = Type::getIntegerTy();
    std::vector<Type *> params;
    for (int i = 0; i < func_type.inputType.size(); i++) {
        if (func_type.inputType[i].dimension == 0)
            params.push_back(Type::getIntegerTy());
        else
            params.push_back(PointerType::get(Type::getIntegerTy()));
    }
    return FunctionType::get(ret_type, params);
}


BinaryInst::BinaryOps convertOpTypeToBinaryOps(OpType opType) {
    switch (opType) {
        case OP_Add:
            return BinaryInst::BinaryOps::Add;
        case OP_Sub :
            return BinaryInst::BinaryOps::Sub;
        case OP_Neg :
            return BinaryInst::BinaryOps::Sub;
        case OP_Mul:
            return BinaryInst::BinaryOps::Mul;
        case OP_Div:
            return BinaryInst::BinaryOps::Div;
        case OP_Mod:
            return BinaryInst::BinaryOps::Mod;
        case OP_Lt:
            return BinaryInst::BinaryOps::Lt;
        case OP_Gt:
            return BinaryInst::BinaryOps::Gt;
        case OP_Le:
            return BinaryInst::BinaryOps::Le;
        case OP_Ge:
            return BinaryInst::BinaryOps::Ge;
        case OP_Eq:
            return BinaryInst::BinaryOps::Eq;
        case OP_Lnot:
            return BinaryInst::BinaryOps::Eq;
        case OP_Ne:
            return BinaryInst::BinaryOps::Ne;
        case OP_Land:
            return BinaryInst::BinaryOps::And;
        case OP_Lor:
            return BinaryInst::BinaryOps::Or;
        case OP_Lxor:
            return BinaryInst::BinaryOps::Xor;
        default:
            throw std::invalid_argument("Unsupported OpType for BinaryOps conversion");
    }
}