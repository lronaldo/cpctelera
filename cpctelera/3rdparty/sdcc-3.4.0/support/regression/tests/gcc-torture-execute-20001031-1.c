/*
   20001031-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports long long constants!
#if 0
void t1 (int x)
{
  if (x != 4100)
    abort ();
}

int t2 (void)
{
  int i;
  t1 ((i = 4096) + 4);
  return i;
}

void t3 (long long x)
{
  if (x != 0x80000fffULL)
    abort ();
}

long long t4 (void)
{
  long long i;
  t3 ((i = 4096) + 0x7fffffffULL);
  return i;
}
#endif

void
testTortureExecute (void)
{
#if 0
  if (t2 () != 4096)
    ASSERT (0);
  if (t4 () != 4096)
    ASSERT (0);
  return;
#endif
}

