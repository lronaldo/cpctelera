/*
   980716-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdarg.h>

void
stub(int num, ...)
{
    va_list ap;
    char *end;
    int i;

    for (i = 0; i < 2; i++) {
        va_start(ap, num);
        while ( 1 ) {
            end = va_arg(ap, char *);
            if (!end) break;
        }
        va_end(ap);
    }
}

void
testTortureExecute (void)
{
    stub(1, "ab", "bc", "cx", 0);
    return;
}

