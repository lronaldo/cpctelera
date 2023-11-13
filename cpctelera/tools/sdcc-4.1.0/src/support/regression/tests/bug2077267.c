/*
    bug 2077267
*/

#include <testfwk.h>

#ifndef __SDCC
#define __critical
#endif
#if defined(__SDCC_pdk14) || defined(__SDCC_pdk15)
#define __critical // __critical not implemented for pdk14
#endif

void bug(char* x)
{
    *x = *x + 1;
}

void
testBug(void)
{
    char x = 1;

    bug(&x);

    __critical {
        bug(&x);
    }

    ASSERT (x == 3);
}
