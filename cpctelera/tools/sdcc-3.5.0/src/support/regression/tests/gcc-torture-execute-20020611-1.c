/*
   20020611-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/6997.  Missing (set_attr "cc" "none") in sleu pattern in
   cris.md.  Testcase from hp@axis.com.  */

int p;
int k;
unsigned int n;

void x ()
{
  unsigned int h;

  h = n <= 30;
  if (h)
    p = 1;
  else
    p = 0;

  if (h)
    k = 1;
  else
    k = 0;
}

unsigned int n = 30;

void
testTortureExecute (void)
{
  x ();
  if (p != 1 || k != 1)
    ASSERT (0);
  return;
}
