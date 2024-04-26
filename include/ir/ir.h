#pragma once

#include "utils/list.h"
#include "utils/casting.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <type_traits>
#include <unistd.h>
#include <utility>
#include <vector>
#include <optional>
#include <string_view>


class Type;
class Value;
class Instruction;
class Constant;
class BasicBlock;
class Function;
class Module;



/// \brief A use is a edge from the operands of a 'Value'
/// to the definition of the used 'Value', which maintains
/// 'use-def' chain.
class Use: public ListNode<Use> {
public:
    Use() = delete;
    Use(const Use &) = delete;
    Use(Use &&);

    /// 'use-def' chain and 'def-use' chain are double linked list.
    void addToList(Value *);
    void removeFromList(Value *);


    operator Value *() const { return Val; }
    Value *get() const { return Val; }
    Value *getUser() const { return Parent; }

    Value *operator->() { return Val; }
    const Value *operator->() const { return Val; }
    /// set the used value 'Val' of a Use.
    void set(Value *V);
    Value *operator=(Value *RHS) {
        set(RHS);
        return RHS;
    }
    const Use &operator=(const Use &RHS) {
        set(RHS.Val);
        return *this;
    }
public:
    /// FIXME: Priave constructor and destructor by design, 
    /// should be only called by subclasses of 'Value'.
    ///
    /// make std::vector<Use> happy.
    ~Use() {
        if (Val)
            removeFromList(Val);
    }
    /// make emplace_back happy.
    Use(Value *Parent);
private:
    // User value
    Value *Parent = nullptr;
    // Used value
    Value *Val = nullptr;
public:
    friend Value;
    friend Instruction;
    friend Constant;
};


class Value {
    const unsigned char SubclassID;
protected:
    Value(Type *Ty, unsigned scid);

public:
    Value(const Value &) = delete;
    Value &operator=(const Value &) = delete;

    /// Inner Use list operation, should be only be called by Use.
    void addUse(Use &U) { U.addToList(this); }
    void removeUse(Use &U) { U.removeFromList(this); }
    /// Traverse all the user of this 'Value'.
    /// 'def-use' chain maintained by a double linked list and
    /// represented by Use as well.
    using UserIterType = List<Use>::iterator;

    struct UserView {
        const UserIterType &UserStart;
        UserIterType begin() { return UserStart; }
        UserIterType end() {
            // null end iterator.
            return UserIterType(); 
        }
    };

    [[nodiscard]] UserView getUserView() { return UserView {.UserStart = UserIter}; }
    [[nodiscard]] const UserView getUserView() const { return UserView {.UserStart = UserIter}; }
    unsigned getNumUses() const;
    /// \brief This is an important method if you'd like to write
    /// a optimization pass on Accipit IR. It replace all the uses 
    /// of this value with a new value V by tracing 'def-use' chain.
    /// This method ONLY ensures the updates of this value.
    void replaceAllUsesWith(Value *V);
private:
    Type *Ty;
    UserIterType UserIter;
    std::string Name;
public:
    Type *getType() const { return Ty; }
    void setName(std::string_view);
    bool hasName() const;


    enum ValueKind {
#define ValueTypeDefine(subclass) subclass##Val,
#include "common/ir.def"

#define ConstantMarker(markname, constant) markname = constant##Val,
#include "common/ir.def"
    };

    /// Return an ID for the concrete type of Value.
    unsigned getValueID() const {
        return SubclassID;
    }

    friend Use;
};


/// Base class of all constants.
class Constant: public Value {
protected:
    Constant(Type *Ty, unsigned VT);
public:
    Constant(const Constant &) = delete;
    static bool classof(const Value *V) {
        return V->getValueID() >= ConstantFirstVal && V->getValueID() <= ConstantLastVal;
    }
};


class ConstantInt: public Constant {
    std::uint32_t value;
protected:
    ConstantInt(std::uint32_t Val);
public:
    static ConstantInt *Create(std::uint32_t Val);

public:
    std::uint32_t getValue() const { return value; }

    static bool classof(const Value *V) {
        return V->getValueID() == Value::ConstantIntVal;
    }
};

class ConstantUnit: public Constant {
protected:
    ConstantUnit();
public:
    static ConstantUnit *Create();
    
    static bool classof(const Value *V) {
        return V->getValueID() == Value::ConstantUnitVal;
    }
};


