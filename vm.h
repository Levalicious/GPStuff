#ifndef RNS2_VM_H
#define RNS2_VM_H

#include <stdint.h>

typedef enum {
    add = 0,
    sub = 1,
    mul,
    rdiv,
    mod,
    and,
    or,
    xor,
    not,
    flip,
    shl,
    shr,
    minv, /* CURRENTLY A NOP */
    gt,
    lt,
    gte,
    lte,
    eq,
    ne,
    mv,
    mvr,
    cpy,
    set0,
    set1,
    set2,
    set3,
    set4,
    set5,
    set6,
    set7,
    nop,
    halt,
    CNT
} instr;

static char* instrNames[CNT] = {
        "add",
        "sub",
        "mul",
        "div",
        "mod",
        "and",
        "or",
        "xor",
        "not",
        "flip",
        "shl",
        "shr",
        "minv",
        "gt",
        "lt",
        "gte",
        "lte",
        "eq",
        "ne",
        "mv",
        "mvr",
        "cpy",
        "set0",
        "set1",
        "set2",
        "set3",
        "set4",
        "set5",
        "set6",
        "set7",
        "nop",
        "halt",
};

typedef struct {
    uint8_t* code;
    uint64_t csize;

    uint64_t iptr;

    double lastScore;
    uint64_t steps;
} prog;

void run(prog* p, uint64_t* mem, uint64_t steps);

#endif
