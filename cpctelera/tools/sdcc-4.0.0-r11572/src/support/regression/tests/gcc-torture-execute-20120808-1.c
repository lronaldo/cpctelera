/*
   20120808-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

volatile int i;
unsigned char *volatile cp;
unsigned char d[32] = { 0 };

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  unsigned char c[32] = { 0 };
  unsigned char *p = d + i;
  int j;
  for (j = 0; j < 30; j++)
    {
      int x = 0xff;
      int y = *++p;
      switch (j)
	{
	case 1: x ^= 2; break;
	case 2: x ^= 4; break;
	case 25: x ^= 1; break;
	default: break;
	}
      c[j] = y | x;
      cp = p;
    }
  if (c[0] != 0xff
      || c[1] != 0xfd
      || c[2] != 0xfb
      || c[3] != 0xff
      || c[4] != 0xff
      || c[25] != 0xfe
      || cp != d + 30)
    ASSERT (0);
#endif
}
