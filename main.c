#include <stdio.h>
#include <stdlib.h>
#include "vm.h"
#include "rns.h"
#include <gmp.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

#define min(a, b) (((a) > (b)) ? (b) : (a))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define ceil(a, b) (((a) + (b) - 1LU) / (b))

// #define LOADNTEST


#define STEPCNT (256)
#define POP (1024)
#define INPCNT (18)
#define OPCNT (1)
#define GENCNT (5000000)
#define SRATE (0.7)
#define RRATE (0.3)
#define MUS (0.001)
#define PRATE (1)
#define MUPROB (0.004)
#define PRGSIZE (STEPCNT)
#define TRIALS (8192 * 8)
#define PGENR (GENCNT)
#define TOURNSIZE (4)
#define ELITE (1)
#define MURATE (0.001)
#define MINIMPR (0.0)

uint64_t gcd(uint64_t a, uint64_t b)
{
    if (a == 0) return b;
    if (b == 0) return a;

    uint64_t sw;

    int shift = __builtin_ctzl(a | b);
    a >>= __builtin_ctzl(a);

    do
    {
        b>>=__builtin_ctzl(b);

        if (a > b) {
            sw = a;
            a = b;
            b = sw;
        }
        b -= a;
    } while (b);

    return a << shift;
}

float rf(uint64_t* rstate) {
    int32_t* fstate = (int32_t *) rstate;
    float res;
    fstate[0] *= 16807;
    *((uint32_t *) &res) = (((uint32_t) fstate[0]) >> 9U) | 0x3f800000;
    return (res - 1.f);
}

