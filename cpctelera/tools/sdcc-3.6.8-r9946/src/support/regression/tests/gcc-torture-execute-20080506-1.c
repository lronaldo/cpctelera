/*
   20080506-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR middle-end/36137 */

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && (__GNUC__ < 5))
  unsigned int u;
  int i = -1;

  u = MAX ((unsigned int) MAX (i, 0), 1);
  if (u != 1)
    ASSERT (0);

  u = MIN ((unsigned int) MAX (i, 0), (unsigned int) i);
  if (u != 0)
    ASSERT (0);
  return;
#endif
}

