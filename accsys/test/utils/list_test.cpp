#include "list_node.h"


#include "gtest/gtest.h"

TEST(UtilsTest, ListEmptyTest) {
    List<PayLoad> Ls;
    ASSERT_TRUE(Ls.empty());
    auto *Value1 = new PayLoad(1);
    Ls.insert(Ls.begin(), Value1);
    ASSERT_FALSE(Ls.empty());
    Ls.erase(Ls.begin());
    ASSERT_TRUE(Ls.empty());
}