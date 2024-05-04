#include "ir/type.h"

#include "gtest/gtest.h"


TEST(IRTest, PrimitiveTypeTest) {
    // Test the primitive type
    Type *IntegerType = Type::getIntegerTy();
    Type *UnitTy = Type::getUnitTy();

    ASSERT_EQ(IntegerType->getTypeID(), Type::IntegerTyID);
    ASSERT_EQ(IntegerType->isIntegerTy(), true);
    ASSERT_EQ(UnitTy->getTypeID(), Type::UnitTyID);
    ASSERT_EQ(UnitTy->isUnitTy(), true);
}

TEST(IRTest, PointerTypeTest) {
    // Test the pointer type
    Type *IntegerType = Type::getIntegerTy();
    // cast to Type * type.
    PointerType *PtrIntType = PointerType::get(IntegerType);
    ASSERT_EQ(PtrIntType->getElementType(), IntegerType);

    // up-casting
    Type *OpaqueType = PtrIntType;
    ASSERT_EQ(OpaqueType->isPointerTy(), true);
    ASSERT_EQ(OpaqueType->getTypeID(), Type::PointerTyID);
    ASSERT_EQ(isa<PointerType>(OpaqueType), true);
    ASSERT_EQ(dyn_cast<PointerType>(OpaqueType)->getElementType(), IntegerType);
}


TEST(IRTest, FunctionTypeTest) {
    // Test the function type
    Type *IntegerType = Type::getIntegerTy();
    Type *UnitTy = Type::getUnitTy();
    std::vector<Type *> Params = {IntegerType, UnitTy};
    FunctionType *FuncType = FunctionType::get(UnitTy, Params);

    ASSERT_EQ(FuncType->getNumParams(), 2);
    ASSERT_EQ(FuncType->getParamType(0), IntegerType);
    ASSERT_EQ(FuncType->getParamType(1), UnitTy);
    ASSERT_EQ(FuncType->getReturnType(), UnitTy);
    ASSERT_EQ(isa<FunctionType>(FuncType), true);
}