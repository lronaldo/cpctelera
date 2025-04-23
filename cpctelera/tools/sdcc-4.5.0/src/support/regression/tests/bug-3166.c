/*
   bug-3166.c
 */

#include <testfwk.h>

#define NDEBUG
#include <assert.h>
#include <stddef.h>

#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) // Lack of memory

enum { RULES_MAX = 4 };

typedef int* rule_seq;
typedef rule_seq* rule_alts;

int       rule_seq_lists[] = { 3, 1, 3, -1,
                               2, -1,
                               2, 1, -1 };
rule_seq  rule_alts_lists[] = { &rule_seq_lists[0], NULL,
                                &rule_seq_lists[4], &rule_seq_lists[6], NULL };
rule_alts rule[RULES_MAX] = { &rule_alts_lists[0],
                              &rule_alts_lists[2],
                              NULL,
                              NULL };
char      rule_terminal[RULES_MAX] = { 0, 0, 'a', 'b' };

enum { GOOD_POOL_MAX = 64,
       BAD_STACK_MAX  = 16 };

struct good {
   rule_seq seq;
   struct good* prev;
} good_pool[GOOD_POOL_MAX+1];  /* +1 to avoid incorrect SDCC warning */

struct bad {
   rule_alts    alts;
   char*        old_pos;
   struct good* old_good;
} bad_stack[BAD_STACK_MAX+1];  /* +1 to avoid incorrect SDCC warning */

int match(char* pos_arg)
{
    static char* pos;
    static struct bad*  bad_sp;
#ifdef AVOID_SCCZ80_BUG
    #define static
#endif
    static struct good* good_alloc;
    static struct good* good;
    static struct good* rgood;
    static rule_alts    alts;
#ifdef AVOID_SDCC_BUG
           rule_seq     seq;
#else
    static rule_seq     seq; /* this line breaks SDCC */
#endif
#ifdef AVOID_SCCZ80_BUG
    #undef static
#endif
    static char*        rpos;
    static int          ruleno;
    pos        = pos_arg;
    bad_sp     = bad_stack;
    good_alloc = good_pool;
    good       = NULL;
    rgood      = NULL;
    alts       = NULL;
    seq        = NULL;
    rpos       = NULL;
    ruleno     = 0;

    while (1) {
        assert(bad_sp < &bad_stack[BAD_STACK_MAX]);
        assert(good_alloc < &good_pool[GOOD_POOL_MAX]);
        assert(ruleno >= 0 && ruleno < RULES_MAX);
        alts = rule[ruleno];
        if (alts) {
            rgood = good;
            rpos = pos;
            goto first_alt;
        }
        assert(rule_terminal[ruleno]);        
        if (*pos != rule_terminal[ruleno])
            goto failure;
        ++pos;
    success:
        if (!good) {
            if (*pos != '\0')
                goto failure;
            return 1;
        }
        seq = good->seq;
        good = good->prev;
        ruleno = *seq++;
        if (*seq >= 0) {
            struct good* ngood = good_alloc++;
            ngood->seq = seq;
            ngood->prev = good;
            good = ngood;
        }
        continue;
    failure:
        if (bad_sp == bad_stack) {
            return 0;       
        }
        --bad_sp;
        alts = bad_sp->alts;
        rgood = bad_sp->old_good;
        rpos = bad_sp->old_pos;
    first_alt:
        good = good_alloc++;
        good->seq = *alts++;
        good->prev = rgood;
        pos = rpos;
        if (*alts) {
            bad_sp->alts = alts;
            bad_sp->old_good = rgood;
            bad_sp->old_pos  = pos;
            ++bad_sp;
        }
        goto success;
    }
}
#endif

void
testBug(void)
{
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) // Lack of memory
    ASSERT (match("baaab"));
#endif
}