class Argument: public Value {
protected:
    Argument(Type *Ty, Function *Parent = nullptr);
};


class GlobalVariable: public Value {
protected:
    GlobalVariable(Type *Ty);
};

class Instruction: public Value, 
                   public ListNode<Instruction> {
public:
    Instruction(const Instruction &) = delete;
    Instruction &operator=(const Instruction &) = delete;

    Instruction(Type *Ty, unsigned Opcode, 
                const std::vector<Value *> &Ops,
                Instruction *InsertBefore);
    Instruction(Type *Ty, unsigned Opcode, 
                const std::vector<Value *> &Ops,
                BasicBlock *InsertAtEnd);

    using InstListType = List<Instruction>;
    using op_iter = typename std::vector<Use>::iterator;
    using const_op_iter = typename std::vector<Use>::const_iterator;
private:
    std::vector<Use> Operands;
    BasicBlock *Parent = nullptr;

    void setParent(BasicBlock *BB);
public:
    // Insert an unlinked instruction into a basic block immediately before the specified instruction.
    void insertBefore(Instruction *InsertPos);
    void insertBefore(InstListType::iterator InsertPos);
    // Inserts an unlinked instruction into ParentBB at position It and returns the iterator of the inserted instruction.
    void insertBefore(BasicBlock &BB, InstListType::iterator IT);
    void insertAfter(Instruction *InsertPos);
    InstListType::iterator insertInto(BasicBlock *ParentBB, InstListType::iterator IT);

 	// This method unlinks 'this' from the containing basic block, but does not delete it.
    void removeFromParent();
 	// This method unlinks 'this' from the containing basic block and deletes it.
    InstListType::iterator eraseFromParent();

    const BasicBlock *getParent() const { return Parent; }
    BasicBlock *getParent() { return Parent; }
    /// Return the integer reptresentation of the instruction opcode enumeration,
    /// which can be 'BinaryOps', 'MemoryOps', 'TerminatorOps' or 'OtherOps'.
    unsigned getOpcode() const { return getValueID() - InstructionVal; }
    /// Get the operands (use information) of Instruction, represented by
    /// a 'Use' class.
    const std::vector<Use> &getOperands() const { return Operands; }
    std::vector<Use> &getOperands() { return Operands; }
    Value *getOperand(unsigned index) const { return getOperands()[index]; }


    bool isBinaryOp() const { return isBinaryOp(getOpcode()); }
    bool isTerminator() const { return isTerminator(getOpcode()); }

    static inline bool isBinaryOp(unsigned Opcode) {
        return Opcode >= BinaryOpsBegin && Opcode <= BinaryOpsEnd;
    }

    static inline bool isTerminator(unsigned Opcode) {
        return Opcode >= TerminatorOpsBegin && Opcode <= TerminatorOpsEnd;
    }

    static bool classof(const Value *V) {
        return V->getValueID() >= InstructionVal;
    }

    // Instruction Opcode enumerations.
    enum BinaryOps {
#define   FirstBinaryInst(Num)                   BinaryOpsBegin = Num,
#define BinaryInstDefine( Num, Opcode, Subclass) Opcode = Num,
#define   LastBinaryInst( Num)                   BinaryOpsEnd = Num,
#include "common/ir.def"     
    };

    enum MemoryOps {
#define   FirstMemoryInst(Num)                   MemoryOpsBegin = Num,
#define MemoryInstDefine( Num, Opcode, Subclass) Opcode = Num,
#define   LastMemoryInst( Num)                   MemoryOpsEnd = Num,
#include "common/ir.def"
    };

    enum TerminatorOps {
#define   FirstTerminator(Num)                   TerminatorOpsBegin = Num,
#define TerminatorDefine( Num, Opcode, Subclass) Opcode = Num,
#define   LastTerminator( Num)                   TerminatorOpsEnd = Num,
#include "common/ir.def"
    };

    enum OtherOps {
#define   FirstOtherInst(Num)                   OtherInstBegin = Num,
#define OtherInstDefine( Num, Opcode, Subclass) Opcode = Num,
#define   LastOtherInst( Num)                   OtherInstEnd = Num,
#include "common/ir.def"
    };    
};


class BinaryInst: public Instruction {
protected:
    BinaryInst(BinaryOps Op, Value *LHS, Value *RHS, Type* Ty,
               Instruction *InsertBefore);
    
