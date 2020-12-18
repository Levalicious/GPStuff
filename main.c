#include <stdio.h>
#include <stdlib.h>
#include "vm.h"
#include "rns.h"
#include <gmp.h>
#include <string.h>
#include <signal.h>
#include <time.h>

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

#define min(a, b) (((a) > (b)) ? (b) : (a))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define ceil(a, b) (((a) + (b) - 1LU) / (b))


#define STEPCNT (1024)
#define POP (30000)
#define INPCNT (9)
#define OPCNT (1)
#define GENCNT (500000000)
#define PRATE (1)
#define MUPROB (0.01)
#define PRGSIZE (4 * STEPCNT)
#define TRIALS (64)
#define PGENR (1)
#define TOURNSIZE (8)
#define ELITE (20)

uint64_t rstate[4];
uint64_t rstate2[4];

int32_t* fstate = (int32_t *) rstate;
int32_t* fstate2 = (int32_t *) rstate2;

float rf() {
    float res;
    fstate[0] *= 16807;
    *((uint32_t *) &res) = (((uint32_t) fstate[0]) >> 9U) | 0x3f800000;
    return (res - 1.f);
}

float rf2() {
    float res;
    fstate2[0] *= 16807;
    *((uint32_t *) &res) = (((uint32_t) fstate2[0]) >> 9U) | 0x3f800000;
    return (res - 1.f);
}


uint64_t r() {
    uint64_t *s = rstate;
    uint64_t const result = ((s[1] * 5 << 7) | (s[1] * 5 >> (64 - 7))) * 9;
    uint64_t const t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = (s[3] << 45) | (s[3] >> (64 - 45));

    return result;
}

uint64_t r2() {
    uint64_t *s = rstate2;
    uint64_t const result = ((s[1] * 5 << 7) | (s[1] * 5 >> (64 - 7))) * 9;
    uint64_t const t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = (s[3] << 45) | (s[3] >> (64 - 45));

    return result;
}

void fromgmp(rnum c, mpz_t a, mpz_t temp) {
    mpz_mod_ui(temp, a, 4294966813);
    c[0] = mpz_get_ui(temp);
    mpz_mod_ui(temp, a, 4294966877);
    c[1] = mpz_get_ui(temp);
    mpz_mod_ui(temp, a, 4294966927);
    c[2] = mpz_get_ui(temp);
    mpz_mod_ui(temp, a, 4294966981);
    c[3] = mpz_get_ui(temp);
    mpz_mod_ui(temp, a, 4294967029);
    c[4] = mpz_get_ui(temp);
    mpz_mod_ui(temp, a, 4294967111);
    c[5] = mpz_get_ui(temp);
    mpz_mod_ui(temp, a, 4294967161);
    c[6] = mpz_get_ui(temp);
    mpz_mod_ui(temp, a, 4294967197);
    c[7] = mpz_get_ui(temp);
    mpz_mod_ui(temp, a, 4294967279);
    c[8] = mpz_get_ui(temp);
}

void randomProg(prog* pr) {
    for (uint64_t i = 0; i < pr->csize; ++i) pr->code[i] = r();
    pr->lastScore = 1.7976931348623158e+308;
}

prog* initProg(uint64_t size) {
    prog* out = malloc(sizeof(prog));
    if (out == NULL) {
        printf("Failed to allocate program.\n");
        exit(-1);
    }
    out->csize = size;
    out->code = malloc(sizeof(uint8_t) * out->csize);
    if (out->code == NULL) {
        printf("Failed to allocate program code.\n");
        exit(-1);
    }
    out->lastScore = 1.7976931348623158e+308;

    return out;
}

double getFitness(uint64_t* mem, prog* prog, const uint64_t* i, uint64_t icnt, const uint64_t* o, uint64_t ocnt) {
    memcpy(mem, i, sizeof(uint64_t) * icnt);
    /* Set memory */

    run(prog, mem, STEPCNT);

    double err = 0;

    for (uint64_t x = 0; x < ocnt; ++x) {
        mem[255 - x] %= 289;
        uint64_t ma = max(o[x], mem[255 - x]);
        uint64_t mi = min(o[x], mem[255 - x]);
        uint64_t ri = (ma - mi);
        err += ri;
    }

    memset(mem, 0, sizeof(uint64_t) * 256);
    return err;
}

