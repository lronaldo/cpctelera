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
#if !defined (__SDCC_mcs51) && !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08)
unsigned long long Lx = 35;
#endif

void
testTortureExecute (void)
{
  unsigned char cy;
  unsigned short sy;
  unsigned int iy;
  unsigned long ly;
#if !defined (__SDCC_mcs51) && !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08)
  unsigned long long Ly;
#endif

  cy = cx / 6; ASSERT (cy == 1);
  cy = cx % 6; ASSERT (cy == 1);

  sy = sx / 6; ASSERT (sy == 2);
  sy = sx % 6; ASSERT (sy == 2);

  iy = ix / 6; ASSERT (iy == 3);
  iy = ix % 6; ASSERT (iy == 3);

  ly = lx / 6; ASSERT (ly == 4);
  ly = lx % 6; ASSERT (ly == 4);
#if !defined (__SDCC_mcs51) && !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08)
  Ly = Lx / 6; ASSERT (Ly == 5);
  Ly = Lx % 6; ASSERT (Ly == 5);
#endif
}
