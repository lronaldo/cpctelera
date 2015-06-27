/*
   20001017-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#pragma disable_warning 85
#endif
 
void bug (double *Cref, char transb, int m, int n, int k,
	  double a, double *A, int fdA, double *B, int fdB,
	  double b, double *C, int fdC)
{
  if (C != Cref) ASSERT (0);
}
 
void
testTortureExecute (void)
{
  double A[1], B[1], C[1];
   
  bug (C, 'B', 1, 2, 3, 4.0, A, 5, B, 6, 7.0, C, 8);
   
  return;
}
                                                                                                                              