void mutProb(prog* prog) {
    for (uint64_t i = 0; i < prog->csize; ++i) {
        if (rf2() < MUPROB) prog->code[i] = r2();
    }
}

void copyProg(prog* c, prog* a) {
    memcpy(c->code, a->code, sizeof(uint8_t) * a->csize);
}

void scross(prog* c, prog* a, prog* b) {
    uint64_t xover, afirst;
    afirst = r2() & 1LU;
    xover = r2() % a->csize;

    uint64_t i;
    if (afirst) {
        for (i = 0; i < xover; ++i) c->code[i] = a->code[i];
        for (; i < a->csize; ++i) c->code[i] = b->code[i];
    } else {
        for (i = 0; i < xover; ++i) c->code[i] = b->code[i];
        for (; i < a->csize; ++i) c->code[i] = a->code[i];
    }
}

void seedr(uint64_t a) {
    rstate[0] = 0xb6d47cfacccc53f8LU ^ a;
    rstate[1] = 0x30b319a052624be7LU ^ a;
    rstate[2] = 0xfbeb173c6d0227d8LU ^ a;
    rstate[3] = 0x99cfe60a00bdd4feLU ^ a;

    rstate2[0] = r();
    rstate2[1] = r();
    rstate2[2] = r();
    rstate2[3] = r();
}

