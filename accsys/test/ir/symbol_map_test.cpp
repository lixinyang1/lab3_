#include "ir/ir.h"
#include "ir/type.h"

#include "gtest/gtest.h"


TEST(SymbolMapTest, GlobalVariableNameTest) {
    auto M = std::make_unique<Module>();
    Type *IntTy = Type::getIntegerTy();
    // nameless global variable
    auto GV = GlobalVariable::Create(IntTy, 100, false, "", M.get());
    ASSERT_EQ(M->getGlobalVariable("foo"), nullptr);
    ASSERT_EQ(GV->getName(), "");
    // set name "foo" by GlobalVariable.
    GV->setName("foo");
    ASSERT_EQ(M->getGlobalVariable("foo"), GV);
    ASSERT_EQ(GV->getName(), "foo");
    // set name "bar" by Value
    cast<Value>(GV)->setName("bar");
    ASSERT_EQ(GV->getName(), "bar");
    ASSERT_EQ(M->getGlobalVariable("foo"), nullptr);
    ASSERT_EQ(M->getGlobalVariable("bar"), GV);
}