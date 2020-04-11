/*
   pr68624.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int b, c, d, e = 1, f, g, h, j;

static int
fn1 ()
{
  int a = c;
  if (h)
    return 9;
  g = (c || b) % e;
  if ((g || f) && b)
    return 9;
  e = d;
  for (c = 0; c > -4; c--)
    ;
  if (d)
    c--;
  j = c;
  return d;
}

void
testTortureExecute (void)
{
  fn1 ();

  if (c != -4)
    ASSERT (0);

  return;
}
