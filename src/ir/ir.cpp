#include "ir/ir.h"
#include "ir/type.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

Use::Use(Value *Parent) : Parent(Parent) { }

Use::Use(Use &&Other) {
    Parent = Other.Parent;
    Val = Other.Val;
    Other.Parent = nullptr;
    Other.Val = nullptr;

    Val->removeUse(Other);
    Val->addUse(*this);
}

void Use::removeFromList(Value *Used) {
    auto &UseListHead = Used->UserIter;
    if (getUser() == *UseListHead)
        ++UseListHead;
    ListNode<Use>::removeFromList();
}

void Use::addToList(Value *Used) {
    auto &UseListHead = Used->UserIter; 
    if (!UseListHead.getNodePtr()) {
        UseListHead->insertBefore(this);
        UseListHead--;
    } else {
        UseListHead = Value::UserIterType(this);
    }
}

void Use::set(Value *V) {
    if (Val)
        removeFromList(Val);
    Val = V;
    if (V)
        V->addUse(*this);
}


Value::Value(Type *Ty, unsigned scid)
    : SubclassID(scid), Ty(Ty) {} 

unsigned Value::getNumUses() const {
    auto view = getUserView();
    unsigned size = 0;
    for (auto I = view.begin(), IE = view.end(); I != IE; ++I)
        ++size;
    return size;
}

void Value::replaceAllUsesWith(Value *V) {
    std::vector<UserIterType> Uses;
    auto view = getUserView();
    for (auto I = view.begin(), IE = view.end(); I != IE; ++I)
        Uses.push_back(I);
    
    for (auto &I: Uses)
        I->set(V);
}

void Value::setName(std::string_view Name) {
    // 'Constant' has no name
    if (isa<Constant>(this))
        return;
    this->Name = Name;
}

bool Value::hasName() const {
    return !Name.empty();
}


Constant::Constant(Type *Ty, unsigned VT)
    : Value(Ty, VT) {}

ConstantInt::ConstantInt(std::uint32_t Val)
    : Constant(Type::getIntegerTy(), Value::ConstantIntVal) {}

ConstantInt *ConstantInt::Create(std::uint32_t Val) {
    return new ConstantInt(Val);
}

ConstantUnit::ConstantUnit()
    : Constant(Type::getUnitTy(), Value::ConstantUnitVal) {}

ConstantUnit *ConstantUnit::Create() {
    return new ConstantUnit;
}


Instruction::Instruction(Type *Ty, unsigned Opcode, 
                         const std::vector<Value *> &Ops,
                         Instruction *InsertBefore) 
    : Value(Ty, Value::InstructionVal + Opcode) {
    for (auto *Op: Ops) {
        Operands.emplace_back(this);
        Operands.back().set(Op);
    }
    if (InsertBefore) {
        BasicBlock *BB = InsertBefore->getParent();
        assert(BB && "Instruction to insert before is not in a basic block!");
        insertInto(BB, BasicBlock::iterator(InsertBefore));
    }
}

Instruction::Instruction(Type *Ty, unsigned Opcode,
                         const std::vector<Value *> &Ops, 
                         BasicBlock *InsertAtEnd)
    : Value(Ty, Value::InstructionVal + Opcode) {
    for (auto *Op: Ops) {
        Operands.emplace_back(this);
        Operands.back().set(Op);
    }
    if (InsertAtEnd) {
        insertInto(InsertAtEnd, InsertAtEnd->end());
    }
}


void Instruction::setParent(BasicBlock *BB) {
    Parent = BB;
}

void Instruction::insertBefore(Instruction *InsertBefore) {
    insertBefore(BasicBlock::iterator(InsertBefore));
}


void Instruction::insertBefore(BasicBlock::iterator InsertBefore) {
    insertBefore(*InsertBefore->getParent(), InsertBefore);
}

void Instruction::insertBefore(BasicBlock &BB, InstListType::iterator IT) {
    BB.getInstList().insert(IT, this);
    setParent(&BB);
}

void Instruction::insertAfter(Instruction *InsertPos) {
    BasicBlock *DestParent = InsertPos->getParent();

    DestParent->getInstList().insertAfter(BasicBlock::iterator(InsertPos), this);
    setParent(DestParent);
}

