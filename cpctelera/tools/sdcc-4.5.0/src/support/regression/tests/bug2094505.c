/*
    bug 2094505
*/

#include <testfwk.h>

void
testBug(void)
{
#if !defined(__SDCC_mcs51) && !(defined(__SDCC_MODEL_LARGE) && !defined(__SDCC_STACK_AUTO)) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_pic14)  // Lack of memory
    // varA has to be declared for the bug to manifest itself
#if defined(__SDCC_STACK_AUTO)
    volatile char varA[64] = {0};
#else
    volatile char varA[256] = {0};
#endif
    volatile unsigned int varB = 0x1;
    volatile unsigned short varC = 0x2;

    // The Less Than comparison ASM for this while loop is generated incorrectly.
    ASSERT (varB < varC);
#endif
}
