/* bug-3081.c
   z80 backend inserted extra whitespace in call via __z88dk_fastcall function pointer 
   for --reserve-regs-iy, confusing peephole optimizer
 */


#include <testfwk.h>

#pragma disable_warning 283

#include <stdint.h>

#if defined(__SDCC_mcs51) || defined(__SDCC_pdk14) || defined(__SDCC_pdk15)
#ifdef __z88dk_fastcall
#undef __z88dk_fastcall
#endif
#define __z88dk_fastcall __reentrant
#elif !(defined(__SDCC_z80) || defined(__SDCC_z180))
#define __z88dk_fastcall
#endif

short example(short x) __z88dk_fastcall
{
    return x+1;
}

short (*ifunc)(short arg) __z88dk_fastcall;

int m()
{
    int i, sum;
    ifunc = example;
    sum = 0;
    for (i = 0; i < 42; ++i)
	sum += (ifunc)(i);
    return sum;
}

void
testBug(void)
{
}

