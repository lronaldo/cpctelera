/*
   pr81555.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/81555 */

unsigned int a = 1, d = 0xfaeU, e = 0xe376U;
_Bool b = 0, f = 1;
unsigned char g = 1;

void
foo (void)
{
  _Bool c = a != b;
  if (c)
    f = 0;
  if (e & c & (unsigned char)d & c)
    g = 0;
}

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && (__GNUC__ < 7))
  foo ();
  if (f || g != 1)
    ASSERT (0);
  return;
#endif
}
