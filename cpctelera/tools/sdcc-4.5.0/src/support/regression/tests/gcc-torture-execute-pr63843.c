/*
   pr60822.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>
#include <string.h>

/* PR rtl-optimization/63843 */

static inline
unsigned short foo (unsigned short v)
{
  return (v << 8) | (v >> 8);
}

unsigned short
bar (unsigned char *x)
{
  unsigned int a;
  unsigned short b;
  memcpy (&a, &x[0], sizeof (a));
  a ^= 0x80808080U;
  memcpy (&x[0], &a, sizeof (a));
  memcpy (&b, &x[2], sizeof (b));
  return foo (b);
}

void
testTortureExecute (void)
{
  unsigned char x[8] = { 0x01, 0x01, 0x01, 0x01 };
  if (CHAR_BIT == 8
      && sizeof (short) == 2
      && sizeof (int) == 4
      && bar (x) != 0x8181U)
    ASSERT (0);
  return;
}
