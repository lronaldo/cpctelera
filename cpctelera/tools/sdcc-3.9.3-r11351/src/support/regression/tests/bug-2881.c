/*
   bug-2881.c
   stack pointer adjustment code overwrote register parameter.
 */

#include <testfwk.h>

#if !defined(__SDCC_z80) && !defined(__SDCC_z180) && !defined(__SDCC_r2k) && !defined(__SDCC_r3ka) && !defined(__SDCC_tlcs90) && !defined(__SDCC_ez80_z80)
#define __z88dk_fastcall
#endif

void g(void *p)
{
    p;
}

int f(int test) __z88dk_fastcall
{
    char buffer[10];
    g(buffer);
    return test + test;
}

void
testBug(void)
{
    ASSERT(f(21) == 42);
}

