/*
   loop-3.c from the execute part of the gcc torture tests.
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
      g (i * INT_MAX / 2);
    }
  while (--i > 0);
}

void
testTortureExecute (void)
{
  f (4);
  if (n != 4)
    ASSERT (0);
  return;
}

