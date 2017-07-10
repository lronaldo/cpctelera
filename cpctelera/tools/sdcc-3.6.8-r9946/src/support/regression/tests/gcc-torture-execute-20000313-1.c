/*
   20000227-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

unsigned int buggy (unsigned int *param)
{
  unsigned int accu, zero = 0, borrow;
  accu    = - *param;
  borrow  = - (accu > zero);
  *param += accu;
  return borrow;
}

void
testTortureExecute (void)
{
  unsigned int param  = 1;
  unsigned int borrow = buggy (&param);

  if (param != 0)
    ASSERT (0);
  if (borrow + 1 != 0)
    ASSERT (0);
  return;
}

