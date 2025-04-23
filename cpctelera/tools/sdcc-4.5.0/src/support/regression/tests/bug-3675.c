/* bug-3642.c
   A bug in handling of parameters to nested function calls (outer one explicit, inner one via helper function)
   that resulted in a register parameter being sent to the wrong function.
*/

#include <testfwk.h>

struct s
{
	unsigned int i;
};

// Need a function where the first paremeter is an integer parameter small enough to be passed in registers, followed by a struct parameter.
void f(unsigned int i, struct s s)
{
	ASSERT (i == 0x5a5a);
	ASSERT (s.i == 0xa5a5);
}

void
testBug (void)
{
#ifndef __SDCC_ds390 // Fails to link
	struct s s = {0xa5a5};

	f(0x5a5a, s); // When passing s, a memcpy call will be used as helper function. The 0x5a5a will be passed as if it was an additional parameter to that memcpy instead of being passed to f.
#endif
}