    BinaryInst(BinaryOps Op, Value *LHS, Value *RHS, Type *Ty,
               BasicBlock *InsertAtEnd);
public:
    static BinaryInst *Create(BinaryOps Op, Value *LHS, Value *RHS, Type *Ty,
                              Instruction *InsertBefore = nullptr);
    
    static BinaryInst *Create(BinaryOps Op, Value *LHS, Value *RHS, Type *Ty,
                              BasicBlock *InsertAtEnd);

public:
    /// forward to Create, useful when you know what type of instruction you are going to create.
#define BinaryInstDefine(Num, Opcode, Subclass) \
    static BinaryInst *Create##Opcode(Value *LHS, Value *RHS, Type *Ty) { \
        return Create(Instruction::Opcode, LHS, RHS, Ty); \
    }
#include "common/ir.def"
#define BinaryInstDefine(Num, Opcode, SubClass) \
    static BinaryInst *Create##Opcode(Value *LHS, Value *RHS, Type *Ty) { \
        return Create(Instruction::Opcode, LHS, RHS, Ty); \
    }

    static bool classof(const Value *V) { 
        return isa<Instruction>(V) && dyn_cast<Instruction>(V)->isBinaryOp();
    }
};

class AllocaInst: public Instruction {
protected:
    AllocaInst(Type *PointeeTy, std::size_t NumElements, Instruction *InsertBefore);
    AllocaInst(Type *PointeeTy, std::size_t NumElements, BasicBlock *InsertAtEnd);
private:
    Type *AllocatedType;
    std::size_t NumElements;
public:
    static AllocaInst *Create(Type *PointeeTy, std::size_t NumElements,
                              Instruction *InsertBefore = nullptr);
    static AllocaInst *Create(Type *PointeeTy, std::size_t NumElements,
                              BasicBlock *InsertAtEnd);

    Type *getAllocatedType() const { return AllocatedType; }
    std::size_t getNumElements() const { return NumElements; }


    static bool classof(const Instruction *I) {
        return I->getOpcode() == Instruction::Alloca;
    }

    static bool classof(const Value *V) {
        return isa<Instruction>(V) && classof(cast<Instruction>(V));
    }
};

class StoreInst: public Instruction {
protected:
    StoreInst(Value *Val, Value *Ptr, Instruction *InsertBefore);
    StoreInst(Value *Val, Value *Ptr, BasicBlock *InsertAtEnd);
public:
    static StoreInst *Create(Value *Val, Value *Ptr,
                             Instruction *InsertBefore = nullptr);
    static StoreInst *Create(Value *Val, Value *Ptr,
                             BasicBlock *InsertAtEnd);
public:
    Value *getVal() const { return getOperand(0); }
    Value *getPtr() const { return getOperand(1); }

    static bool classof(Instruction *I) {
        return I->getOpcode() == Instruction::Store;
    }

    static bool classof(const Value *V) {
        return isa<Instruction>(V) && classof(cast<Instruction>(V));
    }
};


class LoadInst: public Instruction {

public:

    static bool classof(Instruction *I) {
        return I->getOpcode() == Instruction::Load;
    }

    static bool classof(const Value *V) {
        return isa<Instruction>(V) && classof(cast<Instruction>(V));
    }
};


class OffsetInst: public Instruction {
protected:
    OffsetInst(Type *PointeeTy,
               std::vector <Value *> &Ops,
               std::vector <std::optional<std::size_t>> &BoundList,
               Instruction *InsertBefore = nullptr);
    OffsetInst(Type *PointeeTy,
               std::vector <Value *> &Ops,
               std::vector <std::optional<std::size_t>> &BoundList,
               BasicBlock *InsertAtEnd);
private:
    Type *ElementTy;
    // Bounds are stored as explicit integer literal rather than operands, 
    // indices as uses of Value (inherited from class Instruction).
    // std::optional<std::uint32_t> is either 'nullopt' or a uint32_t value.
    // 'nullopt' stands for 'none' bound of the corresponding dimension.
    // See https://en.cppreference.com/w/cpp/utility/optional for more details.
    std::vector<std::optional<std::size_t>> Bounds;

    void AssertOK() const;
public:
    static OffsetInst *Create(Type *PointeeTy, Value *Ptr,
                              std::vector <Value *> &Indices,
                              std::vector <std::optional<std::size_t>> &Bounds,
                              Instruction *InsertBefore = nullptr);
    static OffsetInst *Create(Type *PointeeTy, Value *Ptr,
                              std::vector <Value *> &Indices,
                              std::vector <std::optional<std::size_t>> &Boundss,
                              BasicBlock *InsertAtEnd);

