/*
   930518-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int bar = 0;

f (int *p)
{
  int foo = 2;

  while (foo > bar)
    {
      foo -=  bar;
      *p++ = foo;
      bar = 1;
    }
}

void
testTortureExecute (void)
{
  int tab[2];
  tab[0] = tab[1] = 0;
  f (tab);
  if (tab[0] != 2 || tab[1] != 1)
    ASSERT (0);
  return;
}

