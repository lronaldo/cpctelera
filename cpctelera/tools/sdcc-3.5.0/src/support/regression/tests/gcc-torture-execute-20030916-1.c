/*
   20030916-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* "i" overflows in f().  Check that x[i] is not treated as a giv.  */
#include <limits.h>

#if CHAR_BIT == 8

void f (unsigned int *x)
{
  unsigned char i;
  int j;

  i = 0x10;
  for (j = 0; j < 0x10; j++)
    {
      i += 0xe8;
      x[i] = 0;
      i -= 0xe7;
    }
}

void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51)
  unsigned int x[256];
  int i;

  for (i = 0; i < 256; i++)
    x[i] = 1;
  f (x);
  for (i = 0; i < 256; i++)
    if (x[i] != (i >= 0x08 && i < 0xf8))
      ASSERT (0);
  return;
#endif
}
#else
void
testTortureExecute (void) { return; }
#endif