BasicBlock::iterator Instruction::insertInto(BasicBlock *BB, BasicBlock::iterator IT) {
    assert(getParent() == nullptr && "Expected detached instruction!");
    assert((IT == BB->end() || IT->getParent() == BB) && "IT not in ParentBB");
    insertBefore(*BB, IT);
    setParent(BB);
    return BasicBlock::iterator(this);
}

void Instruction::removeFromParent() {
    getParent()->getInstList().remove(BasicBlock::iterator(this));
}

BasicBlock::iterator Instruction::eraseFromParent() {
    return getParent()->getInstList().erase(BasicBlock::iterator(this));
}



BinaryInst::BinaryInst(BinaryOps Op, Value *LHS, Value *RHS, Type *Ty,
                       Instruction *InsertBefore) 
    : Instruction(Ty, Op, std::vector<Value *> {LHS, RHS}, InsertBefore) {

}

BinaryInst::BinaryInst(BinaryOps Op, Value *LHS, Value *RHS, Type *Ty,
                       BasicBlock *InsertAtEnd)
    : Instruction(Ty, Op, std::vector<Value *> {LHS, RHS}, InsertAtEnd) {

}

BinaryInst *BinaryInst::Create(BinaryOps Op, Value *LHS, Value *RHS, Type *Ty,
                              Instruction *InsertBefore) {
    assert(LHS->getType() == RHS->getType() &&
        "Cannot create binary operator with two operands of differing type!");
    return new BinaryInst(Op, LHS, RHS, Ty, InsertBefore);
}
    
BinaryInst *BinaryInst::Create(BinaryOps Op, Value *LHS, Value *RHS, Type *Ty,
                              BasicBlock *InsertAtEnd) {
    auto *Res = Create(Op, LHS, RHS, Ty);
    Res->insertInto(InsertAtEnd, InsertAtEnd->end());
    return Res;
}

AllocaInst::AllocaInst(Type *PointerType, std::size_t NumElements, Instruction *InsertBefore)
    : Instruction(PointerType::get(PointerType), Instruction::Alloca, 
    { }, InsertBefore) {
    assert(!PointerType->isUnitTy() && "Cannot allocate () type!");
}

AllocaInst::AllocaInst(Type *PointerType, std::size_t NumElements, BasicBlock *InsertAtEnd)
    : Instruction(PointerType::get(PointerType), Instruction::Alloca, 
    { }, InsertAtEnd) {
    assert(!PointerType->isUnitTy() && "Cannot allocate () type!");
}


AllocaInst *AllocaInst::Create(Type *PointeeTy, std::uint64_t NumElements,
                              Instruction *InsertBefore) {
    return new AllocaInst(PointeeTy, NumElements, InsertBefore);
}

AllocaInst *AllocaInst::Create(Type *PointeeTy, std::uint64_t NumElements,
                              BasicBlock *InsertAtEnd) {
    auto *Res = Create(PointeeTy, NumElements);
    Res->insertInto(InsertAtEnd, InsertAtEnd->end());
    return Res;
}

StoreInst::StoreInst(Value *Val, Value *Ptr, Instruction *InsertBefore)
    : Instruction(Type::getUnitTy(), Instruction::Store, { Val, Ptr }, InsertBefore) {

}

StoreInst::StoreInst(Value *Val, Value *Ptr, BasicBlock *InsertAtEnd)
    : Instruction(Type::getUnitTy(), Instruction::Store, { Val, Ptr }, InsertAtEnd) {
        
}

StoreInst *StoreInst::Create(Value *Val, Value *Ptr,
                             Instruction *InsertBefore) {
    return new StoreInst(Val, Ptr, InsertBefore);
}

StoreInst *StoreInst::Create(Value *Val, Value *Ptr,
                             BasicBlock *InsertAtEnd) {
    auto *Res = Create(Val, Ptr);
    Res->insertInto(InsertAtEnd, InsertAtEnd->end());
    return Res;
}


void OffsetInst::AssertOK() const {
    assert(getOperand(0)->getType()->isPointerTy() && "Offset pointer is not of the pointer type!");
    assert(dyn_cast<PointerType>(getOperand(0)->getType())->getElementType() == ElementTy &&
           "Element type of offset does not match the type of pointer!");
    assert(getNumUses() == (unsigned)(bounds().size() + 1) && "Num of indices and bounds does not match!");
}

