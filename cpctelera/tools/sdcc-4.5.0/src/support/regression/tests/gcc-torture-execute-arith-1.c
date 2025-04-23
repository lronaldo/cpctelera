/*
   arith-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c89
#endif

unsigned
sat_add (unsigned i)
{
  unsigned ret = i + 1;
  if (ret < i)
    ret = i;
  return ret;
}

void
testTortureExecute (void)
{
  if (sat_add (~0U) != ~0U)
    ASSERT (0);
  return;
}
