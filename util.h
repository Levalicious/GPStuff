#ifndef RNS_UTIL_H
#define RNS_UTIL_H

#include <stdint.h>

static inline uint64_t nmodinv(uint64_t a, uint64_t n) {
    if (n == 0) return a;
    int64_t x1 = 1, y1 = 0;
    uint64_t a1 = a;
    int64_t x0 = 0, y0 = 1;
    uint64_t a2 = n, q = 0;
    while (a2 != 0) {
        int64_t x2 = x0 - q * x1;
        int64_t y2 = y0 - q * y1;
        x0 = x1; y0 = y1;
        uint64_t a0 = a1;
        x1 = x2; y1 = y2; a1 = a2;
        q = a0 / a1;
        a2 = a0 - q * a1;
    }
    x1 *= (a1 == 1);
    int64_t ni = ((int64_t) n);
    x1 = (x1 % ni + ni) % ni;
    return x1;
}

#endif
