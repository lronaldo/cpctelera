/*
   loop-3b.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

#include <limits.h>

int n = 0;

void g (int i)
{
  n++;
}

void f (int m)
{
  int i;
  i = m;
  do
    {
      g (i * 4);
      i -= INT_MAX / 8;
    }
  while (i > 0);
}

void
testTortureExecute (void)
{
  f (INT_MAX/8*4);
  if (n != 4)
    ASSERT (0);
  return;
}
