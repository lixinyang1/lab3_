#pragma once

#include "ir/type.h"
#include "utils/list.h"
#include "utils/casting.h"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <type_traits>
#include <unordered_map>
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
    Use(Use &&) noexcept;

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
    ~Use() override {
        if (Val)
            removeFromList(Val);
    }
    /// make emplace_back happy.
    explicit Use(Value *Parent);
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

    [[nodiscard]] UserView getUserView() const { return UserView {.UserStart = UserIter}; }
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
#include "ir.def"

#define ConstantMarker(markname, constant) markname = constant##Val,
#include "ir.def"
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


/// Base class of all value binding and terminator instructions.
class Instruction: public Value, 
                   public ListNode<Instruction> {
public:
    Instruction(const Instruction &) = delete;
    Instruction &operator=(const Instruction &) = delete;

    // All instructions has two explicit static constructing methods,
    // specifying the instruction parameters, operands and the insertion position.
    // The first one is to insert the instruction before a specified instruction.
    // The second one is to insert the instruction at the end of a specified basic block.
    // Basically, the constructor of Instruction is private, and the only way to create
    // an instruction is to call the static method 'Create'.
    // The memory management is handled implicitly by intrusive lists.
    Instruction(Type *Ty, unsigned Opcode, 
                const std::vector<Value *> &Ops,
                Instruction *InsertBefore);
    Instruction(Type *Ty, unsigned Opcode, 
                const std::vector<Value *> &Ops,
                BasicBlock *InsertAtEnd);

    using InstListType = List<Instruction>;
    using op_iterator = typename std::vector<Use>::iterator;
    using const_op_iterator = typename std::vector<Use>::const_iterator;
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
    /// Operands iteration.
    op_iterator op_begin() { return Operands.begin(); }
    const_op_iterator op_begin() const { return Operands.cbegin(); }
    op_iterator op_end() { return Operands.end(); }
    const_op_iterator op_end() const { return Operands.cend(); }


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
#include "ir.def"     
    };

    enum MemoryOps {
#define   FirstMemoryInst(Num)                   MemoryOpsBegin = Num,
#define MemoryInstDefine( Num, Opcode, Subclass) Opcode = Num,
#define   LastMemoryInst( Num)                   MemoryOpsEnd = Num,
#include "ir.def"
    };

    enum TerminatorOps {
#define   FirstTerminator(Num)                   TerminatorOpsBegin = Num,
#define TerminatorDefine( Num, Opcode, Subclass) Opcode = Num,
#define   LastTerminator( Num)                   TerminatorOpsEnd = Num,
#include "ir.def"
    };

    enum OtherOps {
#define   FirstOtherInst(Num)                   OtherInstBegin = Num,
#define OtherInstDefine( Num, Opcode, Subclass) Opcode = Num,
#define   LastOtherInst( Num)                   OtherInstEnd = Num,
#include "ir.def"
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
#include "ir.def"
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
    // stored value.
    Value *getValue() const { return getOperand(0); }
    Type *getValueOperandType() const { return getValue()->getType(); }
    // stored destination pointer type value.
    Value *getPointer() const { return getOperand(1); }
    Type *getPointerOperandType() const { return getPointer()->getType(); }

    static bool classof(const Instruction *I) {
        return I->getOpcode() == Instruction::Store;
    }

    static bool classof(const Value *V) {
        return isa<Instruction>(V) && classof(cast<Instruction>(V));
    }
};


class LoadInst: public Instruction {
protected:
    LoadInst(Value *Ptr, Instruction *InsertBefore);
    LoadInst(Value *Ptr, BasicBlock *InsertAtEnd);
public:
    static LoadInst *Create(Value *Ptr, Instruction *InsertBefore = nullptr);
    static LoadInst *Create(Value *Ptr, BasicBlock *InsertAtEnd);

    Value *getPointerOperand() const { return getOperand(0); }
    Type *getPointerOperandType() const { return getPointerOperand()->getType(); }

    static bool classof(const Instruction *I) {
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

    static bool classof(const Instruction *I) {
        return I->getOpcode() == Instruction::Offset;
    }

    static bool classof(const Value *V) {
        return isa<Instruction>(V) && classof(cast<Instruction>(V));
    }
};


class CallInst: public Instruction {
protected:
    CallInst(Function *Callee, const std::vector<Value *> &Args, Instruction *InsertBefore);
    CallInst(Function *Callee, const std::vector<Value *> &Args, BasicBlock *InsertAtEnd);
private:
    Function *Callee;
public:
    static CallInst *Create(Function *Callee, const std::vector<Value *> &Args, 
                            Instruction *InsertBefore = nullptr);
    static CallInst *Create(Function *Callee, const std::vector<Value *> &Args, 
                            BasicBlock *InsertAtEnd);

