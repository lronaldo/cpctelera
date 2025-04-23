/* bug-3459.c
   A bug in CSE resulted in a used temporary holding the address of a struct parameter being optimized out during loop optimizations.
   This was visible as a crash during code generation.
   The testcase is manually simplified code originally created by csmith.
 */

#include <testfwk.h>

#pragma disable_warning 85
#pragma disable_warning 94
#pragma disable_warning 158

#include <stdint.h>

#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) // Lack of memory
#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk15) && !defined(__SDCC_ds390) // Remaining aspects of bug
struct S0 {
   signed f0 : 3;
   signed f1 : 12;
};

static int32_t g_2 = 0x09C3FBB4L;
static int32_t g_22 = 0xD9E62B56L;
static uint64_t g_33 = 1UL;
static volatile int32_t g_38 = (-9L);
static volatile int32_t *g_37[1] = {&g_38};
static volatile int32_t **g_36 = &g_37[0];
static volatile int32_t *** volatile g_39[2] = {&g_36,&g_36};
static volatile uint8_t g_53 = 0xA6L;

static uint64_t func_1(void);
int32_t func_5(const int64_t p_6, int32_t p_7, uint32_t p_8, struct S0 p_9, int8_t p_10);

static uint64_t func_1(void)
{
    const int8_t l_11 = (-2L);
    int32_t l_41 = 0x3EE1E161L;
    int32_t l_52[2];
    int i;
    for (i = 0; i < 2; i++)
        l_52[i] = 0x33B75CABL;
    for (g_2 = 0; (g_2 != (-23)); g_2--)
    {
        int32_t l_12 = 0x58EDFE01L;
        struct S0 l_13 = {1,18};
        int32_t *l_42 = &g_22;
        if (func_5(l_11, g_2, l_12, l_13, g_2))
        {
            int32_t *l_31[2][1];
            int i, j;
            for (i = 0; i < 2; i++)
            {
                for (j = 0; j < 1; j++)
                    l_31[i][j] = &g_22;
            }
            --g_33;
        }
        else
        {
            volatile int32_t ***l_40 = &g_36;
            (*l_40) = g_36;
            (**g_36) = (**g_36);
        }
        (*l_42) ^= (((l_41 |= (l_11 >= 0xB0L)) != l_11) ^ g_38);
    }

    return l_52[0];
}

int32_t func_5(const int64_t p_6, int32_t p_7, uint32_t p_8, struct S0 p_9, int8_t p_10)
{
    return 0;
}
#endif
#endif

void
testBug (void)
{
#if !defined( __SDCC_pdk13) && !defined( __SDCC_pdk14) // Lack of memory
#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk15) && !defined(__SDCC_ds390) // Remaining aspects of bug
    func_1();
#endif
#endif
}

