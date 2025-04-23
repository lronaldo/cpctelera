/* bug-3717.c
   A bug in generalized constant propagation-based optimizations' handling of casts to float.
*/

#include <testfwk.h>

unsigned char foo(void) { return 0; } // Return type must be an unsigned char to reproduce bug
int func(float a) { return a; } // Argument type must be a float/double

void
testBug(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
    func(foo() + 0); // Summand literal must be 0 or a floating point (e.g. 1.0)
#endif
}

