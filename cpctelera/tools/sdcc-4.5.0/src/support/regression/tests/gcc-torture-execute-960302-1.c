/*
   960302-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

long a = 1;

#if !defined(__SDCC_pdk14) // Lack of memory - see RFE #364.
int
foo (void)
{
  switch (a % 2 % 2 % 2 % 2 % 2 % 2 % 2 % 2)
    {
    case 0:
      return 0;
    case 1:
      return 1;
    default:
      return -1;
    }
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) // Lack of memory - see RFE #364.
  ASSERT(foo () == 1);
  return;
#endif
}