uint64_t r(uint64_t* rstate) {
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

void randomProg(prog* pr, uint64_t* rstate) {
    for (uint64_t i = 0; i < pr->csize; ++i) pr->code[i] = r(rstate);
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

double getFitness(stack* stk, prog* prog, const uint64_t* i, uint64_t icnt, const uint64_t* o, uint64_t ocnt) {
    memcpy(stk->steak, i, sizeof(uint64_t) * icnt);
    stk->size = icnt;

    run(prog, stk, STEPCNT);

    double err = 0;

    /*
    mem[255] %= 4294966813;
    mem[254] %= 4294966877;
    mem[253] %= 4294966927;
    mem[252] %= 4294966981;
    mem[251] %= 4294967029;
    mem[250] %= 4294967111;
    mem[249] %= 4294967161;
    mem[248] %= 4294967197;
    mem[247] %= 4294967279;
     */

    for (uint64_t x = 0; x < ocnt; ++x) {
        if (stk->size == 0) {
            err += 16.0 * UINT64_MAX;
            continue;
        }
        uint64_t ta = pop(stk);
        ta &= 1LU;
        uint64_t ma = max(o[x], ta);
        uint64_t mi = min(o[x], ta);
        uint64_t ri = (ma - mi);
        err += ri;
    }

    stk->size = 0;

    return err;
}

void mutProb(prog* prog, double mu, uint64_t* rstate) {
    for (uint64_t i = 0; i < prog->csize; ++i) {
        if (rf(rstate) < mu) prog->code[i] = r(rstate);
    }
}

void mutSingleActive(prog* prog, uint64_t* rstate) {
    uint64_t muInd = r(rstate) % (prog->steps | 1LU);
    prog->code[muInd] = r(rstate);
}

void mutSingle(prog* prog, uint64_t* rstate) {
    uint64_t muInd = r(rstate) % prog->csize;
    prog->code[muInd] = r(rstate);
}

void loadProgram(prog* prog, char* path) {
    FILE* in = fopen(path, "r");
    fseek(in, 0L, SEEK_END);
    uint64_t savedProglen = ftell(in);
    rewind(in);
    size_t actLen = min(prog->csize, savedProglen);
    size_t res = fread(prog->code, sizeof(uint8_t), actLen, in);
    if (res != actLen) {
        printf("Read failed.\n");
        exit(-1);
    }

    fclose(in);
}

void copyProg(prog* c, prog* a) {
    memcpy(c->code, a->code, sizeof(uint8_t) * a->csize);
}

void scross(prog* c, prog* a, prog* b, uint64_t* rstate) {
    uint64_t xover, afirst;
    afirst = r(rstate) & 1LU;
    xover = r(rstate) % a->csize;

    uint64_t i;
    if (afirst) {
        memcpy(c->code, a->code, xover);
        memcpy(c->code + xover, b->code + xover,  (a->csize - xover));
    } else {
        memcpy(c->code, b->code, xover);
        memcpy(c->code + xover, a->code + xover,  (a->csize - xover));
    }
}

int main() {
    uint64_t* rstate = malloc(sizeof(uint64_t) * 4);
    uint64_t* rstate2 = malloc(sizeof(uint64_t) * 4);
    if (rstate == NULL || rstate2 == NULL) {
        printf("Failed to allocate RNGs\n");
        exit(-1);
    }
    uint64_t seed = time(NULL);

    rstate[0] = 0xb6d47cfacccc53f8LU ^ seed;
    rstate[1] = 0x30b319a052624be7LU ^ seed;
    rstate[2] = 0xfbeb173c6d0227d8LU ^ seed;
    rstate[3] = 0x99cfe60a00bdd4feLU ^ seed;

    rstate2[0] = r(rstate);
    rstate2[1] = r(rstate);
    rstate2[2] = r(rstate);
    rstate2[3] = r(rstate);

    stack* stk = malloc(sizeof(stack));
    if (stk == NULL) {
        printf("Failed to allocate stack.\n");
        exit(-1);
    }

    stk->steak = malloc(sizeof(uint64_t) * 512);
    if (stk->steak == NULL) {
        printf("Failed to allocate stack space.\n");
        exit(-1);
    }

#ifdef LOADNTEST
    prog* preeg = initProg(PRGSIZE);
    loadProgram(preeg, "../bestProg.gpr");
    uint64_t t4, t5;

    t4 = r(rstate2);
    t5 = r(rstate2);

    stk->stk[0] = t4;
    stk->stk[1] = t5;
    stk->size = 2;

    run(preeg, stk, STEPCNT);
    return 0;
#endif

    signal(SIGINT, intHandler);

    uint64_t survivors = POP * SRATE;

    prog ** p = malloc(sizeof(prog*) * POP);
    if (p == NULL) {
        printf("Failed to allocate parent array.\n");
        exit(-1);
    }
    for (uint64_t i = 0; i < POP; ++i) {
        p[i] = initProg(PRGSIZE);
        randomProg(p[i], rstate);
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
    memset(iarr, 0, sizeof(uint64_t) * INPCNT * TRIALS);

    uint64_t* oarr = malloc(sizeof(uint64_t) * OPCNT * TRIALS);
    if (oarr == NULL) {
        printf("Failed to allocate output array.\n");
        exit(-1);
    }
    memset(iarr, 0, sizeof(uint64_t) * OPCNT * TRIALS);

    mpz_t curr, curr2, curr3, curr4, maxMod;
    mpz_inits(curr, curr2, curr3, curr4, maxMod, NULL);

    char* modulus = "497322971361754518238941948902741570670397519085315904581334798510489434833404449209379";
    mpz_set_str(maxMod, modulus, 10);

    gmp_randstate_t gmpr;
    gmp_randinit_default(gmpr);
    gmp_randseed_ui(gmpr, r(rstate));

    double mu = MUPROB;

    double prevScore = 0.0;

    printf("%lu\n", seed);
    printf("Setup complete.\n");

    prog* bestProg = p[0];
    bestProg->lastScore = 99999999999.0;

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

    for (uint64_t gen = 0; gen < GENCNT; ++gen) {
        if (!keepRunning) break;

        rstate2[r(rstate) % 4] ^= r(rstate); /* Inject randomness into RNG */

        if (gen % PGENR == 0) {
            for (uint64_t t = 0; t < TRIALS; ++t) {
                uint64_t* currIn = iarr + (t * INPCNT);
                uint64_t* currOu = oarr + (t * OPCNT);

                /* ActualGCD
                uint64_t a = r(rstate);
                uint64_t b = r(rstate);
                uint64_t c = r(rstate);
                a &= UINT32_MAX;
                b &= UINT32_MAX;
                c &= UINT32_MAX;
                c %= a;
                c %= b;

                currIn[0] = a * c;
                currIn[1] = b * c;

                currOu[0] = gcd(currIn[0], currIn[1]);
                 */
                /* converter
                mpz_urandomm(curr, gmpr, maxMod);
                fromgmp(currIn, curr, curr3);

                memset(currOu, 0, sizeof(uint64_t) * OPCNT);
                mpz_export(currOu, NULL, -1, sizeof(uint64_t), 0, 0, curr);
                */

                /* modulus
                mpz_urandomm(curr, gmpr, maxMod);
                mpz_urandomm(curr2, gmpr, maxMod);

                fromgmp(currIn, curr, curr3); // store dividend
                fromgmp(currIn + 9, curr2, curr3); // store divisor

                mpz_mod(curr4, curr, curr2);

                fromgmp(currOu, curr4, curr); // store remainder
                 */

                /* comparison  */
                mpz_urandomm(curr, gmpr, maxMod);
                mpz_urandomm(curr2, gmpr, maxMod);

                int cmpRes = mpz_cmp(curr, curr2);
                (*currOu) = t & 1LU;
                cmpRes = (cmpRes > 0);
                if (cmpRes > 0 == (*currOu) > 0) {
                    fromgmp(currIn, curr, curr3);
                    fromgmp(currIn + 9, curr2, curr3);
                } else {
                    fromgmp(currIn, curr2, curr3);
                    fromgmp(currIn + 9, curr, curr3);
                }

                /* log2
                mpz_rrandomb(curr, gmpr, (t % 286) + 1);

                fromgmp(currIn, curr, curr3);

                (*currOu) = mpz_sizeinbase(curr, 2);
                */

                /* u256 add
                mpz_urandomb(curr, gmpr, 128);
                mpz_urandomb(curr2, gmpr, 128);

                mpz_add(curr3, curr, curr2);

                mpz_mod_2exp(curr4, curr3, 128);

                mpz_export(currIn, NULL, -1, sizeof(uint64_t), 0, 32, curr);
                mpz_export(currIn + OPCNT, NULL, -1, sizeof(uint64_t), 0, 32, curr2);

                mpz_export(currOu, NULL, -1, sizeof(uint64_t), 0, 32, curr4);
                 */
            }
        }

        double totScore;
        totScore = 0;
        bestProg = p[0];

        /* Fill elite array with valid parent programs (doesn't matter which ones) */
        for (uint64_t z = 0; z < ELITE; ++z) {
            el[z] = p[z];
        }

        /* Run fitness eval for each parent */
        for (uint64_t currP = 0; currP < POP; ++currP) {
            p[currP]->lastScore = 0;
            for (uint64_t t = 0; t < TRIALS; ++t) {
                uint64_t* currIn = iarr + (t * INPCNT);
                uint64_t* currOu = oarr + (t * OPCNT);

                p[currP]->lastScore += getFitness(stk, p[currP], currIn, INPCNT, currOu, OPCNT);
            }

            p[currP]->lastScore /= TRIALS;
        }

        /* Update total score, find best program, and set elite */
        uint32_t vcnt = 0;
        for (uint64_t currP = 0; currP < POP; ++currP) {
            if (p[currP]->lastScore < 2) {
                totScore += (p[currP]->lastScore);
                vcnt++;
            }

            if (p[currP]->lastScore < bestProg->lastScore) {
                bestProg = p[currP];
            }

            if (p[currP]->lastScore == bestProg->lastScore && p[currP]->steps < bestProg->steps) {
                bestProg = p[currP];
            }

            for (uint64_t z = 0; z < ELITE; ++z) {
                if (p[currP]->lastScore < el[z]->lastScore) {
                    el[z] = p[currP];
                    break;
                }
            }
        }

        /* Select and reproduce programs */
        for (uint64_t ch = 0; ch < POP; ++ch) {
            /* Pick K random programs from parent pool */
            for (uint64_t tsel = 0; tsel < TOURNSIZE; ++tsel) {
                tourn[tsel] = p[r(rstate2) % POP];
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

            if (ch < survivors) {
                copyProg(children[ch], tourn[0]);
            } else {
                /* Cross the two best programs */
                scross(children[ch], tourn[0], tourn[1], rstate2);
            }

            mutProb(children[ch], mu, rstate2);
        }

        /* Copy elite programs into child array */
        for (uint64_t rt = 0; rt < ELITE; ++rt) {
            copyProg(children[rt], el[rt]);
        }

        // copyProg(children[r(rstate2) % POP], bestProg);

        /* Swap parent array with child array */
        prog** progswitch = p;
        p = children;
        children = progswitch;

        /* Print progress */
        if (gen % PRATE == 0) printf(" %10lu : %28.6f %28.6f %10lu %28.10f\n", gen, 1.0 - (totScore / vcnt), 1.0 - bestProg->lastScore, bestProg->steps, mu);

        /* If there was no improvement, increase mutation rate
         * to escape this local optimum. If there was an improvment,
         * reset to original mutation rate */
        if (prevScore - MINIMPR <= bestProg->lastScore) {
            mu += (MURATE * mu);
        } else {
            mu += MUPROB * 3;
            mu *= 0.25;
        }
        /* If mutation rate is greater than 50%, reset to original rate */
        if (mu > 0.5) mu = MUPROB;
        prevScore = bestProg->lastScore;
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
    free(stk->steak);
    free(stk);
    free(rstate);
    free(rstate2);

    printf("Seed: %lu\n", seed);
}