    Function *getCallee() const { return Callee; }

    static bool classof(const Instruction *I) {
        return I->getOpcode() == Instruction::Call;
    }

    static bool classof(const Value *V) {
        return isa<Instruction>(V) && classof(cast<Instruction>(V));
    }
};


class RetInst: public Instruction {
protected:
    RetInst(Value *RetVal, Instruction *InsertBefore);
    RetInst(Value *RetVal, BasicBlock *InsertAtEnd);
public:
    static RetInst *Create(Value *RetVal, Instruction *InsertBefore = nullptr);
    static RetInst *Create(Value *RetVal, BasicBlock *InsertAtEnd);

    Value *getReturnValue() const { return getOperand(0); }

    static bool classof(const Instruction *I) {
        return I->getOpcode() == Instruction::Ret;
    }

    static bool classof(const Value *V) {
        return isa<Instruction>(V) && classof(cast<Instruction>(V));
    }
};


class JumpInst: public Instruction {
protected:
    JumpInst(BasicBlock *Dest, Instruction *InsertBefore);
    JumpInst(BasicBlock *Dest, BasicBlock *InsertAtEnd);
private:
    BasicBlock *Dest;
public:
    static JumpInst *Create(BasicBlock *Dest, Instruction *InsertBefore = nullptr);
    static JumpInst *Create(BasicBlock *Dest, BasicBlock *InsertAtEnd);

    BasicBlock *getDestBasicBlock() const { return Dest; }

    static bool classof(const Instruction *I) {
        return I->getOpcode() == Instruction::Jump;
    }

    static bool classof(const Value *V) {
        return isa<Instruction>(V) && classof(cast<Instruction>(V));
    }
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

    static bool classof(const Instruction *I) {
        return I->getOpcode() == Instruction::Br;
    }

    static bool classof(const Value *V) {
        return isa<Instruction>(V) && classof(cast<Instruction>(V));
    }
};



/// Panic is a temporarily a placeholder terminator.
/// When a program encounters a 'PanicInst', it crashes.
/// You are NOT required to handle this instruction.
class PanicInst: public Instruction {
protected:
    PanicInst(Instruction *InsertBefore);
    PanicInst(BasicBlock *InsertAtEnd);
public:
    static PanicInst *Create(Instruction *InsertBefore = nullptr);
    static PanicInst *Create(BasicBlock *InsertAtEnd);

    static bool classof(const Instruction *I) {
        return I->getOpcode() == Instruction::Panic;
    }

    static bool classof(const Value *V) {
        return isa<Instruction>(V) && classof(cast<Instruction>(V));
    }
};


class BasicBlock final: public ListNode<BasicBlock> {
public:
    BasicBlock(const BasicBlock &) = delete;
    BasicBlock &operator=(const BasicBlock &) = delete;
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
    std::string Name;

    InstListType &getInstList() { return InstList; }
    const InstListType &getInstList() const { return InstList; }

    BasicBlock(Function *Parent, BasicBlock *InsertBefore);
public:
    static BasicBlock *Create(Function *Parent = nullptr, BasicBlock *InsertBefore = nullptr);
    void insertInto(Function *Parent, BasicBlock *InsertBefore = nullptr);

    void setParent(Function *F);
    Function *getParent() const { return Parent; }
    bool hasName() const;
    void setName(std::string_view Name);
    
