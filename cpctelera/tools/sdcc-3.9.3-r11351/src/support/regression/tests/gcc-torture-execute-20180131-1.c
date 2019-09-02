/*
20180131-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* PR rtl-optimization/84071 */
/* Reported by Wilco <wilco@gcc.gnu.org> */

typedef union 
{
  signed short ss;
  unsigned short us;
  int x;
} U;

#if 0 // Enable when SDCC cann pass struct / union
int f(int x, int y, int z, int a, U u);

int f(int x, int y, int z, int a, U u)
{
  return (u.ss <= 0) + u.us;
}
#endif

void
testTortureExecute (void)
{
#if 0 // Enable when SDCC cann pass struct / union
  U u = { .ss = -1 };

  if (f (0, 0, 0, 0, u) != (1 << sizeof (short) * 8))
    ASSERT (0);

  return;
#endif
}
