/*
    bug 2094505
*/

#include <testfwk.h>

void
testBug(void)
{
#if !defined(__SDCC) || !defined(__SDCC_mcs51) || defined(__SDCC_MODEL_LARGE)
    // varA has to be declared for the bug to manifest itself
    volatile char varA[256] = {0};
    volatile unsigned int varB = 0x1;
    volatile unsigned short varC = 0x2;

    // The Less Than comparison ASM for this while loop is generated incorrectly.
    ASSERT (varB < varC);
#endif
}