int main() {
    uint64_t seed = time(NULL);
    signal(SIGINT, intHandler);

    seedr(seed);

    prog ** p = malloc(sizeof(prog*) * POP);
    if (p == NULL) {
        printf("Failed to allocate parent array.\n");
        exit(-1);
    }
    for (uint64_t i = 0; i < POP; ++i) {
        p[i] = initProg(PRGSIZE);
        randomProg(p[i]);
    }

    prog** children = malloc(sizeof(prog*) * POP);
    if (children == NULL) {
        printf("Failed to allocate child array.\n");
        exit(-1);
    }
    for (uint64_t i = 0; i < POP; ++i) {
        children[i] = initProg(PRGSIZE);
    }

    uint64_t* iarr = malloc(sizeof(uint64_t) * INPCNT * TRIALS);
    if (iarr == NULL) {
        printf("Failed to allocate input array.\n");
        exit(-1);
    }
    uint64_t* oarr = malloc(sizeof(uint64_t) * OPCNT * TRIALS);
    if (oarr == NULL) {
        printf("Failed to allocate output array.\n");
        exit(-1);
    }

    mpz_t curr, curr2, curr3, curr4, maxMod;
    mpz_inits(curr, curr2, curr3, curr4, maxMod, NULL);

    char* modulus = "497322971361754518238941948902741570670397519085315904581334798510489434833404449209379";
    mpz_set_str(maxMod, modulus, 10);

    gmp_randstate_t gmpr;
    gmp_randinit_default(gmpr);
    gmp_randseed_ui(gmpr, r());

    uint64_t* mem = malloc(sizeof(uint64_t) * 256);
    if (mem == NULL) {
        printf("Failed to allocate memory array.\n");
        exit(-1);
    }
    memset(mem, 0, sizeof(uint64_t) * 256);

    printf("%lu\n", seed);
    printf("Setup complete.\n");

    prog* bestProg;

    prog** el = malloc(sizeof(prog*) * ELITE);
    if (el == NULL) {
        printf("Failed to allocate elite array.\n");
        exit(-1);
    }
    prog** tourn = malloc(sizeof(prog*) * TOURNSIZE);
    if (tourn == NULL) {
        printf("Failed to allocate tournament array.\n");
        exit(-1);
    }

    for (uint64_t t = 0; t < TRIALS; ++t) {
        uint64_t* currIn = iarr + (t * INPCNT);
        uint64_t* currOu = oarr + (t * OPCNT);

        mpz_urandomm(curr, gmpr, maxMod);
        mpz_ui_pow_ui(curr2, 2, (r()) % 289);
        mpz_mod(curr4, curr, curr2);

        fromgmp(currIn, curr4, curr3);

        (*currOu) = mpz_sizeinbase(curr4, 2);
    }

    for (uint64_t gen = 0; gen < GENCNT; ++gen) {
        if (!keepRunning) break;

        rstate2[r() % 4] ^= r(); /* Inject randomness into RNG */

        /*
        if (gen % PGENR == 0) {
            for (uint64_t t = 0; t < TRIALS; ++t) {
                uint64_t* currIn = iarr + (t * INPCNT);
                uint64_t* currOu = oarr + (t * OPCNT);

                mpz_urandomm(curr, gmpr, maxMod);
                mpz_ui_pow_ui(curr2, 2, (r()) % 289);
                mpz_mod(curr4, curr, curr2);

                fromgmp(currIn, curr4, curr3);

                (*currOu) = mpz_sizeinbase(curr4, 2);
            }
        }
         */

        /* Shuffle parents */
        /* No need to shuffle anymore if I'm picking randomly I guess? */
        /*
        for (uint64_t i = POP - 1; i > 0; --i) {
            uint64_t j = (r2() % (i + 1));
            prog* temp = p[j];
            p[j] = p[i];
            p[i] = temp;
        }
         */

        double totScore;
        totScore = 0;
        /* Run eval on every prog in parent array */
        bestProg = p[0];

        for (uint64_t z = 0; z < ELITE; ++z) {
            el[z] = p[z];
        }

        for (uint64_t currP = 0; currP < POP; ++currP) {
            p[currP]->lastScore = 0;
            for (uint64_t t = 0; t < TRIALS; ++t) {
                uint64_t* currIn = iarr + (t * INPCNT);
                uint64_t* currOu = oarr + (t * OPCNT);

                p[currP]->lastScore += getFitness(mem, p[currP], currIn, INPCNT, currOu, OPCNT);
            }

            p[currP]->lastScore /= TRIALS;
        }

        for (uint64_t currP = 0; currP < POP; ++currP) {
            totScore += p[currP]->lastScore;

            if (p[currP]->lastScore < bestProg->lastScore) {
                bestProg = p[currP];
            }

            for (uint64_t z = 0; z < ELITE; ++z) {
                if (p[currP]->lastScore < el[z]->lastScore) {
                    el[z] = p[currP];
                    break;
                }
            }
        }

        for (uint64_t ch = 0; ch < POP; ++ch) {
            /* Pick K random programs from parent pool */
            for (uint64_t tsel = 0; tsel < TOURNSIZE; ++tsel) {
                tourn[tsel] = p[r2() % POP];
            }

            /* Sort selections by score */
            for (uint64_t r = 0; r < TOURNSIZE - 1; ++r) {
                uint64_t minind = r;
                for (uint64_t rr = r + 1; rr < TOURNSIZE; ++rr) {
                    if (tourn[rr]->lastScore < tourn[minind]->lastScore) minind = rr;
                }
                prog* sw = tourn[minind];
                tourn[minind] = tourn[r];
                tourn[r] = sw;
            }

            /* Cross the two best programs */
            scross(children[ch], tourn[0], tourn[1]);

            /* Mutate the child */
            mutProb(children[ch]);
        }

        /* Copies best parent into child array */
        for (uint64_t r = 0; r < ELITE; ++r) {
            copyProg(children[r], el[r]);
        }

        /* Swaps parent array with child array */
        prog** progswitch = p;
        p = children;
        children = progswitch;

        /* Print progress */
        if (gen % PRATE == 0) printf(" %10lu : %25.2f %25.2f\n", gen, totScore / (POP), bestProg->lastScore);
    }

    /* Write best program to output file */
    FILE* out = fopen("../bestProg.gpr", "w");
    fwrite(bestProg->code, sizeof(uint8_t), bestProg->csize, out);
    fclose(out);

    mpz_clears(curr, curr2, curr3, curr4, maxMod, NULL);
    gmp_randclear(gmpr);

    for (uint64_t i = 0; i < POP; ++i) {
        free(p[i]->code);
        free(p[i]);
        free(children[i]->code);
        free(children[i]);
    }

    free(tourn);
    free(iarr);
    free(oarr);
    free(el);
    free(p);
    free(children);
    free(mem);

    printf("Seed: %lu\n", seed);
}


























