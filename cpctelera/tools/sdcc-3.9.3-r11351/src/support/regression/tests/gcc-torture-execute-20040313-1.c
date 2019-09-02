/*
   20040313-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR middle-end/14470 */
/* Origin: Lodewijk Voge <lvoge@cs.vu.nl> */

void
testTortureExecute (void)
{
  int t[4] = { 1 }, d;

  d = 0;
  d = t[d]++;
  ASSERT (t[0] == 2);
  ASSERT (d == 1);
  return;
}
