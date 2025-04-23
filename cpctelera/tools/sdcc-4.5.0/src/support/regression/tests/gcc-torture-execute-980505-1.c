/*
   980505-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static int f(int);

void
testTortureExecute (void)
{
   int f1, f2, x;
   x = 1; f1 = f(x);
   x = 2; f2 = f(x);
   if (f1 != 1 || f2 != 2)
     ASSERT (0);
   return;
}
static int f(int x) { return x; }

