/*
   bug-3249.c. A check for the possibility of tail-call optimization in codegen assumed that code is generated for a function (as opposed to initialization of file-scope objects),
   resulting in a segfault when initializing file-scope static objects in non-intrinsic named address spaces from the Embedded C standard.
 */

#include <testfwk.h>

volatile char tmp;

void set_RAM_bank1(void) { tmp = 1; }

#if defined(__SDCC_mcs51)
#define DATA
#elif !defined(PORT_HOST)
__addressmod set_RAM_bank1 DATA;
#else
#define DATA
#endif

DATA int addendum1_ram = 2;

void
testBug (void)
{
	ASSERT (addendum1_ram == 2);
}