    using bound_iter = std::vector<std::optional<std::size_t>>::iterator;
    using const_bound_iter = std::vector<std::optional<std::size_t>>::const_iterator;


public:
    Type *getElementType() const { return ElementTy; }
    Value *getPointerOperand() const { return getOperand(0); }
    Type *getPointerOperandType() const { return getPointerOperand()->getType(); }

    std::vector<std::optional<std::size_t>> &bounds() { return Bounds; }
    const std::vector<std::optional<std::size_t>> &bounds() const { return Bounds; }

	/// Accumulate the constant address offset by unit of element type if possible.
    /// This routine accepts an size_t into which it will try to accumulate the constant offset.
    /// Examples:
    /// int g[3][4][5], &g[1][2][3], we have i32, %g.addr, [1 < 3], [2 < 4], [3 < 5]
    /// accumulateConstantOffset(0) gets 0 + (1 * 4 * 5) + (2 * 5) + (3 * 1s) 
    bool accumulateConstantOffset(std::size_t &Offset) const;

    static bool classof(Instruction *I) {
        return I->getOpcode() == Instruction::Offset;
    }

    static bool classof(const Value *V) {
        return isa<Instruction>(V) && classof(cast<Instruction>(V));
    }
};


class CallInst: public Instruction {

};


class RetInst: public Instruction {

};


class JumpInst: public Instruction {

};


class BranchInst: public Instruction {
protected:
    BranchInst(BasicBlock *IfTrue, BasicBlock *IfFalse, Value *Cond, Instruction *InsertBefore);
    BranchInst(BasicBlock *IfTrue, BasicBlock *IfFalse, Value *Cond, BasicBlock *InsertAtEnd);
private:
    BasicBlock *IfTrue; 
    BasicBlock *IfFalse;

    void AssertOK() const;
public:
    static BranchInst *Create(BasicBlock *IfTrue, BasicBlock *IfFalse, 
                              Value *Cond, 
                              Instruction *InsertBefore = nullptr);
    static BranchInst *Create(BasicBlock *IfTrue, BasicBlock *IfFalse, 
                              Value *Cond, 
                              BasicBlock *InsertAtEnd);

    Value *getCondition() const { return getOperand(0); }
    BasicBlock *getTrueBB() const { return IfTrue; }
    BasicBlock *getFalseBB() const { return IfFalse; }

};



/// Panic is a temporarily a placeholder terminator.
/// When a program encounters a 'PanicInst', it crashes.
/// You are NOT required to handle this instruction.
class PanicInst: public Instruction {

};


class BasicBlock: public ListNode<BasicBlock> {
public:
    BasicBlock(const BasicBlock &) = delete;
    BasicBlock &operator=(const BasicBlock &) = delete;
    ~BasicBlock();
public:
    using InstListType = List<Instruction>;
    using iterator = InstListType::iterator;
    using const_iterator = InstListType::const_iterator;
    using reverse_iterator = InstListType::reverse_iterator;
    using const_reverse_iterator = InstListType::const_reverse_iterator;

    friend class Instruction;

private:
    InstListType InstList;
    Function *Parent;

    InstListType &getInstList() { return InstList; }
    const InstListType &getInstList() const { return InstList; }

public:
    InstListType &getInstructions() { return InstList; }
    const Instruction *getTerminator() const {
        if (InstList.empty() || !InstList.back().isTerminator())
            return nullptr;
        return &InstList.back();
    }
    /// Returns the terminator instruction if the block is well formed or null
    /// if the block is not well formed.
    Instruction *getTerminator() {
        return const_cast<Instruction *>(
            static_cast<const BasicBlock *>(this)->getTerminator()
        );
    }

    iterator begin() { return InstList.begin(); }
    const_iterator cbegin() const { return InstList.cbegin(); }
    iterator end() { return InstList.end(); }
    const_iterator cend() const { return InstList.cend(); }
    reverse_iterator rbegin() { return InstList.rbegin(); }
    const_reverse_iterator crbegin() const { return InstList.crbegin(); }
    reverse_iterator rend() { return InstList.rend(); }
    const_reverse_iterator crend() const { return InstList.crend(); }
};