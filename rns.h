#ifndef RNS_RNS_H
#define RNS_RNS_H

#include "util.h"

typedef uint64_t rnum[9];

void rns_set(rnum a, uint64_t b);
void rns_copy(rnum c, const rnum a);
void rns_add(rnum c, const rnum a, const rnum b);
void rns_sub(rnum c, const rnum a, const rnum b);
void rns_mul(rnum c, const rnum a, const rnum b);
void rns_mul_u(rnum c, const rnum a, uint64_t b);
void rns_inv(rnum b, rnum a);

#endif
