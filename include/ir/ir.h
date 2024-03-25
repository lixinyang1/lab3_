#pragma once

#include "utils/list.h"

class Value {
};

class Instruction : public Value, public ListNode<Instruction> {

};


class BasicBlock {
private:
    List<Instruction> instrs;
};