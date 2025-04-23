/*
   20000314-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

typedef unsigned long long uint64;
const uint64 bigconst = 1ULL << 34;

int a = 1;

static
uint64 getmask(void)
{
    if (a)
      return bigconst;
    else
      return 0;
}

void
testTortureExecute (void)
{
    uint64 f = getmask();
    if (sizeof (long long) == 8
	&& f != bigconst) ASSERT (0);
    return;
}