OffsetInst::OffsetInst(Type *PointeeTy,
               std::vector <Value *> &Ops,
               std::vector <std::optional<std::size_t>> &Bounds,
               Instruction *InsertBefore) 
    : Instruction(PointerType::get(PointeeTy), Instruction::Offset, Ops, InsertBefore),
      ElementTy(PointeeTy),
      Bounds(Bounds) {
    AssertOK();
}

OffsetInst::OffsetInst(Type *PointeeTy,
               std::vector <Value *> &Ops,
               std::vector <std::optional<std::size_t>> &Bounds,
               BasicBlock *InsertAtEnd) 
    : Instruction(PointerType::get(PointeeTy), Instruction::Offset, Ops, InsertAtEnd),
      ElementTy(PointeeTy),
      Bounds(Bounds) {
    AssertOK();
}

OffsetInst *OffsetInst::Create(Type *PointeeTy, Value *Ptr,
                              std::vector <Value *> &Indices,
                              std::vector <std::optional<std::size_t>> &Bounds,
                              Instruction *InsertBefore) {
    std::vector<Value *> Ops { Ptr };
    Ops.insert(Ops.end(), Indices.begin(), Indices.end());
    return new OffsetInst(PointeeTy, Ops, Bounds, InsertBefore);
}

OffsetInst *OffsetInst::Create(Type *PointeeTy, Value *Ptr,
                              std::vector <Value *> &Indices,
                              std::vector <std::optional<std::size_t>> &Bounds,
                              BasicBlock *InsertAtEnd) {

    std::vector<Value *> Ops { Ptr };
    Ops.insert(Ops.end(), Indices.begin(), Indices.end());
    auto *Res = new OffsetInst(PointeeTy, Ops, Bounds);
    Res->insertInto(InsertAtEnd, InsertAtEnd->end());
    return Res;
}


bool OffsetInst::accumulateConstantOffset(std::size_t &Offset) const {
    std::vector<std::size_t> indices;
    std::vector<std::size_t> no_option_bounds;
    for (auto BI = Bounds.begin() + 1; BI != Bounds.end(); ++BI) {
        no_option_bounds.emplace_back(BI->value());
    }
    no_option_bounds.emplace_back(1);

    for (auto &Op: getOperands()) {
        if (auto *Index = dyn_cast<ConstantInt>(Op.get())) {
            indices.emplace_back(Index->getValue());
        } else {
            return false;
        }
    }

    size_t TotalOffset = 0;
    for (size_t dim = 0; dim < indices.size(); ++dim) {
        TotalOffset += indices[dim] * no_option_bounds[dim];
    }
    Offset += TotalOffset;

    return true;
}

void BranchInst::AssertOK() const {
    assert(getCondition()->getType()->isIntegerTy() &&
           "May only branch on integer predicate!");
}

BranchInst::BranchInst(BasicBlock *IfTrue, BasicBlock *IfFalse, Value *Cond, Instruction *InsertBefore)
    : Instruction(Type::getUnitTy(), Instruction::Br, 
    { Cond }, InsertBefore) {
    AssertOK();
} 

BranchInst::BranchInst(BasicBlock *IfTrue, BasicBlock *IfFalse, Value *Cond, BasicBlock *InsertAtEnd)
    : Instruction(Type::getUnitTy(), Instruction::Br,
    { Cond }, InsertAtEnd) {
    AssertOK();
}


BranchInst *BranchInst::Create(BasicBlock *IfTrue, BasicBlock *IfFalse, 
                              Value *Cond, 
                              Instruction *InsertBefore) {
    return new BranchInst(IfTrue, IfFalse, Cond, InsertBefore);
}

BranchInst *BranchInst::Create(BasicBlock *IfTrue, BasicBlock *IfFalse, 
                              Value *Cond, 
                              BasicBlock *InsertAtEnd) {
    auto *Res = Create(IfTrue, IfFalse, Cond);
    Res->insertInto(InsertAtEnd, InsertAtEnd->end());
    return Res;
}