    /// Returns the terminator instruction if the block is well formed or null
    /// if the block is not well formed.
    const Instruction *getTerminator() const {
        if (InstList.empty() || !InstList.back().isTerminator())
            return nullptr;
        return &InstList.back();
    }
    Instruction *getTerminator() {
        return const_cast<Instruction *>(
            static_cast<const BasicBlock *>(this)->getTerminator()
        );
    }
    // container standard interface
    iterator begin() { return InstList.begin(); }
    const_iterator cbegin() const { return InstList.cbegin(); }
    iterator end() { return InstList.end(); }
    const_iterator cend() const { return InstList.cend(); }
    reverse_iterator rbegin() { return InstList.rbegin(); }
    const_reverse_iterator crbegin() const { return InstList.crbegin(); }
    reverse_iterator rend() { return InstList.rend(); }
    const_reverse_iterator crend() const { return InstList.crend(); }
    [[nodiscard]] std::size_t size() const { return InstList.size(); }
    [[nodiscard]] bool empty() const { return InstList.empty(); }
    [[nodiscard]] const Instruction &front() const { return InstList.front(); }
    [[nodiscard]] Instruction &front() { return InstList.front(); }
    [[nodiscard]] const Instruction &back() const { return InstList.back(); }
    [[nodiscard]] Instruction &back() { return InstList.back(); }
};


class Argument: public Value,
                public ListNode<Argument> {
protected:
    Argument(Type *Ty, Function *Parent = nullptr, unsigned ArgNo = 0);
private:
    Function *Parent;
    unsigned ArgNo;

    friend class Function;
public:
    const Function *getParent() const { return Parent; }
    Function *getParent() { return Parent; }
    unsigned getArgNo() const { return ArgNo; }

    static bool classof(const Value *V) {
        return V->getValueID() == Value::ArgumentVal;
    }
};


class Function final: public ListNode<Function> {
private:
    Function(FunctionType *FTy, bool ExternalLinkage, 
             std::string_view Name, Module *M);
    ~Function() final;
public:
    /// Basic blocks iteration.
    using BasicBlockListType = List<BasicBlock>;
    using iterator = BasicBlockListType::iterator;
    using const_iterator = BasicBlockListType::const_iterator;
    using reverse_iterator = BasicBlockListType::reverse_iterator;
    using const_reverse_iterator = BasicBlockListType::const_reverse_iterator;

    // Arguments iteration
    using arg_iterator = Argument *;
    using const_arg_iterator = const Argument *;
private:
    FunctionType *FTy;
    unsigned NumArgs = 0;
    Argument *Arguments = nullptr;
    bool ExternalLinkage = false;
    std::string Name;
    Module *Parent;
    BasicBlockListType BasicBlockList;

    friend class BasicBlock;

    // Private accessors to basic blocks and function arguments.
    BasicBlockListType &getBasicBlockList() { return BasicBlockList; }
    const BasicBlockListType &getBasicBlockList() const { return BasicBlockList; }
    
public:
    static Function *Create(FunctionType *FTy, bool ExternalLinkage = false,
                            std::string_view Name = "", Module *M = nullptr);
    // Insert BB in the basic block list at Position.
    Function::iterator insert(Function::iterator Position, BasicBlock *BB);
    
    std::string_view getName() const { return Name; }
    Module *getParent() const { return Parent; }
    /// Returns the FunctionType.
    FunctionType *getFunctionType() const {
        return cast<FunctionType>(FTy);
    }
    /// Returns the type of the ret val.
    Type *getReturnType() const { return FTy->getReturnType(); }
    std::size_t getNumParams() const { return NumArgs; }
    /// Return the function is defined in the current module or has external linkage.
    bool hasExternalLinkage() const { return ExternalLinkage; }

    /// Get the first basic block of the function.
    const BasicBlock &getEntryBlock() const { return BasicBlockList.front(); }
    BasicBlock &getEntryBlock() { return BasicBlockList.front(); }

    // Basic block iteration.
    iterator begin() { return BasicBlockList.begin(); }
    const_iterator cbegin() const { return BasicBlockList.cbegin(); }
    iterator end() { return BasicBlockList.end(); }
    const_iterator cend() const { return BasicBlockList.cend(); }
    reverse_iterator rbegin() { return BasicBlockList.rbegin(); }
    const_reverse_iterator crbegin() const { return BasicBlockList.crbegin(); }
    reverse_iterator rend() { return BasicBlockList.rend(); }
    const_reverse_iterator crend() const { return BasicBlockList.crend(); }
    // Basic blocks container method.
    [[nodiscard]] std::size_t size() const { return BasicBlockList.size(); }
    [[nodiscard]] bool empty() const { return BasicBlockList.empty(); }
    [[nodiscard]] const BasicBlock &front() const { return BasicBlockList.front(); }
    [[nodiscard]] BasicBlock &front() { return BasicBlockList.front(); }
    [[nodiscard]] const BasicBlock &back() const { return BasicBlockList.back(); }
    [[nodiscard]] BasicBlock &back() { return BasicBlockList.back(); }

