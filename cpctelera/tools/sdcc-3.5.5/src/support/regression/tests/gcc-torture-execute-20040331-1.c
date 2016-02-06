/*
   20040331-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

/* PR c++/14755 */

void
testTortureExecute (void)
{
#if INT_MAX >= 2147483647
  struct { int count: 31; } s = { 0 };
  while (s.count--)
    ASSERT (0);
#elif INT_MAX >= 32767
  struct { int count: 15; } s = { 0 };
  while (s.count--)
    ASSERT (0);
#else
  /* Don't bother because __INT_MAX__ is too small.  */
#endif
  return;
}
