/*
   930614-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51)
  int i, j, k, l;
  float x[8][2][8][2];

  for (i = 0; i < 8; i++)
    for (j = i; j < 8; j++)
      for (k = 0; k < 2; k++)
	for (l = 0; l < 2; l++)
	  {
	    if ((i == j) && (k == l))
	      x[i][k][j][l] = 0.8;
	    else
	      x[i][k][j][l] = 0.8;
	    if (x[i][k][j][l] < 0.0)
	      ASSERT (0);
	  }

  return;
#endif
}

