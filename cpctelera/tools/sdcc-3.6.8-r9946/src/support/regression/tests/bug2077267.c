/*
    bug 2077267
*/

#include <testfwk.h>

#ifndef __SDCC
#define __critical
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
