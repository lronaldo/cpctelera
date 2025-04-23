/*
20100209-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

int bar(int foo)
{
  return (int)(((unsigned long long)(long long)foo) / 8);
}

void
testTortureExecute (void)
{
  if (sizeof (long long) > sizeof (int)
      && bar(-1) != -1)
    ASSERT (0);
  return;
}
