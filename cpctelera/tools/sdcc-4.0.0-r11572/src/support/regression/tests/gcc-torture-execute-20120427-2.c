/*
   20120427-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c89
#endif

typedef struct sreal
{
  unsigned sig;		/* Significant.  */
  int exp;		/* Exponent.  */
} sreal;

sreal_compare (sreal *a, sreal *b)
{
  if (a->exp > b->exp)
    return 1;
  if (a->exp < b->exp)
    return -1;
  if (a->sig > b->sig)
    return 1;
  if (a->sig < b->sig)
    return -1;
  return 0;
}

sreal a[] = {
   { 0, 0 },
   { 1, 0 },
   { 0, 1 },
   { 1, 1 }
};

void
testTortureExecute (void)
{
  int i, j;
  for (i = 0; i <= 3; i++) {
    for (j = 0; j < 3; j++) {
      if (i < j && sreal_compare(&a[i], &a[j]) != -1) ASSERT(0);
      if (i == j && sreal_compare(&a[i], &a[j]) != 0) ASSERT(0);
      if (i > j && sreal_compare(&a[i], &a[j]) != 1) ASSERT(0);
    }
  }
}
