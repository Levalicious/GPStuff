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
    log2instr,
    gt,
    lt,
    gte,
    lte,
    eq,
    ne,
    mv,
    bze,
    dupp,
    popp,
    swapp,
    yankk,
    shovee,
    set4,
    set5,
    set6,
    set7,
    set,
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
        "log2",
        "gt",
        "lt",
        "gte",
        "lte",
        "eq",
        "ne",
        "mv",
        "bze",
        "dup",
        "pop",
        "swap",
        "set2",
        "set3",
        "set4",
        "set5",
        "set6",
        "set7",
        "set",
        "halt",
};

typedef struct {
    uint64_t* steak;
    uint64_t size;
} stack;

typedef struct {
    uint8_t* code;
    uint64_t csize;

    uint64_t iptr;

    double lastScore;
    uint64_t steps;
} prog;


void push(stack* stk, uint64_t val);
uint64_t pop(stack* stk);

void run(prog* p, stack* stk, uint64_t steps);

#endif
