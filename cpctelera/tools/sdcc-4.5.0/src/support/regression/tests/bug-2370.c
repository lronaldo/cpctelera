/*
   bug-2370.c
   --reserve-regs-iy register allocation issue.
*/

#include <testfwk.h>

void *f(int n)
{
  ASSERT(n == 10 * sizeof(int));

  return 0;
}

int nblock;
int *tot;

void testBug(void)
{
  int ndigit = 100;

  if (nblock < 20) ndigit = 20;

  nblock = ndigit / 2;
  tot = (int *)f(nblock*sizeof(int));
}

