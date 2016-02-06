/*
   20050224-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Origin: Mikael Pettersson <mikpe@csd.uu.se> and the Linux kernel.  */

unsigned long a = 0xc0000000, b = 0xc0400000;
unsigned long c = 0xc01bb958, d = 0xc0264000;
unsigned long e = 0xc0288000, f = 0xc02d4378;

void
foo (int x, int y, int z)
{
  ASSERT (!(x != 245 || y != 36 || z != 444));
}

void
testTortureExecute (void)
{
  unsigned long g;
  int h = 0, i = 0, j = 0;

  ASSERT (!(sizeof (unsigned long) < 4));

  for (g = a; g < b; g += 0x1000)
    if (g < c)
      h++;
    else if (g >= d && g < e)
      j++;
    else if (g < f)
      i++;
  foo (i, j, h);
}
