/*
   20070623-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

int nge(int a, int b) {return -(a >= b);}
int ngt(int a, int b) {return -(a > b);}
int nle(int a, int b) {return -(a <= b);}
int nlt(int a, int b) {return -(a < b);}
int neq(int a, int b) {return -(a == b);}
int nne(int a, int b) {return -(a != b);}
int ngeu(unsigned a, unsigned b) {return -(a >= b);}
int ngtu(unsigned a, unsigned b) {return -(a > b);}
int nleu(unsigned a, unsigned b) {return -(a <= b);}
int nltu(unsigned a, unsigned b) {return -(a < b);}


void
testTortureExecute (void)
{
  if (nge(INT_MIN, INT_MAX) !=  0) ASSERT (0);
  if (nge(INT_MAX, INT_MIN) != -1) ASSERT (0);
  if (ngt(INT_MIN, INT_MAX) !=  0) ASSERT (0);
  if (ngt(INT_MAX, INT_MIN) != -1) ASSERT (0);
  if (nle(INT_MIN, INT_MAX) != -1) ASSERT (0);
  if (nle(INT_MAX, INT_MIN) !=  0) ASSERT (0);
  if (nlt(INT_MIN, INT_MAX) != -1) ASSERT (0);
  if (nlt(INT_MAX, INT_MIN) !=  0) ASSERT (0);

  if (neq(INT_MIN, INT_MAX) !=  0) ASSERT (0);
  if (neq(INT_MAX, INT_MIN) !=  0) ASSERT (0);
  if (nne(INT_MIN, INT_MAX) != -1) ASSERT (0);
  if (nne(INT_MAX, INT_MIN) != -1) ASSERT (0);

  if (ngeu(0, ~0U) !=  0) ASSERT (0);
  if (ngeu(~0U, 0) != -1) ASSERT (0);
  if (ngtu(0, ~0U) !=  0) ASSERT (0);
  if (ngtu(~0U, 0) != -1) ASSERT (0);
  if (nleu(0, ~0U) != -1) ASSERT (0);
  if (nleu(~0U, 0) !=  0) ASSERT (0);
  if (nltu(0, ~0U) != -1) ASSERT (0);
  if (nltu(~0U, 0) !=  0) ASSERT (0);
  
  return;
}
