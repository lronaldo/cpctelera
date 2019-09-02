/*
   931102-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

typedef union
{
  long align;
  struct
    {
      short h, l;
    } b;
} T;

int f (int x)
{
  int num = 0;
  T reg;

  reg.b.l = x;
  while ((reg.b.l & 1) == 0)
    {
      num++;
      reg.b.l >>= 1;
    }
  return num;
}

void
testTortureExecute (void)
{
  if (f (2) != 1)
    ASSERT (0);
  return;
}

