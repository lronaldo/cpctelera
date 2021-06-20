/*
   20001112-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void)
{
// Some ports do not support long long yet.
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  long long i = 1;

  i = i * 2 + 1;
  
  if (i != 3)
    ASSERT (0);
  return;
#endif
}

