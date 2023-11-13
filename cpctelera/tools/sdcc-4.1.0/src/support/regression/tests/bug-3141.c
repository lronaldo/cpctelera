/* bug 3141
   a bug in code generation for stm8 large meory model (24-bit function pointers),
   when returning function pointers fromk a function with a single 16-bit parameter while optimizing for code size.
 */
#include <testfwk.h>

#pragma opt_code_size

void f(void)
{
}

typedef void (*p) (void);

p g(int b)
{
	return (b ? &f : (p)0);
}

void testBug(void)
{
#ifndef __SDCC_mcs51 // Bug #3146
	ASSERT (g(0) == 0);
	ASSERT (g(1) == &f);
	#endif
}

