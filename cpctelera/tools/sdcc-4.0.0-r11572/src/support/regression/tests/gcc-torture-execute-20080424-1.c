/*
   20080424-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/36008 */

#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
int g[48][3][3];

void
bar (int x[3][3], int y[3][3])
{
  static int i;
  if (x != g[i + 8] || y != g[i++])
    ASSERT (0);
}

static inline void
foo (int x[][3][3])
{
  int i;
  for (i = 0; i < 8; i++)
    {
      int k = i + 8;
      bar (x[k], x[k - 8]);
    }
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  foo (g);
  return;
#endif
}

