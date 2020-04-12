/*
   pr78856.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int a, b, c, d, e, f[3]; 

void
testTortureExecute (void)
{
  while (d)
    while (1)
      ;
#if 0 // Enable when SDCC intermingles
  int g = 0, h, i = 0;
  for (; g < 21; g += 9) 
    {
      int j = 1;
      for (h = 0; h < 3; h++)
	f[h] = 1;
      for (; j < 10; j++) {
	d = i && (b ? 0 : c); 
	i = 1;
	if (g)
	  a = e;
      }
  }
#endif
  return;
}