    // Argument iteration.
    arg_iterator arg_begin() { return Arguments; }
    const_arg_iterator arg_cbegin() const { return Arguments; }
    arg_iterator arg_end() { return Arguments + NumArgs; }
    const_arg_iterator arg_cend() const { return Arguments + NumArgs; }
    Argument *getArg(unsigned index) const {
        assert (index < NumArgs && "getArg() out of range!");
        return Arguments + index;
    }
    // Arguments container method.
    [[nodiscard]] std::size_t arg_size() const { return NumArgs; }
    [[nodiscard]] bool arg_empty() const { return arg_size() == 0; }
};


class GlobalVariable: public Value,
                      public ListNode<GlobalVariable> {
protected:
    GlobalVariable(Type *EleTy, std::size_t NumElements, bool ExternalLinkage,
                   std::string_view Name, Module *M);
private:
    Type *EleTy;
    std::size_t NumElements;
    bool ExternalLinkage;
    std::string Name;
    Module *Parent;
public:
    static GlobalVariable *Create(Type *EleTy, std::size_t NumElements = 1, bool ExternalLinkage = false,
                                  std::string_view Name = "", Module *M = nullptr);
    /// Return the element type of the global variable.
    /// e.g. for '@a : region i32, 2', element type of '@a' is i32 while type of '@a' is i32*.
    Type *getElementType() const { return EleTy; }
    /// Return the region size of the global variable.
    std::size_t getNumElements() const { return NumElements; }
    /// Return the function is defined in the current module or has external linkage.
    bool hasExternalLinkage() const { return ExternalLinkage; }
    Module *getParent() const { return Parent; }
    std::string_view getName() const { return Name; }

};

class Module {
public:
    // Function iteration.
    using FunctionListType = List<Function>;
    using iterator = FunctionListType::iterator;
    using const_iterator = FunctionListType::const_iterator;
    using reverse_iterator = FunctionListType::reverse_iterator;
    using const_reverse_iterator = FunctionListType::const_reverse_iterator;

    // Global variable iteration.
    using GlobalListType = List<GlobalVariable>;
    using global_iterator = GlobalListType::iterator;
    using const_global_iterator = GlobalListType::const_iterator;

private:
    FunctionListType FunctionList;
    GlobalListType GlobalVariableList;
    std::unordered_map<std::string_view, Function *> SymbolFunctionMap;
    std::unordered_map<std::string_view, GlobalVariable *> SymbolGlobalMap;

    // Private functions and global variables accessors.
    FunctionListType &getFunctionList() { return FunctionList; }
    const FunctionListType &getFunctionList() const { return FunctionList; }
    GlobalListType &getGlobalList() { return GlobalVariableList; }
    const GlobalListType &getGlobalList() const { return GlobalVariableList; }

    friend class Function;
    friend class GlobalVariable;
public:
    /// Function accessor.
    /// Look up the specified function in the module symbol table.
    Function *getFunction(std::string_view Name) const;
    // Function iteration.
    iterator begin() { return FunctionList.begin(); }
    const_iterator cbegin() const { return FunctionList.cbegin(); }
    iterator end() { return FunctionList.end(); }
    const_iterator cend() const { return FunctionList.cend(); }
    reverse_iterator rbegin() { return FunctionList.rbegin(); }
    const_reverse_iterator crbegin() const { return FunctionList.crbegin(); }
    reverse_iterator rend() { return FunctionList.rend(); }
    const_reverse_iterator crend() const { return FunctionList.crend(); }

    [[nodiscard]] std::size_t size() const { return FunctionList.size(); }
    [[nodiscard]] bool empty() const { return FunctionList.empty(); }

    /// Global variable accessor.
    /// Look up the specified global variable in the module symbol table.
    GlobalVariable *getGlobalVariable(std::string_view Name) const;
    // Global iteration.
    global_iterator global_begin() { return GlobalVariableList.begin(); }
    const_global_iterator global_cbegin() const { return GlobalVariableList.cbegin(); }
    global_iterator global_end() { return GlobalVariableList.end(); }
    const_global_iterator global_cend() const { return GlobalVariableList.cend(); }

    [[nodiscard]] std::size_t global_size() const { return GlobalVariableList.size(); }
    [[nodiscard]] bool global_empty() const { return GlobalVariableList.empty(); }
};