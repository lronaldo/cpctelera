/*
   ssad-run.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

extern int abs (int __x);

static int
foo (signed char *w, int i, signed char *x, int j)
{
  int tot = 0;
  for (int a = 0; a < 16; a++)
    {
      for (int b = 0; b < 16; b++)
	tot += abs (w[b] - x[b]);
      w += i;
      x += j;
    }
  return tot;
}

void
bar (signed char *w, signed char *x, int i, int *result)
{
  *result = foo (w, 16, x, i);
}

void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !( (defined (__SDCC_mos6502) || defined(__SDCC_mos65c02 )) && defined(__SDCC_STACK_AUTO) ) // Lack of memory
  signed char m[256];
  signed char n[256];
  int sum, i;

  for (i = 0; i < 256; ++i)
    if (i % 2 == 0)
      {
	m[i] = (i % 8) * 2 + 1;
	n[i] = -(i % 8);
      }
    else
      {
	m[i] = -((i % 8) * 2 + 2);
	n[i] = -((i % 8) >> 1);
      }

  bar (m, n, 16, &sum);

  if (sum != 2368)
    ASSERT (0);

  return;
#endif
}
