#include "vm.h"

void run(prog* p, uint64_t* mem, uint64_t steps) {
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

    ptr = 0;
    for (i = 0; i < steps; ++i) {
        if (ptr > p->csize - 4) break;

        goto *instrTable[code(ptr) & 0b11111U];

        /* add */
        i0:
        reg(ptr + 1) = reg(ptr + 2) + reg(ptr + 3);
        ptr += 4;
        continue;

        /* sub */
        i1:
        reg(ptr + 1) = reg(ptr + 2) - reg(ptr + 3);
        ptr += 4;
        continue;

        /* mul */
        i2:
        reg(ptr + 1) = reg(ptr + 2) * reg(ptr + 3);
        ptr += 4;
        continue;

        /* div */
        i3:
        if (reg(ptr + 3)) reg(ptr + 1) = reg(ptr + 2) / reg(ptr + 3);
        ptr += 4;
        continue;

        /* mod */
        i4:
        if (reg(ptr + 3)) reg(ptr + 1) = reg(ptr + 2) % reg(ptr + 3);
        ptr += 4;
        continue;

        /* and */
        i5:
        reg(ptr + 1) = reg(ptr + 2) & reg(ptr + 3);
        ptr += 4;
        continue;

        /* or */
        i6:
        reg(ptr + 1) = reg(ptr + 2) | reg(ptr + 3);
        ptr += 4;
        continue;

        /* xor */
        i7:
        reg(ptr + 1) = reg(ptr + 2) ^ reg(ptr + 3);
        ptr += 4;
        continue;

        /* not */
        i8:
        reg(ptr + 1) = !reg(ptr + 2);
        ptr += 3;
        continue;

        /* flip */
        i9:
        reg(ptr + 1) = ~reg(ptr + 2);
        ptr += 3;
        continue;

        /* shl */
        i10:
        reg(ptr + 1) = reg(ptr + 2) << reg(ptr + 3);
        ptr += 4;
        continue;

        /* shr */
        i11:
        reg(ptr + 1) = reg(ptr + 2) >> reg(ptr + 3);
        ptr += 4;
        continue;

        /* minv */
        i12:
        /* TODO */
        ptr++;
        continue;

        /* gt */
        i13:
        reg(ptr + 1) = reg(ptr + 2) > reg(ptr + 3);
        ptr += 4;
        continue;

        /* lt */
        i14:
        reg(ptr + 1) = reg(ptr + 2) < reg(ptr + 3);
        ptr += 4;
        continue;

        /* gte */
        i15:
        reg(ptr + 1) = reg(ptr + 2) >= reg(ptr + 3);
        ptr += 4;
        continue;

        /* lte */
        i16:
        reg(ptr + 1) = reg(ptr + 2) <= reg(ptr + 3);
        ptr += 4;
        continue;

        /* eq */
        i17:
        reg(ptr + 1) = reg(ptr + 2) == reg(ptr + 3);
        ptr += 4;
        continue;

        /* ne */
        i18:
        reg(ptr + 1) = reg(ptr + 2) != reg(ptr + 3);
        ptr += 4;
        continue;

        /* mv */
        i19:
        ptr = reg(ptr + 1);
        ptr += 2;
        continue;

        /* mvr */
        i20:
        ptr += reg(ptr + 1);
        ptr += 2;
        continue;

        /* cpy */
        i21:
        reg(ptr + 1) = reg(ptr + 2);
        ptr += 3;
        continue;

        /* set 0 */
        i22:
        tval = code(ptr + 2);
        reg(ptr + 1) &= 0xFFFFFFFFFFFFFF00LU;
        reg(ptr + 1) |= tval;
        ptr += 3;
        continue;

        /* set 1 */
        i23:
        tval = code(ptr + 2);
        tval <<= 8LU;
        reg(ptr + 1) &= 0xFFFFFFFFFFFF00FFLU;
        reg(ptr + 1) |= tval;
        ptr += 3;
        continue;

        /* set 2 */
        i24:
        tval = code(ptr + 2);
        tval <<= 16U;
        reg(ptr + 1) &= 0xFFFFFFFFFF00FFFFLU;
        reg(ptr + 1) |= tval;
        ptr += 3;
        continue;

        /* set 3 */
        i25:
        tval = code(ptr + 2);
        tval <<= 24U;
        reg(ptr + 1) &= 0xFFFFFFFF00FFFFFFLU;
        reg(ptr + 1) |= tval;
        ptr += 3;
        continue;

        /* set 4 */
        i26:
        tval = code(ptr + 2);
        tval <<= 32U;
        reg(ptr + 1) &= 0xFFFFFF00FFFFFFFFLU;
        reg(ptr + 1) |= tval;
        ptr += 3;
        continue;

        /* set 5 */
        i27:
        tval = code(ptr + 2);
        tval <<= 40U;
        reg(ptr + 1) &= 0xFFFF00FFFFFFFFFFLU;
        reg(ptr + 1) |= tval;
        ptr += 3;
        continue;

        /* set 6 */
        i28:
        tval = code(ptr + 2);
        tval <<= 48U;
        reg(ptr + 1) &= 0xFF00FFFFFFFFFFFFLU;
        reg(ptr + 1) |= tval;
        ptr += 3;
        continue;

        /* set 7 */
        i29:
        tval = code(ptr + 2);
        tval <<= 56U;
        reg(ptr + 1) &= 0x00FFFFFFFFFFFFFFLU;
        reg(ptr + 1) |= tval;
        ptr += 3;
        continue;

        /* nop */
        i30:
        ptr++;
        continue;

        /* halt */
        i31:
        break;
    }
}