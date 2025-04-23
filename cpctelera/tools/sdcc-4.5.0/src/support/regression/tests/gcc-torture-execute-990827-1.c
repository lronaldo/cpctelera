/*
   990827-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

unsigned test(unsigned one , unsigned  bit)
{
    unsigned val=  bit & 1;
    unsigned zero= one >> 1;

    val++;
    return zero + ( val>> 1 );
}

void
testTortureExecute (void)
{
  if (test (1,0) != 0)
    ASSERT (0);
  if (test (1,1) != 1)
    ASSERT (0);
  if (test (1,65535) != 1)
    ASSERT (0);
  return;

  return;
}

