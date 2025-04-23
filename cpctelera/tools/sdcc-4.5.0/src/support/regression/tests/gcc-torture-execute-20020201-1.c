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
unsigned long long Lx = 35;

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  unsigned char cy;
  unsigned short sy;
  unsigned int iy;
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
  unsigned long ly;
  unsigned long long Ly;
#endif

  cy = cx / 6; ASSERT (cy == 1);
  cy = cx % 6; ASSERT (cy == 1);

  sy = sx / 6; ASSERT (sy == 2);
  sy = sx % 6; ASSERT (sy == 2);

  iy = ix / 6; ASSERT (iy == 3);
  iy = ix % 6; ASSERT (iy == 3);
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
  ly = lx / 6; ASSERT (ly == 4);
  ly = lx % 6; ASSERT (ly == 4);

  Ly = Lx / 6; ASSERT (Ly == 5);
  Ly = Lx % 6; ASSERT (Ly == 5);
#endif
#endif
}
