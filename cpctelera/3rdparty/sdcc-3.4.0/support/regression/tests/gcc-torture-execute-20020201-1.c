/*
   20020201-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Test whether division by constant works properly.  */

unsigned char cx = 7;
unsigned short sx = 14;
unsigned int ix = 21;
unsigned long lx = 28;
#if 0
unsigned long long Lx = 35;
#endif

void
testTortureExecute (void)
{
  unsigned char cy;
  unsigned short sy;
  unsigned int iy;
  unsigned long ly;
#if 0
  unsigned long long Ly;
#endif

  cy = cx / 6; if (cy != 1) ASSERT (0);
  cy = cx % 6; if (cy != 1) ASSERT (0);

  sy = sx / 6; if (sy != 2) ASSERT (0);
  sy = sx % 6; if (sy != 2) ASSERT (0);

  iy = ix / 6; if (iy != 3) ASSERT (0);
  iy = ix % 6; if (iy != 3) ASSERT (0);

  ly = lx / 6; if (ly != 4) ASSERT (0);
  ly = lx % 6; if (ly != 4) ASSERT (0);
#if 0 // TODO: Enable when long long modulo is supported!
  Ly = Lx / 6; if (Ly != 5) ASSERT (0);
  Ly = Lx % 6; if (Ly != 5) ASSERT (0);
#endif
}

