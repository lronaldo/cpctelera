/*
   20020225-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static int 
ftest(int x)
{
  union 
    {
      int i;
      double d;
  } a;
  a.d = 0;
  a.i = 1;
  return x >> a.i;
}

void testTortureExecute (void)
{
  if (ftest (5) != 2)
    ASSERT (0);
  return;
}

