#include "utils/list.h"


struct PayLoad: public ListNode<PayLoad> {
    int Value;
    PayLoad(int Value): Value(Value) {}
};