/*
   pr47148.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Bug #2874
/* PR tree-optimization/47148 */

static inline unsigned
bar (unsigned x, unsigned y)
{
  if (y >= 32)
    return x;
  else
    return x >> y;
}

static unsigned a = 1, b = 1;

static inline void
foo (unsigned char x, unsigned y)
{
  if (!y)
    return;
  unsigned c = (0x7000U / (x - 2)) ^ a;
  unsigned d = bar (a, a);
  b &= ((a - d) && (a - 1)) + c;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Bug #2874
  foo (1, 1);
  foo (-1, 1);
  if (b && ((unsigned char) -1) == 255)
    ASSERT (0);
  return;
#endif
}

