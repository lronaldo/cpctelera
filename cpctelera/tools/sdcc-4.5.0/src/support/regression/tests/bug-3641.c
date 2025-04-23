/* bug-3641.c
   The hc08/s08 backend omitted the least-significant byte in an assignment of a literal to a local 24-bit variable, depending on which registers it resided in.
*/

#include <testfwk.h>

#include <stdint.h>

#if __SDCC_BITINT_MAXWIDTH >= 32 // TODO: When we can regression-test in --std-c23 mode, use the standard macro from limits.h instead!
// In both assignments to i, the least-significant byte gets omitted.
void loopm2(unsigned char *a)
{
    for (unsigned _BitInt(24) i = (1ul << 20); i < (1ul << 20) + 1; i = (1ul << 20) + 1)
        a[i - (1ul << 20)] = 1;
}
#endif

void
testBug(void)
{
#if __SDCC_BITINT_MAXWIDTH >= 32 // TODO: When we can regression-test in --std-c23 mode, use the standard macro from limits.h instead!
	char c = 0;
	loopm2(&c);
#ifndef __SDCC_pdk15 // Bug #3643.
	ASSERT(c == 1);
#endif
#endif
}

