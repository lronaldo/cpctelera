/*
   bug-3132.c
   A bug in the handling of inlined function call in a comma operator inside a for condition (code failed to compile)
 */

#include <testfwk.h>

volatile int i;

static inline void iRestore(void)
{
	i++;
}

int m(void) {
    for(unsigned char td = 1; td; td = 0, iRestore());
    return 0;
}

void testBug (void)
{
	m();
	ASSERT (i == 1);
}

