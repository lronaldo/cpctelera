/*
   20001121-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

double d;

static
inline double foo (void)
{
  return d;
}

static
inline int bar (void)
{
  foo();
  return 0;
}

void
testTortureExecute (void)
{
  if (bar ())
    ASSERT (0);
  return;
}

