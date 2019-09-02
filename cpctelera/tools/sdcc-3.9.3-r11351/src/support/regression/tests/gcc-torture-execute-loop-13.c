/*
   loop-13.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR opt/7130 */
#define TYPE long

#if !defined(__SDCC_pdk14) // Lack of memory
void
scale (TYPE *alpha, TYPE *x, int n)
{
  int i, ix;

  if (*alpha != 1)
    for (i = 0, ix = 0; i < n; i++, ix += 2)
      {
	TYPE tmpr, tmpi;
	tmpr = *alpha * x[ix];
	tmpi = *alpha * x[ix + 1];
	x[ix] = tmpr;
	x[ix + 1] = tmpi;
      }
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) // Lack of memory
  int i;
  TYPE x[10];
  TYPE alpha = 2;

  for (i = 0; i < 10; i++)
    x[i] = i;

  scale (&alpha, x, 5);

  if (x[9] != 18)
    ASSERT (0);

  return;
#endif
}
