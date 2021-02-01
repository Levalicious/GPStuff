#include "vm.h"
#include <stdio.h>
#include <string.h>

void push(stack* stk, uint64_t val) {
    if (stk->size >= 512) return;
    stk->steak[stk->size] = val;
    stk->size++;
}

uint64_t pop(stack* stk) {
    stk->size--;
    return stk->steak[stk->size];
}

void swap(stack* stk, uint64_t ind) {
    ind %= stk->size;
    uint64_t t = stk->steak[stk->size - 1];
    stk->steak[stk->size - 1] = stk->steak[ind];
    stk->steak[ind] = t;
}

void yank(stack* stk, uint64_t ind) {
    ind %= stk->size;
    uint64_t* a = &(stk->steak[ind]);
    uint64_t* b = &(stk->steak[ind + 1]);
    uint64_t t = *a;
    memmove(a, b, sizeof(uint64_t) * (stk->size - ind));
    stk->steak[stk->size - 1] = t;
}

void shove(stack* stk, uint64_t ind) {
    ind %= stk->size;
    uint64_t* a = &(stk->steak[ind]);
    uint64_t* b = &(stk->steak[ind + 1]);
    uint64_t t = stk->steak[stk->size - 1];
    memmove(b, a, sizeof(uint64_t) * (stk->size - 1 - ind));
    *a = t;
}

void run(prog* p, stack* stk, uint64_t steps) {
    uint64_t i, tval;
    void* instrTable[CNT] = {
            &&i0,
            &&i1,
            &&i2,
            &&i3,
            &&i4,
            &&i5,
            &&i6,
            &&i7,
            &&i8,
            &&i9,
            &&i10,
            &&i11,
            &&i12,
            &&i13,
            &&i14,
            &&i15,
            &&i16,
            &&i17,
            &&i18,
            &&i19,
            &&i20,
            &&i21,
            &&i22,
            &&i23,
            &&i24,
            &&i25,
            &&i26,
            &&i27,
            &&i28,
            &&i29,
            &&i30,
            &&i31,
    };
#define ptr (p->iptr)
#define reg(x) (mem[p->code[x]])
#define code(x) (p->code[x])
// #define DEBUG

    ptr = 0;
    for (i = 0; i < steps; ++i) {
        if (ptr > p->csize - 9) break;

        goto *instrTable[code(ptr) & 0b11111U];

        /* add */
        i0:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a + b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* sub */
        i1:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a - b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* mul */
        i2:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a * b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* div */
        i3:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            if (b) {
                uint64_t c = a / b;
                push(stk, c);
            }
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* mod */
        i4:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            if (b) {
                uint64_t c = a % b;
                push(stk, c);
            }
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* and */
        i5:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a & b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* or */
        i6:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a | b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* xor */
        i7:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a ^ b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* not */
        i8:
        if (stk->size >= 1) {
            uint64_t a = pop(stk);
            uint64_t b = !a;
            push(stk, b);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* flip */
        i9:
        if (stk->size >= 1) {
            uint64_t a = pop(stk);
            uint64_t b = ~a;
            push(stk, b);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* shl */
        i10:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a << b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* shr */
        i11:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a >> b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* log2 */
        i12:
        if (stk->size >= 1) {
            uint64_t a = pop(stk);
            uint64_t b = 63LU - __builtin_clzll((a) | 1LU);
            push(stk, b);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* gt */
        i13:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a > b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* lt */
        i14:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a < b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* gte */
        i15:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a >= b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* lte */
        i16:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a <= b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* eq */
        i17:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a == b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* ne */
        i18:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            uint64_t c = a != b;
            push(stk, c);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* mv */
        i19:
        if (stk->size >= 1) {
            uint64_t a = pop(stk);
            ptr = a;
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* bze */
        i20:
        if (stk->size >= 2) {
            uint64_t a = pop(stk);
            uint64_t b = pop(stk);
            if (!a) {
                ptr = b;
            } else {
                ptr++;
            }
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        continue;

        /* dup */
        i21:
        if (stk->size >= 1) {
            uint64_t a = pop(stk);
            push(stk, a);
            push(stk, a);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* pop */
        i22:
        if (stk->size >= 1) {
            pop(stk);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* swap */
        i23:
        if (stk->size >= 3) {
            uint64_t a = pop(stk);
            swap(stk, a);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* yank */
        i24:
        if (stk->size >= 3) {
            uint64_t a = pop(stk);
            yank(stk, a);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* shove */
        i25:
        if (stk->size >= 3) {
            uint64_t a = pop(stk);
            shove(stk, a);
#ifdef DEBUG
            printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        }
        ptr++;
        continue;

        /* set 4 */
        i26:
        ptr++;
        continue;

        /* set 5 */
        i27:
        ptr++;
        continue;

        /* set 6 */
        i28:
        ptr++;
        continue;

        /* set 7 */
        i29:
        ptr++;
        continue;

        /* set */
        i30:
        push(stk, *((uint64_t*) (&(code(ptr + 1)))));
#ifdef DEBUG
        printf("%10s\n", instrNames[code(ptr) & 0b11111U]);
#endif
        ptr += 9;
        continue;

        /* halt */
        i31:
#ifdef DEBUG
        printf("%10s : halt\n");
#endif
        break;
    }
    p->steps = i;
}