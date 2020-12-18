#include "rns.h"

uint64_t moduli[9] = {
        4294966813,
        4294966877,
        4294966927,
        4294966981,
        4294967029,
        4294967111,
        4294967161,
        4294967197,
        4294967279
};

static inline uint64_t ulog2(uint64_t n) {
    return 63 - __builtin_clzll(((uint64_t) n) | 1LU);
}

void rns_set(rnum a, uint64_t b) {
    a[0] = b % moduli[0];
    a[1] = b % moduli[1];
    a[2] = b % moduli[2];
    a[3] = b % moduli[3];
    a[4] = b % moduli[4];
    a[5] = b % moduli[5];
    a[6] = b % moduli[6];
    a[7] = b % moduli[7];
    a[8] = b % moduli[8];
}

void rns_copy(rnum c, const rnum a) {
    c[0] = a[0];
    c[1] = a[1];
    c[2] = a[2];
    c[3] = a[3];
    c[4] = a[4];
    c[5] = a[5];
    c[6] = a[6];
    c[7] = a[7];
    c[8] = a[8];
}

void rns_add(rnum c, const rnum a, const rnum b) {
    c[0] = (a[0] + b[0]) % moduli[0];
    c[1] = (a[1] + b[1]) % moduli[1];
    c[2] = (a[2] + b[2]) % moduli[2];
    c[3] = (a[3] + b[3]) % moduli[3];
    c[4] = (a[4] + b[4]) % moduli[4];
    c[5] = (a[5] + b[5]) % moduli[5];
    c[6] = (a[6] + b[6]) % moduli[6];
    c[7] = (a[7] + b[7]) % moduli[7];
    c[8] = (a[8] + b[8]) % moduli[8];
}

void rns_sub(rnum c, const rnum a, const rnum b) {
    c[0] = (a[0] - b[0] + moduli[0]) % moduli[0];
    c[1] = (a[1] - b[1] + moduli[1]) % moduli[1];
    c[2] = (a[2] - b[2] + moduli[2]) % moduli[2];
    c[3] = (a[3] - b[3] + moduli[3]) % moduli[3];
    c[4] = (a[4] - b[4] + moduli[4]) % moduli[4];
    c[5] = (a[5] - b[5] + moduli[5]) % moduli[5];
    c[6] = (a[6] - b[6] + moduli[6]) % moduli[6];
    c[7] = (a[7] - b[7] + moduli[7]) % moduli[7];
    c[8] = (a[8] - b[8] + moduli[8]) % moduli[8];
}

void rns_mul(rnum c, const rnum a, const rnum b) {
    c[0] = (a[0] * b[0]) % moduli[0];
    c[1] = (a[1] * b[1]) % moduli[1];
    c[2] = (a[2] * b[2]) % moduli[2];
    c[3] = (a[3] * b[3]) % moduli[3];
    c[4] = (a[4] * b[4]) % moduli[4];
    c[5] = (a[5] * b[5]) % moduli[5];
    c[6] = (a[6] * b[6]) % moduli[6];
    c[7] = (a[7] * b[7]) % moduli[7];
    c[8] = (a[8] * b[8]) % moduli[8];
}

void rns_mul_u(rnum c, const rnum a, uint64_t b) {
    rnum rb;
    rns_set(rb, b);
    rns_mul(c, a, rb);
}

void rns_inv(rnum b, rnum a) {
    b[0] = nmodinv(a[0], moduli[0]);
    b[1] = nmodinv(a[1], moduli[1]);
    b[2] = nmodinv(a[2], moduli[2]);
    b[3] = nmodinv(a[3], moduli[3]);
    b[4] = nmodinv(a[4], moduli[4]);
    b[5] = nmodinv(a[5], moduli[5]);
    b[6] = nmodinv(a[6], moduli[6]);
    b[7] = nmodinv(a[7], moduli[7]);
}