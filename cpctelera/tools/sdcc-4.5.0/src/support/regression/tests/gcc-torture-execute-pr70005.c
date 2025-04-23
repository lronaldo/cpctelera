/*
   pr70005.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

unsigned char a = 6;
int b, c;

static void
fn1 ()
{
  int i = a > 1 ? 1 : a, j = 6 & (c = a && (b = a));
  int d = 0, e = a, f = ~c, g = b || a;
  unsigned char h = ~a;
  if (a)
    f = j;
  if (h && g)
    d = a;
  i = -~(f * d * h) + c && (e || i) ^ f;
  if (i != 1) 
    ASSERT (0); 
}

void
testTortureExecute (void)
{
  fn1 ();
  return; 
}
