/*
   20041212-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* A function pointer compared with a void pointer should not be canonicalized.
   See PR middle-end/17564.  */
void *f (void);
void *
f (void)
{
  return f;
}
void
testTortureExecute (void)
{
  if (f () != f)
    ASSERT (0);
  return;
}
