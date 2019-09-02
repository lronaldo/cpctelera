/*
   pr80501.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

/* PR rtl-optimization/80501 */

signed char v = 0;

static signed char
foo (int x, int y)
{
  return x << y;
}

int
bar (void)
{
  return foo (v >= 0, CHAR_BIT - 1) >= 1;
}

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && (__GNUC__ < 7))
  if (sizeof (int) > sizeof (char) && bar () != 0)
    ASSERT (0);
  return;
#endif
}
