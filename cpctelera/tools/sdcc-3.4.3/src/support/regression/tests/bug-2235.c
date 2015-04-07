/*      bug2235.c
*/

#include <testfwk.h>

#ifdef __SDCC_mcs51
__sfr32 __at (0xFCFBFAF9) SFR32;
#else
volatile unsigned long SFR32;
#endif

void foo(unsigned long u1, unsigned long u2, unsigned long u3)
{
        u1;
        u2;
        u3;
}

/* This failed to compile due to iTemps getting SFR storage class */
void testBug2235(void)
{
        unsigned long U1, U2, U3;

        U1 = 0xABCDEF01 + SFR32;
        U2 = U1 + SFR32;
        U3 = U2 + SFR32;
        foo(U1, U2, U3);
        ASSERT(1);
}
